# Process Hollowing Toolkit 🔧

This repository contains a simple demonstration of **process hollowing**, an advanced technique where a legitimate process is started in suspended mode and its contents are replaced with custom code (a "shellcode" payload). It is intended **for educational use only** and should never be deployed on systems without explicit permission.

---

## 📘 Understanding the Technique
1. **Create a target process** suspended (e.g. `svchost.exe`).
2. **Query process information** via `NtQueryInformationProcess` to retrieve the PEB.
3. **Allocate memory** inside the target with `VirtualAllocEx` using `PAGE_EXECUTE_READWRITE`.
4. **Write your payload** (PE image) into the remote process, section by section.
5. **Update the PEB.ImageBaseAddress** to point at the new image.
6. **Start a remote thread** at the payload's entry point using `CreateRemoteThread`.
7. **Resume execution** so the hollowed process runs your code in its address space.

This project contains two minimal injectors (x64 and x86) plus a helper to convert arbitrary executables into C arrays.

---

## 🚀 Getting Started
You don’t need a C++ compiler to read or modify the code, but the steps below assume a Windows development environment with Visual Studio.

### 1. Clone the repository
```powershell
git clone https://github.com/mateethedesigner/process-hollowing-main.git
cd process-hollowing-main
```

### 2. Generate shellcode (optional)
Use the provided converter to transform any PE file (DLL/EXE) into a C header:
```powershell
cd shellcode-converter/shellcode-converter
# build the converter or use prebuilt binary
# run it and choose an executable when prompted
```
The tool will save `shellcode.h` alongside the selected file. This header defines:
```cpp
unsigned char shellcode[] = { ... };
const size_t shellcodeSize = ...;
``` 
For the x86 project the array is named `rawData`/`rawDataSize`.

#### Converter CLI options
You can also run the converter non-interactively:

```powershell
shellcode-converter.exe --input C:\path\payload.exe
```

Supported options:
- `-i, --input <path>` input binary path
- `-o, --output <path>` output header path
- `-s, --symbol <name>` data symbol name (default: `shellcode`)
- `--size-symbol <name>` size symbol name (default: `shellcodeSize`)
- `--bytes-per-line <n>` bytes per line in generated array (default: `16`)
- `-h, --help` print help

Example with custom symbol names:

```powershell
shellcode-converter.exe --input C:\payload.exe --output C:\payload_header.h --symbol rawData --size-symbol rawDataSize
```

### 3. Prepare injector projects
- Copy the generated header into:
  - `x64/process_hollowing/src/hdr` (for 64‑bit payloads)
  - `x86/process_hollowing/src/hdr` (for 32‑bit payloads)
- `hdr/shellcode.h` in each project already declares the array as `extern`, so you can swap payloads easily.

### 4. Build (optional)
Open the appropriate solution and select the correct platform & configuration:
- `x64/process_hollowing/process_hollowing.sln` → Release | x64
- `x86/process_hollowing/process_hollowing.sln` → Release | Win32

You can also compile from the command line using `msbuild` or `cl.exe` if desired.

### 5. Run
Execute the compiled injector; it will log its progress and payload size to the console. The target process (`svchost.exe` by default) will be hollowed with your code.

---

## 🛠️ Project Structure
```
/README.md               ← this file
/shellcode-converter/    ← converter app and source
/x64/                    ← 64‑bit injector
/x86/                    ← 32‑bit injector
```
Each `hdr` directory contains `shellcode.h` which declares the payload array.

---

## 💡 Tips & Notes
- Matching **subsystems** (console vs GUI) between shellcode and injector improves reliability.
- Both injectors log helpful diagnostics (errors, shellcode size, success messages).
- The code is intentionally minimal; extend it at your own risk!
- Use only in controlled environments and for learning.

---

## 📚 Further Reading
- [What is process hollowing by bmdyy](https://www.youtube.com/watch?v=aQQT-nYoiJo)
- [Malware Theory – Process Injection by MalwareAnalysisForHedgehogs](https://www.youtube.com/watch?v=tBR1-1J5Jec)

---

> **Disclaimer:** This repository is provided "as is" for educational purposes. The author assumes no responsibility for misuse.

---

🌟 Star the repo if you found it useful!

