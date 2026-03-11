#pragma once

#include <filesystem>
#include <string>

struct ConverterOptions {
    std::filesystem::path inputPath;
    std::filesystem::path outputPath;
    std::string symbolName = "shellcode";
    std::string sizeSymbolName = "shellcodeSize";
    size_t bytesPerLine = 16;
    bool showHelp = false;
};

bool ParseArguments(int argc, wchar_t* argv[], ConverterOptions& options, std::wstring& error);
void PrintUsage();
