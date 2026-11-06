#pragma once
#include <filesystem>
#include<string>

namespace minigit {

    class RepositoryPaths {
    public:
        explicit RepositoryPaths(std::filesystem::path projectRoot);  
        const std::filesystem::path& projectRoot() const;             
        const std::filesystem::path& metadataRoot() const;            

    // RP-4
        std::filesystem::path commitDirectory(const std::string& id) const;
        std::filesystem::path committedFile(const std::string& id,
                                        const std::string& path) const;

    private:
        std::filesystem::path projectRoot_;     // RP-1, must stay
        std::filesystem::path metadataRoot_;    // RP-1, must stay
};

} // namespace minigit
    
