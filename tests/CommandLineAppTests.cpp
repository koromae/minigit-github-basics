// Tests for CLA-1: the CommandLineApp constructor and run().
// CLA-2 and CLA-3 add their tests to this file.

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
                ("minigit-cla1-" +
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

} // namespace

int main() {
    testNoCommandPrintsUsage();
    testUnknownCommandIsRejected();
    testCreatesNothingOnDisk();

    if (failures > 0) {
        std::cerr << failures << " CommandLineApp check(s) failed\n";
        return 1;
    }
    std::cout << "All CommandLineApp tests passed\n";
    return 0;
}