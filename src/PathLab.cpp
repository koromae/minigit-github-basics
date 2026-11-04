#include "PathLab.hpp"
#include <fstream>
#include <system_error>

namespace minigit {

std::filesystem::path metadataDirectory(
    const std::filesystem::path& root) {
    return root / ".minigit";
}

std::filesystem::path headFilePath(
    const std::filesystem::path& root) {
    return metadataDirectory(root) / "HEAD";
}
//  This function is used to write a text file to the specified path. It creates any
// necessary parent directories if they exist. It returns true if the file was written successfully, 
// and false otherwise.
bool writeTextFile(const std::filesystem::path& filePath,
                   const std::string& text) {
    const auto parent = filePath.parent_path();
    if (!parent.empty()) {
        std::error_code error;
        std::filesystem::create_directories(parent, error);
        if (error) {
            return false;
        }
    }

    std::ofstream file(filePath, std::ios::binary);
    if (!file) {
        return false;
    }

    file << text;
    file.close();
    return !file.fail();
}
} // namespace minigit
