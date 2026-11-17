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
        // Issue 13: Return the directory path for a checkpoint ID.
        std::filesystem::path checkpointDirectory(const std::string& id) const;

        // RP-14 (#32): Return where a user's file lives in the Working Tree:
        // projectRoot() / relativePath. Only builds a path; touches nothing on disk.
        // Throws std::invalid_argument if relativePath is empty or absolute.
        std::filesystem::path workingFile(const std::string& relativePath) const;

        // RP-15 (#34): Return where a file is saved inside a checkpoint:
        // checkpointDirectory(id) / "files" / relativePath. Only builds a path.
        // Throws std::invalid_argument if id is empty, or if relativePath is
        // empty or absolute.
        std::filesystem::path commitFile(const std::string& id,
                                         const std::string& relativePath) const;

    private:
        std::filesystem::path projectRoot_;
        std::filesystem::path metadataRoot_;
    };

} // namespace minigit