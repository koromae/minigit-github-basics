#pragma once

#include "FileSystem.hpp"
#include "RepositoryPaths.hpp"

#include <string>
#include <vector>

namespace minigit {

// HDR-1: The project files and the Stage (.minigit/stage/). Built in
// milestone M4. Open question for the instructor: if the Stage functions
// belong in CommitStore instead, WT-3 to WT-6 move there unchanged.
class WorkingTree {
public:
    // WT-1 (M4): Connect the helper classes.
    WorkingTree(const RepositoryPaths& paths, const FileSystem& files);

    // WT-2 (M4): The project files as sorted relative paths, skipping .minigit/.
    std::vector<std::string> listFiles() const;

    // WT-3 (M4): The relative paths inside .minigit/stage/.
    std::vector<std::string> stagedPaths() const;

    // WT-4 (M4): True if nothing is staged.
    bool isStageEmpty() const;

    // WT-5 (M4): Remove everything inside .minigit/stage/. Called only after
    // HEAD has been updated by a successful commit.
    void clearStage() const;

    // WT-6 (M4): The staged content of one path (paths.stageFile(relativePath)).
    std::string readStaged(const std::string& relativePath) const;

private:
    const RepositoryPaths& paths_;
    const FileSystem& files_;
};

} // namespace minigit
