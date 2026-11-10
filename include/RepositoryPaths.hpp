#pragma once
#include <filesystem>
#include <string>

namespace minigit {

    class RepositoryPaths {
    public:
        explicit RepositoryPaths(std::filesystem::path projectRoot);
        const std::filesystem::path& projectRoot() const;
        const std::filesystem::path& metadataRoot() const;

        // RP-2: Return paths to the HEAD and index files.
        std::filesystem::path headFile() const;
        std::filesystem::path indexFile() const;

        // RP-3: Returns the path to a file inside the staging area of the repository.
        std::filesystem::path stageFile(const std::string& path) const;

        // RP-4: where one commit is stored, and one file inside that commit.
        // These only build paths; they do not create anything on disk.
        std::filesystem::path commitDirectory(const std::string& id) const;
        std::filesystem::path committedFile(const std::string& id,
                                            const std::string& path) const;

        // RP-5: Check and clean a file path without opening it.
        std::string safeRelativeFile(const std::string& argument) const;

    private:
        std::filesystem::path projectRoot_;
        std::filesystem::path metadataRoot_;
    };

} // namespace minigit