// minimal header for linking compiled payload data
#pragma once

#include <Windows.h>
#include <winternl.h>

extern unsigned char rawData[];
extern const size_t rawDataSize;

// the x86 typedef for NtQueryInformationProcess
typedef NTSTATUS(WINAPI* NTQUERYINFOPROC)(
    HANDLE           ProcessHandle,
    PROCESSINFOCLASS ProcessInformationClass,
    PVOID            ProcessInformation,
    ULONG            ProcessInformationLength,
    PULONG           ReturnLength
    );
