#include "file_dialog.h"

#include <windows.h>
#include <commdlg.h>

std::filesystem::path SelectInputFile() {
    OPENFILENAMEW ofn{};
    wchar_t filePathBuffer[MAX_PATH] = { 0 };

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = nullptr;
    ofn.lpstrFile = filePathBuffer;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = L"Executables (*.exe;*.dll)\0*.exe;*.dll\0All Files (*.*)\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileNameW(&ofn) == TRUE) {
        return std::filesystem::path(filePathBuffer);
    }

    return {};
}
