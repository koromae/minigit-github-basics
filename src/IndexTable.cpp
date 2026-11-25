#include "IndexTable.hpp"

namespace minigit {

// IT-1: Keep the paths and file helper for index work.
IndexTable::IndexTable(const RepositoryPaths& paths, const FileSystem& files)
    : paths_(paths), files_(files) {}

} // namespace minigit
