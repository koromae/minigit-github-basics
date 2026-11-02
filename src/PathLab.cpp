#include "PathLab.hpp"

namespace minigit {

std::filesystem::path metadataDirectoryPath(
    const std::filesystem::path& root
) {
    return root / ".minigit";
}

// Returns the HEAD file path inside the metadata directory.
std::filesystem::path headFilePath(
    const std::filesystem::path& root
) {
    return metadataDirectoryPath(root) / "HEAD";
}

} // namespace minigit
