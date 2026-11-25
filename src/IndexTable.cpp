#include "IndexTable.hpp"
#include "PathLab.hpp"
#include <algorithm>
#include <stdexcept>


namespace minigit {

// IT-1: Keep the paths and file helper for index work.
IndexTable::IndexTable(const RepositoryPaths& paths, const FileSystem& files)
    : paths_(paths), files_(files) {}

// IT-4: Save sorted, unique paths to the index.
void IndexTable::write(const std::vector<std::string>& entries) const {
    auto sortedEntries = entries;

    // Use forward slashes before checking for duplicates.
    for (auto& path : sortedEntries) {
        std::replace(path.begin(), path.end(), '\\', '/');
    }

    std::sort(sortedEntries.begin(), sortedEntries.end());
    sortedEntries.erase(std::unique(sortedEntries.begin(), sortedEntries.end()),
                        sortedEntries.end());

    // Write one path per line, including a newline after the last path.
    std::string contents;
    for (const auto& path : sortedEntries) {
        contents += path;
        contents += '\n';
    }

    const auto indexFile = paths_.indexFile();
    if (!writeTextFile(indexFile, contents)) {
        throw std::runtime_error("Unable to write index file: " + indexFile.string());
    }
}

} // namespace minigit
