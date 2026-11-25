#include "IndexTable.hpp"

#include <algorithm>

namespace minigit {

// IT-1: Keep the paths and file helper for index work.
IndexTable::IndexTable(const RepositoryPaths& paths, const FileSystem& files)
    : paths_(paths), files_(files) {}

// IT-3: Check whether the path is already in the index.
bool IndexTable::contains(const std::vector<std::string>& entries,
                          const std::string& path) {
    return std::find(entries.begin(), entries.end(), path) != entries.end();
}

} // namespace minigit