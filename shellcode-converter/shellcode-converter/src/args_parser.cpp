#include "args_parser.h"

#include <iostream>

namespace {

bool IsFlag(const std::wstring& arg, const wchar_t* shortName, const wchar_t* longName) {
    return arg == shortName || arg == longName;
}

bool ParsePositiveInteger(const std::wstring& text, size_t& value) {
    if (text.empty()) {
        return false;
    }

    size_t parsed = 0;
    for (wchar_t ch : text) {
        if (ch < L'0' || ch > L'9') {
            return false;
        }
        parsed = parsed * 10 + static_cast<size_t>(ch - L'0');
    }

    if (parsed == 0) {
        return false;
    }

    value = parsed;
    return true;
}

} // namespace

bool ParseArguments(int argc, wchar_t* argv[], ConverterOptions& options, std::wstring& error) {
    for (int i = 1; i < argc; ++i) {
        std::wstring arg = argv[i];

        if (IsFlag(arg, L"-h", L"--help")) {
            options.showHelp = true;
            return true;
        }

        if (IsFlag(arg, L"-i", L"--input")) {
            if (i + 1 >= argc) {
                error = L"Missing value for --input.";
                return false;
            }
            options.inputPath = argv[++i];
            continue;
        }

        if (IsFlag(arg, L"-o", L"--output")) {
            if (i + 1 >= argc) {
                error = L"Missing value for --output.";
                return false;
            }
            options.outputPath = argv[++i];
            continue;
        }

        if (IsFlag(arg, L"-s", L"--symbol")) {
            if (i + 1 >= argc) {
                error = L"Missing value for --symbol.";
                return false;
            }
            std::wstring value = argv[++i];
            options.symbolName.assign(value.begin(), value.end());
            continue;
        }

        if (arg == L"--size-symbol") {
            if (i + 1 >= argc) {
                error = L"Missing value for --size-symbol.";
                return false;
            }
            std::wstring value = argv[++i];
            options.sizeSymbolName.assign(value.begin(), value.end());
            continue;
        }

        if (arg == L"--bytes-per-line") {
            if (i + 1 >= argc) {
                error = L"Missing value for --bytes-per-line.";
                return false;
            }
            size_t parsed = 0;
            if (!ParsePositiveInteger(argv[++i], parsed)) {
                error = L"--bytes-per-line must be a positive integer.";
                return false;
            }
            options.bytesPerLine = parsed;
            continue;
        }

        // First unknown positional value is treated as input path.
        if (options.inputPath.empty()) {
            options.inputPath = arg;
            continue;
        }

        error = L"Unknown argument: " + arg;
        return false;
    }

    return true;
}

void PrintUsage() {
    std::wcout << L"Shellcode Converter\n"
               << L"Usage:\n"
               << L"  shellcode-converter.exe [input_path] [options]\n\n"
               << L"Options:\n"
               << L"  -h, --help                 Show this help\n"
               << L"  -i, --input <path>         Input binary path\n"
               << L"  -o, --output <path>        Output header path (default: input dir/shellcode.h)\n"
               << L"  -s, --symbol <name>        Data symbol name (default: shellcode)\n"
               << L"      --size-symbol <name>   Size symbol name (default: shellcodeSize)\n"
               << L"      --bytes-per-line <n>   Bytes per output line (default: 16)\n";
}
