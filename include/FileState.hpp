#pragma once

#include <optional>
#include <string>
#include <vector>

namespace minigit {

// HDR-1: The status of one file, built from its content in three places.
// An empty optional means the file is absent there. FileState uses no other
// class; MiniGitRepository::status() creates one per path (milestone M5).
class FileState {
public:
    // FST-1 (M5): Store whether the path is tracked and its content in the
    // Working Tree, the Stage and the latest commit.
    FileState(bool tracked,
              const std::optional<std::string>& working,
              const std::optional<std::string>& staged,
              const std::optional<std::string>& latest);

    // FST-2 (M5): Every label that applies: "untracked", "staged",
    // "staged and changed after staging", "modified", "deleted" or "clean".
    std::vector<std::string> labels() const;

    // FST-3 (M5): The labels joined into the text that status prints.
    std::string describe() const;

private:
    bool tracked_;
    std::optional<std::string> working_;
    std::optional<std::string> staged_;
    std::optional<std::string> latest_;
};

} // namespace minigit
