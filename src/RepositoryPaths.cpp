#include "RepositoryPaths.hpp"
#include <utility>

namespace minigit {

    RepositoryPaths::RepositoryPaths(std::filesystem::path projectRoot)
        : projectRoot_(std::move(projectRoot)),
          metadataRoot_(projectRoot_ / ".minigit") {}

    const std::filesystem::path& RepositoryPaths::projectRoot() const {
        return projectRoot_;
    }

    const std::filesystem::path& RepositoryPaths::metadataRoot() const {
        return metadataRoot_;
    }
    // Returns the path to a file inside the staging area of the repository.
    std::filesystem::path RepositoryPaths::stageFile(
         const std::string& path) const {
            return metadataRoot_/"stage" / path;
         }
    

} // namespace minigit