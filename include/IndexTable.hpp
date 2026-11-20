#pragma once

#include "FileSystem.hpp"
#include "RepositoryPaths.hpp"

#include <string>
#include <vector>

namespace minigit {

// HDR-1: Reads, searches and writes .minigit/index.txt, the tracked paths
// (one relative path per line, sorted). Built in milestone M3.
class IndexTable {
public:
    // IT-1 (M3): Connect the helper classes. index.txt is paths.indexFile().
    IndexTable(const RepositoryPaths& paths, const FileSystem& files);

    // IT-2 (M3): Read the tracked paths, one per line.
    std::vector<std::string> read() const;

    // IT-3 (M3): Check whether a path is already tracked.
    static bool contains(const std::vector<std::string>& entries,
                         const std::string& path);

    // IT-4 (M3): Sort and save the tracked paths.
    void write(const std::vector<std::string>& entries) const;

private:
    const RepositoryPaths& paths_;
    const FileSystem& files_;
};

} // namespace minigit
