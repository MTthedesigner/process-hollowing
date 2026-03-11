#include <filesystem>
#include <iostream>
#include <vector>

#include "args_parser.h"
#include "converter.h"
#include "file_dialog.h"

int wmain(int argc, wchar_t* argv[]) {
    ConverterOptions options;
    std::wstring error;
    if (!ParseArguments(argc, argv, options, error)) {
        std::wcerr << L"Argument error: " << error << L"\n\n";
        PrintUsage();
        return 1;
    }

    if (options.showHelp) {
        PrintUsage();
        return 0;
    }

    if (options.inputPath.empty()) {
        options.inputPath = SelectInputFile();
        if (options.inputPath.empty()) {
            std::wcerr << L"No file selected.\n";
            return 1;
        }
    }

    if (!std::filesystem::exists(options.inputPath) || !std::filesystem::is_regular_file(options.inputPath)) {
        std::wcerr << L"Input file is missing or invalid: " << options.inputPath.wstring() << L"\n";
        return 1;
    }

    std::vector<unsigned char> bytes;
    if (!ReadBinaryFile(options.inputPath, bytes, error)) {
        std::wcerr << L"Read failed for " << options.inputPath.wstring() << L": " << error << L"\n";
        return 1;
    }

    if (options.outputPath.empty()) {
        options.outputPath = options.inputPath.parent_path() / L"shellcode.h";
    }

    std::error_code compareError;
    if (std::filesystem::equivalent(options.inputPath, options.outputPath, compareError)) {
        std::wcerr << L"Output path must be different from input path.\n";
        return 1;
    }

    if (!WriteShellcodeHeader(
            options.outputPath,
            bytes,
            options.symbolName,
            options.sizeSymbolName,
            options.bytesPerLine,
            error)) {
        std::wcerr << L"Write failed for " << options.outputPath.wstring() << L": " << error << L"\n";
        return 1;
    }

    std::wcout << L"Header written to " << options.outputPath.wstring() << L"\n";
    std::wcout << L"Byte count: " << bytes.size() << L"\n";
    std::wcout << L"Symbols: "
               << std::wstring(options.symbolName.begin(), options.symbolName.end())
               << L", "
               << std::wstring(options.sizeSymbolName.begin(), options.sizeSymbolName.end())
               << L"\n";
    return 0;
}
