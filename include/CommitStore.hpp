#pragma once

#include "FileSystem.hpp"
#include "RepositoryPaths.hpp"

#include <filesystem>
#include <map>
#include <string>
#include <vector>

namespace minigit {

// HDR-1: The data of one commit. parentId is empty for the first commit.
struct Commit {
    std::string id;
    std::string message;
    std::string parentId;
};

// HDR-1: Owns HEAD, next-id.txt and the commits/<id>/ folders.
// Built in milestone M4. See docs/storage-format.md for the layout.
class CommitStore {
public:
    // CS-1 (M4): Connect the helper classes.
    CommitStore(const RepositoryPaths& paths, const FileSystem& files);

    // CS-2 (M4): The ID in HEAD, or "" if there is no commit yet.
    std::string latestId() const;

    // CS-3 (M4): Write an ID into HEAD.
    void setLatest(const std::string& id) const;

    // CS-4 (M4): Return the ID in next-id.txt and save the next one (1, 2, 3, ...).
    std::string nextId();

    // CS-5 (M4): Create commits/<id>/ with files/, message.txt, parent.txt,
    // time.txt and index.txt. sources maps each relative path to the file
    // whose content is saved for it.
    void createCommit(const std::string& id,
                      const std::string& message,
                      const std::string& parentId,
                      const std::map<std::string, std::filesystem::path>& sources) const;

    // CS-6 (M4): All commits newest first, starting at HEAD and following parent.txt.
    std::vector<Commit> historyNewestFirst() const;

    // CS-7 (M4): The relative paths saved in one commit.
    std::vector<std::string> pathsIn(const std::string& id) const;

    // CS-8 (M4): The saved content of one path in one commit.
    std::string readContent(const std::string& id,
                            const std::string& relativePath) const;

private:
    const RepositoryPaths& paths_;
    const FileSystem& files_;
};

} // namespace minigit
