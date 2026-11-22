// TEST-1: Integration test for the minigit init command.

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>

namespace fs = std::filesystem;

namespace {

int failures = 0;

void check(bool condition, const char* message)
{
    if (!condition) {
        std::cerr << "FAILED: " << message << '\n';
        ++failures;
    }
}

class TestDirectory {
public:
    TestDirectory()
        : path_(fs::temp_directory_path() /
                ("minigit-test-1-" +
                 std::to_string(std::chrono::steady_clock::now()
                                    .time_since_epoch()
                                    .count())))
    {
        fs::create_directories(path_);
    }

    ~TestDirectory()
    {
        std::error_code error;
        fs::remove_all(path_, error);
    }

    const fs::path& path() const
    {
        return path_;
    }

private:
    fs::path path_;
};

std::string readText(const fs::path& file)
{
    std::ifstream input(file);
    return std::string(std::istreambuf_iterator<char>(input),
                       std::istreambuf_iterator<char>());
}

int runInit(const fs::path& executable)
{
    const std::string command =
        "\"" + executable.string() + "\" init";
    return std::system(command.c_str());
}

// HW12: Run minigit with the given arguments (may be empty) in the current
// folder, sending its error output to errorFile. Returns std::system's result:
// only "== 0" or "!= 0" is portable across Windows, Mac and Linux.
int runMinigit(const fs::path& executable,
               const std::string& arguments,
               const fs::path& errorFile)
{
    std::string command = "\"" + executable.string() + "\"";
    if (!arguments.empty()) {
        command += " " + arguments;
    }
    command += " 2> \"" + errorFile.string() + "\"";
#ifdef _WIN32
    // cmd.exe strips the first and last quote when a command has more than
    // two, so wrap the whole command once more to keep the inner quotes.
    command = "\"" + command + "\"";
#endif
    return std::system(command.c_str());
}

bool startsWithError(const fs::path& errorFile)
{
    return readText(errorFile).rfind("Error: ", 0) == 0;
}

class CurrentFolder {
public:
    explicit CurrentFolder(const fs::path& folder)
        : original_(fs::current_path())
    {
        fs::current_path(folder);
    }

    ~CurrentFolder()
    {
        std::error_code error;
        fs::current_path(original_, error);
    }

private:
    fs::path original_;
};

} // namespace

int main(int argc, char* argv[])
{
    if (argc != 2) {
        std::cerr << "Usage: init_integration_tests <minigit executable>\n";
        return 1;
    }

    const fs::path executable = argv[1];
    TestDirectory folder;
    const fs::path root = folder.path() / ".minigit";

    // TEST-1 : Run init in a temporary project folder.
    {
        CurrentFolder projectFolder(folder.path());
        check(runInit(executable) == 0,
              "minigit init should exit successfully");
    }

    check(fs::is_directory(root),
          "init should create .minigit");
    check(fs::is_directory(root / "stage"),
          "init should create stage");
    check(fs::is_directory(root / "commits"),
          "init should create commits");
    check(fs::is_regular_file(root / "HEAD"),
          "init should create HEAD");
    check(fs::is_regular_file(root / "index.txt"),
          "init should create index.txt");
    check(fs::is_regular_file(root / "next-id.txt"),
          "init should create next-id.txt");

    check(readText(root / "HEAD").empty(),
          "HEAD should start empty");
    check(readText(root / "index.txt").empty(),
          "index.txt should start empty");
    check(readText(root / "next-id.txt") == "1",
          "next-id.txt should contain 1");

    // TEST-1: Change repository files and an outside file, then run init again.
    std::ofstream(root / "HEAD") << "custom-head";
    std::ofstream(root / "index.txt") << "custom-index";
    std::ofstream(root / "next-id.txt") << "99";
    std::ofstream(root / "stage" / "keep.txt") << "keep";
    std::ofstream(folder.path() / "outside.txt") << "unchanged";

    {
        CurrentFolder projectFolder(folder.path());
        check(runInit(executable) == 0,
              "running init again should exit successfully");
    }

    check(readText(root / "HEAD") == "custom-head",
          "running init again should preserve HEAD");
    check(readText(root / "index.txt") == "custom-index",
          "running init again should preserve index.txt");
    check(readText(root / "next-id.txt") == "99",
          "running init again should preserve next-id.txt");
    check(readText(root / "stage" / "keep.txt") == "keep",
          "running init again should preserve stage files");
    check(readText(folder.path() / "outside.txt") == "unchanged",
          "running init should not change files outside .minigit");

    // HW12: No command (just minigit) prints "Error: ..." and fails.
    TestDirectory errors;
    const fs::path errorFile = errors.path() / "stderr.txt";
    {
        CurrentFolder projectFolder(folder.path());
        check(runMinigit(executable, "", errorFile) != 0,
              "minigit with no command should fail");
    }
    check(startsWithError(errorFile),
          "minigit with no command should print Error:");

    // HW12: Another command (status) outside a repository prints "Error: ..."
    // and fails, creating nothing.
    {
        TestDirectory empty;
        {
            CurrentFolder projectFolder(empty.path());
            check(runMinigit(executable, "status", errorFile) != 0,
                  "minigit status outside a repository should fail");
        }
        check(startsWithError(errorFile),
              "minigit status outside a repository should print Error:");
        check(fs::is_empty(empty.path()),
              "minigit status outside a repository should create nothing");
    }

    // HW12: A damaged .minigit (HEAD missing) prints "Error: ..." and fails,
    // without repairing anything.
    fs::remove(root / "HEAD");
    {
        CurrentFolder projectFolder(folder.path());
        check(runMinigit(executable, "init", errorFile) != 0,
              "minigit init in a damaged repository should fail");
    }
    check(startsWithError(errorFile),
          "minigit init in a damaged repository should print Error:");
    check(!fs::exists(root / "HEAD"),
          "minigit init should not recreate a missing HEAD");
    check(readText(root / "index.txt") == "custom-index",
          "minigit init should leave a damaged repository unchanged");

    if (failures > 0) {
        std::cerr << failures << " init integration check(s) failed\n";
        return 1;
    }

    std::cout << "Init integration test passed\n";
    return 0;
}
