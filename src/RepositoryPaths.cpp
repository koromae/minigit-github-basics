#include "RepositoryPaths.hpp"
#include <stdexcept>
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
// RP-2: Construct paths without creating files.
std::filesystem::path RepositoryPaths::headFile() const {
    return metadataRoot_ / "HEAD";
}

std::filesystem::path RepositoryPaths::indexFile() const {
    return metadataRoot_ / "index.txt";
}
    // Returns the path to a file inside the staging area of the repository.
    std::filesystem::path RepositoryPaths::stageFile(
         const std::string& path) const {
            return metadataRoot_/"stage" / path;
         }
    

    // RS-5: Accept only safe file paths inside the project.
    std::string RepositoryPaths::safeRelativeFile(
        const std::string& argument) const {
        // RS-5: Reject empty and rooted paths first.
        if (argument.empty()) {
            throw std::runtime_error("File path must not be empty");
        }

        const std::filesystem::path input(argument);
        if (input.has_root_path()) {
            throw std::runtime_error("File path must be relative");
        }

        // RS-5: Clean the path before checking where it goes.
        const auto normalized = input.lexically_normal();
        if (normalized.empty() || normalized == ".") {
            throw std::runtime_error("File path must identify a file");
        }

        if (*normalized.begin() == "..") {
            throw std::runtime_error("File path must stay within the project");
        }

        // RS-5: Check each folder name, so names like "my..notes.txt" stay safe.
        for (const auto& part : normalized) {
            if (part == ".git" || part == ".minigit") {
                throw std::runtime_error(
                    "File path must not include repository metadata");
            }
        }

        return normalized.generic_string();
    }

} // namespace minigit