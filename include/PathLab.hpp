#pragma once
#include <filesystem>
#include <string>

namespace minigit {

// Week 1 Issues will add small helpers for building paths below a repository root.
// Path construction describes locations; it does not create files or directories.
//Return the path to the metadata folder inside a repository.
//This builds the path only; it does not create the folder.
std::filesystem::path metadataDirectory(const std::filesystem::path& root); 

// Returns the HEAD file path inside the repository metadata directory.
std::filesystem::path headFilePath(const std::filesystem::path& root);
bool writeTextFile(const std::filesystem::path& filePath, const std::string& text);

} // namespace minigit
