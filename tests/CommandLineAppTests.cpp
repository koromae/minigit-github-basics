// Tests for CLA-1 and CLA-2: the CommandLineApp constructor, run() and init.
// CLA-3 adds its tests to this file.

#include "CommandLineApp.hpp"

#include <chrono>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace {

int failures = 0;

void check(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAILED: " << message << '\n';
        ++failures;   // keep going, so cleanup at the end always runs
    }
}

bool contains(const std::string& text, const std::string& part) {
    return text.find(part) != std::string::npos;
}

// A fresh, empty folder that is removed when the test ends.
class TestDirectory {
public:
    TestDirectory()
        : path_(fs::temp_directory_path() /
                ("minigit-cla-" +
                 std::to_string(std::chrono::steady_clock::now()
                                    .time_since_epoch()
                                    .count()))) {
        fs::create_directories(path_);
    }

    ~TestDirectory() {
        std::error_code error;
        fs::remove_all(path_, error);
    }

    const fs::path& path() const { return path_; }

private:
    fs::path path_;
};

// CLA-1 test 1: no command prints usage to err, nothing to out, and returns 1.
void testNoCommandPrintsUsage() {
    TestDirectory folder;
    std::ostringstream out;
    std::ostringstream err;
    minigit::CommandLineApp app(folder.path(), out, err);

    const int code = app.run(std::vector<std::string>{});

    check(code == 1, "no command should return 1");
    check(contains(err.str(), "Usage"),
          "no command should print usage to err");
    check(out.str().empty(),
          "no command should print nothing to out");
}

// CLA-1 test 2: an unknown command prints an error and usage, and returns 1.
void testUnknownCommandIsRejected() {
    TestDirectory folder;
    std::ostringstream out;
    std::ostringstream err;
    minigit::CommandLineApp app(folder.path(), out, err);

    const int code = app.run(std::vector<std::string>{"banana"});

    check(code == 1, "an unknown command should return 1");
    check(contains(err.str(), "Error: unknown command banana"),
          "an unknown command should print the error to err");
    check(contains(err.str(), "Usage"),
          "an unknown command should print usage to err");
    check(out.str().empty(),
          "an unknown command should print nothing to out");
}

// CLA-1 test 3: constructing and running a bad command create nothing on disk.
void testCreatesNothingOnDisk() {
    TestDirectory folder;
    std::ostringstream out;
    std::ostringstream err;
    minigit::CommandLineApp app(folder.path(), out, err);

    app.run(std::vector<std::string>{});
    app.run(std::vector<std::string>{"banana"});

    check(!fs::exists(folder.path() / ".minigit"),
          "the constructor and usage errors should not create .minigit/");
    check(fs::is_empty(folder.path()),
          "the project folder should stay empty");
}

// CLA-2 test 1: init creates the repository, prints the message, returns 0.
void testInitCreatesRepository() {
    TestDirectory folder;
    std::ostringstream out;
    std::ostringstream err;
    minigit::CommandLineApp app(folder.path(), out, err);

    const int code = app.run(std::vector<std::string>{"init"});

    check(code == 0, "init should return 0");
    check(contains(out.str(), "Initialized empty MiniGit repository in"),
          "init should print the initialized message to out");
    check(contains(out.str(), ".minigit"),
          "init should print the .minigit path");
    check(err.str().empty(), "init should print nothing to err");

    const minigit::MiniGitRepository repo(folder.path());
    check(repo.isInitialized(), "init should create the complete layout");
}

// CLA-2 test 2: running init again is harmless and says so.
void testInitTwiceIsHarmless() {
    TestDirectory folder;
    std::ostringstream out;
    std::ostringstream err;
    minigit::CommandLineApp app(folder.path(), out, err);

    app.run(std::vector<std::string>{"init"});
    out.str("");

    const int code = app.run(std::vector<std::string>{"init"});

    check(code == 0, "a second init should return 0");
    check(contains(out.str(), "MiniGit repository already exists in"),
          "a second init should print the already-exists message");
    check(err.str().empty(), "a second init should print nothing to err");
}

// CLA-2 test 3: init with extra arguments is an error and creates nothing.
void testInitRejectsArguments() {
    TestDirectory folder;
    std::ostringstream out;
    std::ostringstream err;
    minigit::CommandLineApp app(folder.path(), out, err);

    const int code = app.run(std::vector<std::string>{"init", "extra"});

    check(code == 1, "init with arguments should return 1");
    check(contains(err.str(), "Error: init takes no arguments"),
          "init with arguments should print the error to err");
    check(out.str().empty(), "init with arguments should print nothing to out");
    check(!fs::exists(folder.path() / ".minigit"),
          "init with arguments should create nothing");
}

} // namespace

int main() {
    testNoCommandPrintsUsage();
    testUnknownCommandIsRejected();
    testCreatesNothingOnDisk();
    testInitCreatesRepository();
    testInitTwiceIsHarmless();
    testInitRejectsArguments();

    if (failures > 0) {
        std::cerr << failures << " CommandLineApp check(s) failed\n";
        return 1;
    }
    std::cout << "All CommandLineApp tests passed\n";
    return 0;
}