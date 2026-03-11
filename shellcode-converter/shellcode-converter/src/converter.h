#pragma once

#include <filesystem>
#include <string>
#include <vector>

bool ReadBinaryFile(const std::filesystem::path& inputPath, std::vector<unsigned char>& output, std::wstring& error);
bool WriteShellcodeHeader(
	const std::filesystem::path& outputPath,
	const std::vector<unsigned char>& data,
	const std::string& symbolName,
	const std::string& sizeSymbolName,
	size_t bytesPerLine,
	std::wstring& error);
