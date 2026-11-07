
#include "RepositoryPaths.hpp"
#include <stdexcept>
#include <system_error>
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

// RP-3: Returns the path to a file in the staging area.
std::filesystem::path RepositoryPaths::stageFile(
    const std::string& path) const {
    return metadataRoot_ / "stage" / path;
}

// RP-5 and RP-6: Validate a relative file path.
std::string RepositoryPaths::safeRelativeFile(
    const std::string& argument) const {

    if (argument.empty()) {
        throw std::runtime_error("File path must not be empty");
    }

    const std::filesystem::path input(argument);

    if (input.has_root_path()) {
        throw std::runtime_error("File path must be relative");
    }

    const auto normalized = input.lexically_normal();

    if (normalized.empty() || normalized == ".") {
        throw std::runtime_error("File path must identify a file");
    }

    if (*normalized.begin() == "..") {
        throw std::runtime_error(
            "File path must stay within the project");
    }

    // RP-5: Reject repository metadata directories.
    for (const auto& part : normalized) {
        if (part == ".git" || part == ".minigit") {
            throw std::runtime_error(
                "File path must not include repository metadata");
        }
    }

    // RP-6: Reject symbolic links in any path component.
    // Check the original components before normalization.
    std::filesystem::path current = projectRoot_;

    for (const auto& part : input) {
        current /= part;

        std::error_code ec;
        const auto status =
            std::filesystem::symlink_status(current, ec);

        // A missing file is allowed by RP-5.
        if (ec == std::errc::no_such_file_or_directory) {
            continue;
        }

        if (ec) {
            throw std::runtime_error(
                "Unable to check file path: " + ec.message());
        }

        if (std::filesystem::is_symlink(status)) {
            throw std::runtime_error(
                "File path must not contain symbolic links");
        }
    }

    return normalized.generic_string();
}

} // namespace minigit
