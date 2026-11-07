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
            // Returns the path to a file inside the staging area of the repository.
            std::filesystem::path stageFile(const std::string& path) const;

            // RS-5: Check and clean a file path without opening it.
            std::string safeRelativeFile(const std::string& argument) const;

        private:
            std::filesystem::path projectRoot_;
            std::filesystem::path metadataRoot_;
    };

} // namespace minigit
    
