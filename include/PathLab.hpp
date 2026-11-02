#pragma once

#include <filesystem>

namespace minigit {

// Returns the metadata directory path inside the repository.
std::filesystem::path metadataDirectoryPath(
    const std::filesystem::path& root
);

// Returns the HEAD file path inside the metadata directory.
std::filesystem::path headFilePath(
    const std::filesystem::path& root
);

} // namespace minigit
