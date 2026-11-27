#pragma once

#include "CommitStore.hpp"
#include "FileSystem.hpp"
#include "RepositoryPaths.hpp"

#include <filesystem>
#include <string>
#include <vector>

namespace minigit {

// HDR-1: Runs each MiniGit command. Every command function either returns
// data or throws; CommandLineApp does all the printing. Bodies are written
// only in the milestone shown next to each function.
class MiniGitRepository {
public:
    // MR-1 (M2): Store the project root. Creates nothing on disk.
    explicit MiniGitRepository(const std::filesystem::path& projectRoot);

    // MR-1 (M2): The repository locations, e.g. for printing the .minigit path.
    const RepositoryPaths& paths() const;

    // MR-2 (M2): True only if .minigit/, stage/, commits/, HEAD, index.txt and
    // next-id.txt all exist.
    bool isInitialized() const;

    // MR-3 (M2): Create any missing part of the .minigit layout, never
    // overwriting an existing file. Returns false if the repository already
    // existed, so running init again does no harm.
    bool initialize();

    // MR-4 (M2): Throw if the folder is not a valid repository.
    void requireRepository() const;

    // MR-5a (M3): Check the file a user wants to add and return its clean
    // relative path (with / separators). Throws std::runtime_error if the path
    // is empty, absolute, outside the project or inside .minigit, if the file
    // does not exist, or if it is a folder. Creates and changes nothing.
    // Public so it can be tested on its own; addFile() (MR-5b) calls it.
    std::string checkFileToAdd(const std::string& argument) const;

    // MR-5 (M3): Check the path, copy the working file into the Stage and
    // add the path to index.txt if it is not tracked yet.
    void addFile(const std::string& argument);

    // MR-6 (M4): Create a commit from the Stage and return its new ID.
    // Throws if the message or the Stage is empty.
    std::string commit(const std::string& message);

    // MR-7 (M4): All commits, newest first.
    std::vector<Commit> log() const;

    // MR-8 (M5): One sorted "path: state" line per file. Changes nothing.
    std::vector<std::string> status() const;

private:
    RepositoryPaths paths_;
    FileSystem files_;
    // Added in their milestones, once their constructors exist:
    //   IndexTable  index_;    (M3, IT-1)
    //   WorkingTree tree_;     (M4, WT-1)
    //   CommitStore commits_;  (M4, CS-1)
};

} // namespace minigit
