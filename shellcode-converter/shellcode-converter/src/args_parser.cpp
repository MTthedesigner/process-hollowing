#include "args_parser.h"

#include <iostream>
#include <limits>

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

        size_t digit = static_cast<size_t>(ch - L'0');
        if (parsed > (std::numeric_limits<size_t>::max() - digit) / 10) {
            return false;
        }
        parsed = parsed * 10 + digit;
    }

    if (parsed == 0) {
        return false;
    }

    value = parsed;
    return true;
}

bool IsAsciiIdentifierStart(char ch) {
    return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || ch == '_';
}

bool IsAsciiIdentifierContinue(char ch) {
    return IsAsciiIdentifierStart(ch) || (ch >= '0' && ch <= '9');
}

bool ConvertAndValidateIdentifier(const std::wstring& input, std::string& output) {
    if (input.empty()) {
        return false;
    }

    output.clear();
    output.reserve(input.size());

    for (wchar_t ch : input) {
        if (ch > 0x7F) {
            return false;
        }
        output.push_back(static_cast<char>(ch));
    }

    if (!IsAsciiIdentifierStart(output[0])) {
        return false;
    }

    for (size_t i = 1; i < output.size(); ++i) {
        if (!IsAsciiIdentifierContinue(output[i])) {
            return false;
        }
    }

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
            if (!ConvertAndValidateIdentifier(value, options.symbolName)) {
                error = L"--symbol must be a valid ASCII C identifier.";
                return false;
            }
            continue;
        }

        if (arg == L"--size-symbol") {
            if (i + 1 >= argc) {
                error = L"Missing value for --size-symbol.";
                return false;
            }
            std::wstring value = argv[++i];
            if (!ConvertAndValidateIdentifier(value, options.sizeSymbolName)) {
                error = L"--size-symbol must be a valid ASCII C identifier.";
                return false;
            }
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

            if (parsed > 256) {
                error = L"--bytes-per-line must be <= 256.";
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
