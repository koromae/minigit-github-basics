#include "IndexTable.hpp"

#include <algorithm>
#include <sstream>

namespace minigit {

// IT-1: Keep the paths and file helper for index work.
IndexTable::IndexTable(const RepositoryPaths& paths, const FileSystem& files)
    : paths_(paths), files_(files) {}

// IT-2: Read nonblank paths and remove Windows line endings.
std::vector<std::string> IndexTable::read() const {
    std::istringstream input(files_.readFile(paths_.indexFile()));
    std::vector<std::string> entries;
    std::string line;

    while (std::getline(input, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        if (!line.empty()) {
            entries.push_back(line);
        }
    }

    return entries;
}

// IT-3: Check whether a path is already tracked.
bool IndexTable::contains(const std::vector<std::string>& entries,
                          const std::string& path) {
    return std::find(entries.begin(), entries.end(), path) != entries.end();
}

} // namespace minigit
