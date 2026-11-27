// Tests for MiniGitRepository: MR-1 (constructor), MR-2 to MR-4 (init and
// repository checks), MR-5a (checking a file to add) and MR-5b (addFile).

#include "MiniGitRepository.hpp"

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <iostream>
#include <string>
#include <fstream>
#include <iterator>
#include <stdexcept>
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

// A fresh, empty folder that is removed when the test ends.
class TestDirectory {
public:
    TestDirectory()
        : path_(fs::temp_directory_path() /
                ("minigit-mr1-" +
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

// MR-1 tests 1 and 2: the stored root is the one given, for a plain and a
// nested project root.
void testStoresProjectRoot() {
    const fs::path root = "course/repo";
    const fs::path nestedRoot = "course/team/repo";

    const minigit::MiniGitRepository repo(root);
    const minigit::MiniGitRepository nestedRepo(nestedRoot);

    check(repo.paths().projectRoot() == root,
          "paths().projectRoot() should be course/repo");
    check(nestedRepo.paths().projectRoot() == nestedRoot,
          "paths().projectRoot() should handle a nested root");
}

// MR-1 test 3: the repository locations are built from the root.
void testBuildsRepositoryLocations() {
    const fs::path root = "course/repo";
    const minigit::MiniGitRepository repo(root);

    check(repo.paths().metadataRoot() == root / ".minigit",
          "paths().metadataRoot() should be course/repo/.minigit");
    check(repo.paths().headFile() == root / ".minigit" / "HEAD",
          "paths().headFile() should be course/repo/.minigit/HEAD");
    check(repo.paths().indexFile() == root / ".minigit" / "index.txt",
          "paths().indexFile() should be course/repo/.minigit/index.txt");
}

// MR-1 test 4: constructing creates nothing on disk.
void testCreatesNothingOnDisk() {
    TestDirectory folder;
    const minigit::MiniGitRepository repo(folder.path());

    check(repo.paths().projectRoot() == folder.path(),
          "paths().projectRoot() should be the temporary folder");
    check(!fs::exists(folder.path() / ".minigit"),
          "the constructor should not create .minigit/");
    check(fs::is_empty(folder.path()),
          "the constructor should leave the project folder empty");
}

// MR-1 test 5: paths() returns the same stored object every time.
void testPathsIsStable() {
    const minigit::MiniGitRepository repo("course/repo");
    check(&repo.paths() == &repo.paths(),
          "paths() should return a reference to the stored RepositoryPaths");
}
// MR-2: An empty project is not an initialized repository.
void testEmptyProjectIsNotInitialized() {
    TestDirectory folder;
    minigit::MiniGitRepository repo(folder.path());

    check(!repo.isInitialized(),
          "An empty project should not be initialized");

    check(fs::is_empty(folder.path()),
          "isInitialized should create nothing");
}

// MR-2: All required directories and files must exist.
void testRequiredRepositoryParts() {
    TestDirectory folder;
    minigit::MiniGitRepository repo(folder.path());
    const auto root = repo.paths().metadataRoot();

    // Create the required directories.
    fs::create_directories(root / "stage");
    fs::create_directories(root / "commits");

    // Create the required files and close each stream.
    std::ofstream(root / "HEAD").close();
    std::ofstream(root / "index.txt").close();
    std::ofstream(root / "next-id.txt").close();

    check(repo.isInitialized(),
          "A complete layout should be initialized");

    // Remove each required part separately, then restore it.
    for (const char* name :
         {"stage", "commits", "HEAD", "index.txt", "next-id.txt"}) {
        const auto part = root / name;
        const bool directory = fs::is_directory(part);

        fs::remove(part);

        check(!repo.isInitialized(),
              "A missing required part should make the layout invalid");

        if (directory) {
            fs::create_directory(part);
        } else {
            std::ofstream(part).close();
        }
    }
}
// MR-3: Initialize a fresh repository with the correct contents.
void testInitializeCreatesLayout() {
    TestDirectory folder;
    minigit::MiniGitRepository repo(folder.path());
    const auto root = repo.paths().metadataRoot();

    check(repo.initialize(),
          "First initialization should return true");
    check(repo.isInitialized(),
          "Initialization should create the complete layout");

    check(fs::is_empty(root / "stage"),
          "Stage should start empty");
    check(fs::is_empty(root / "commits"),
          "Commits should start empty");
    check(fs::file_size(root / "HEAD") == 0,
          "HEAD should start empty");
    check(fs::file_size(root / "index.txt") == 0,
          "Index should start empty");

    std::ifstream input(root / "next-id.txt");
    std::string id;
    std::getline(input, id);
    check(id == "1", "Next commit ID should start at 1");

    check(!repo.initialize(),
          "Second initialization should return false");
}

// MR-3: A damaged layout (.minigit exists but parts are missing) is rejected
// and never repaired; existing contents stay exactly as they were (HW12).
void testInitializeRejectsDamagedRepository() {
    TestDirectory folder;
    minigit::MiniGitRepository repo(folder.path());
    const auto root = repo.paths().metadataRoot();

    fs::create_directories(root);

    // These streams close at the end of each statement.
    std::ofstream(root / "HEAD") << "7";
    std::ofstream(root / "index.txt") << "notes.txt";
    std::ofstream(root / "next-id.txt") << "8";

    std::string message;
    try {
        repo.initialize();
    } catch (const std::runtime_error& error) {
        message = error.what();
    }

    check(!message.empty(),
          "A damaged repository should make initialize() throw");
    check(message.find("stage") != std::string::npos,
          "The error should name the first missing part (stage)");
    check(!fs::exists(root / "stage") && !fs::exists(root / "commits"),
          "A damaged repository should not be repaired");

    std::string text;

    std::ifstream head(root / "HEAD");
    std::getline(head, text);
    check(text == "7", "Existing HEAD should be unchanged");

    std::ifstream index(root / "index.txt");
    std::getline(index, text);
    check(text == "notes.txt", "Existing index should be unchanged");

    std::ifstream next(root / "next-id.txt");
    std::getline(next, text);
    check(text == "8", "Existing next ID should be unchanged");
}

// MR-3: A complete repository with HEAD deleted is damaged too.
void testInitializeRejectsMissingHead() {
    TestDirectory folder;
    minigit::MiniGitRepository repo(folder.path());

    repo.initialize();
    fs::remove(repo.paths().headFile());

    std::string message;
    try {
        repo.initialize();
    } catch (const std::runtime_error& error) {
        message = error.what();
    }

    check(message.find("HEAD") != std::string::npos,
          "A missing HEAD should be reported by name");
    check(!fs::exists(repo.paths().headFile()),
          "initialize() should not recreate a missing HEAD");
}

// MR-4: Reject an empty or incomplete repository; accept a valid one.
void testRequireRepository() {
    TestDirectory folder;
    minigit::MiniGitRepository repo(folder.path());

    bool threw = false;
    try {
        repo.requireRepository();
    } catch (const std::runtime_error&) {
        threw = true;
    }

    check(threw, "An empty project should be rejected");
    check(fs::is_empty(folder.path()),
          "requireRepository should create nothing");

    repo.initialize();

    try {
        repo.requireRepository();
    } catch (...) {
        check(false, "A valid repository should be accepted");
    }

    fs::remove(repo.paths().headFile());

    threw = false;
    try {
        repo.requireRepository();
    } catch (const std::runtime_error&) {
        threw = true;
    }

    check(threw, "An incomplete repository should be rejected");
    check(!fs::exists(repo.paths().headFile()),
          "requireRepository should not recreate HEAD");
}


} // namespace

// MR-5a helper: create a file with some text, making parent folders.
void writeFile(const fs::path& file, const std::string& text) {
    fs::create_directories(file.parent_path());
    std::ofstream output(file, std::ios::binary);
    output << text;
}

// MR-5a helper: the error message checkFileToAdd() throws, or "" if none.
std::string addError(const minigit::MiniGitRepository& repo,
                     const std::string& argument) {
    try {
        repo.checkFileToAdd(argument);
    } catch (const std::runtime_error& error) {
        return error.what();
    }
    return "";
}

// MR-5a test 1: a file in the project returns its clean relative path,
// including a file in a subfolder (with / separators).
void testCheckFileToAddAcceptsFiles() {
    TestDirectory folder;
    const minigit::MiniGitRepository repo(folder.path());
    writeFile(folder.path() / "notes.txt", "hello");
    writeFile(folder.path() / "docs" / "a.txt", "nested");

    check(repo.checkFileToAdd("notes.txt") == "notes.txt",
          "checkFileToAdd() should return notes.txt");
    check(repo.checkFileToAdd("docs/a.txt") == "docs/a.txt",
          "checkFileToAdd() should return docs/a.txt with /");
}

// MR-5a test 2: messy paths are cleaned before they are checked.
void testCheckFileToAddCleansPaths() {
    TestDirectory folder;
    const minigit::MiniGitRepository repo(folder.path());
    writeFile(folder.path() / "notes.txt", "hello");
    writeFile(folder.path() / "docs" / "a.txt", "nested");

    check(repo.checkFileToAdd("./notes.txt") == "notes.txt",
          "./notes.txt should be cleaned to notes.txt");
    check(repo.checkFileToAdd("docs/../notes.txt") == "notes.txt",
          "docs/../notes.txt should be cleaned to notes.txt");
    check(repo.checkFileToAdd("docs//a.txt") == "docs/a.txt",
          "docs//a.txt should be cleaned to docs/a.txt");
}

// MR-5a test 3: a missing file and a folder are rejected with clear errors.
void testCheckFileToAddRejectsMissingAndFolders() {
    TestDirectory folder;
    const minigit::MiniGitRepository repo(folder.path());
    fs::create_directories(folder.path() / "docs");

    const std::string missing = addError(repo, "missing.txt");
    check(missing.find("File does not exist") != std::string::npos,
          "A missing file should report File does not exist");
    check(missing.find("missing.txt") != std::string::npos,
          "The missing-file error should name the file");

    const std::string folderError = addError(repo, "docs");
    check(folderError.find("Cannot add a folder") != std::string::npos,
          "A folder should report Cannot add a folder");
}

// MR-5a test 4: empty, absolute, outside-the-project and .minigit paths are
// rejected, even when the file really exists.
void testCheckFileToAddRejectsUnsafePaths() {
    TestDirectory folder;
    minigit::MiniGitRepository repo(folder.path());
    repo.initialize();
    writeFile(folder.path() / "notes.txt", "hello");

    check(!addError(repo, "").empty(), "An empty path should be rejected");
    check(!addError(repo, "../outside.txt").empty(),
          "A path outside the project should be rejected");
    check(!addError(repo, (folder.path() / "notes.txt").string()).empty(),
          "An absolute path should be rejected");
    check(!addError(repo, ".minigit/HEAD").empty(),
          "A path inside .minigit should be rejected");
    check(!addError(repo, "docs/../.minigit/HEAD").empty(),
          "A path that cleans to .minigit should be rejected");
}

// MR-5a test 5: checking never creates or changes anything, even when it
// succeeds; no .minigit, stage copy or index entry appears.
void testCheckFileToAddChangesNothing() {
    TestDirectory folder;
    const minigit::MiniGitRepository repo(folder.path());
    writeFile(folder.path() / "notes.txt", "hello");

    repo.checkFileToAdd("notes.txt");
    addError(repo, "missing.txt");

    int entries = 0;
    for (const auto& entry : fs::directory_iterator(folder.path())) {
        (void)entry;
        ++entries;
    }
    check(entries == 1, "checkFileToAdd() should leave only notes.txt");
    check(!fs::exists(folder.path() / ".minigit"),
          "checkFileToAdd() should not create .minigit");
}

// MR-5b helper: the whole content of a file, or "" if it does not exist.
std::string fileText(const fs::path& file) {
    std::ifstream input(file, std::ios::binary);
    return std::string(std::istreambuf_iterator<char>(input),
                       std::istreambuf_iterator<char>());
}

// MR-5b helper: every file and folder under a folder, as sorted generic paths.
std::vector<std::string> listAll(const fs::path& folder) {
    std::vector<std::string> paths;
    if (fs::exists(folder)) {
        for (const auto& entry : fs::recursive_directory_iterator(folder)) {
            paths.push_back(fs::relative(entry.path(), folder).generic_string());
        }
    }
    std::sort(paths.begin(), paths.end());
    return paths;
}

// MR-5b test 1: a new file is copied to the Stage and its path added to the
// index; addFile() returns the clean path.
void testAddFileNewFile() {
    TestDirectory folder;
    minigit::MiniGitRepository repo(folder.path());
    repo.initialize();
    writeFile(folder.path() / "notes.txt", "hello");

    check(repo.addFile("notes.txt") == "notes.txt",
          "addFile() should return notes.txt");
    check(fileText(repo.paths().stageFile("notes.txt")) == "hello",
          "addFile() should copy the file into the Stage");
    check(fileText(repo.paths().indexFile()) == "notes.txt\n",
          "addFile() should add notes.txt to the index");
    check(fileText(folder.path() / "notes.txt") == "hello",
          "addFile() should not change the original file");
}

// MR-5b test 2: adding the same file again replaces the staged copy and does
// not duplicate the index entry; a changed file stages its new content.
void testAddFileAgain() {
    TestDirectory folder;
    minigit::MiniGitRepository repo(folder.path());
    repo.initialize();
    writeFile(folder.path() / "notes.txt", "first");

    repo.addFile("notes.txt");
    repo.addFile("notes.txt");
    check(fileText(repo.paths().indexFile()) == "notes.txt\n",
          "Adding the same file twice should not duplicate the index entry");

    writeFile(folder.path() / "notes.txt", "second");
    repo.addFile("./notes.txt");
    check(fileText(repo.paths().stageFile("notes.txt")) == "second",
          "Adding a changed file should replace the staged copy");
    check(fileText(repo.paths().indexFile()) == "notes.txt\n",
          "Adding a changed file should leave the index unchanged");
}

// MR-5b test 3: a file in a subfolder is staged at the same relative path,
// and the index stays sorted with / separators.
void testAddFileSubfolderAndSorting() {
    TestDirectory folder;
    minigit::MiniGitRepository repo(folder.path());
    repo.initialize();
    writeFile(folder.path() / "z.txt", "z");
    writeFile(folder.path() / "docs" / "a.txt", "nested");
    writeFile(folder.path() / "a.txt", "a");

    repo.addFile("z.txt");
    check(repo.addFile("docs/a.txt") == "docs/a.txt",
          "addFile() should return docs/a.txt with /");
    repo.addFile("a.txt");

    check(fileText(repo.paths().stageFile("docs/a.txt")) == "nested",
          "A subfolder file should be staged at stage/docs/a.txt");
    check(fileText(repo.paths().indexFile()) == "a.txt\ndocs/a.txt\nz.txt\n",
          "The index should stay sorted with / separators");
}

// MR-5b test 4: every error case throws and changes nothing in .minigit.
void testAddFileErrorsChangeNothing() {
    TestDirectory folder;
    minigit::MiniGitRepository repo(folder.path());
    repo.initialize();
    writeFile(folder.path() / "notes.txt", "hello");
    fs::create_directories(folder.path() / "docs");
    repo.addFile("notes.txt");

    const auto before = listAll(repo.paths().metadataRoot());
    const std::string indexBefore = fileText(repo.paths().indexFile());

    const std::string badPaths[] = {
        "", "missing.txt", "docs", "../outside.txt", ".minigit/HEAD"};
    for (const auto& path : badPaths) {
        bool threw = false;
        try {
            repo.addFile(path);
        } catch (const std::runtime_error&) {
            threw = true;
        }
        check(threw, "addFile() should throw for a bad path");
    }

    check(listAll(repo.paths().metadataRoot()) == before,
          "A failed addFile() should not create or remove anything in .minigit");
    check(fileText(repo.paths().indexFile()) == indexBefore,
          "A failed addFile() should not change the index");
}

// MR-5b test 5: outside a repository, addFile() throws and creates nothing.
void testAddFileNeedsRepository() {
    TestDirectory folder;
    minigit::MiniGitRepository repo(folder.path());
    writeFile(folder.path() / "notes.txt", "hello");

    std::string message;
    try {
        repo.addFile("notes.txt");
    } catch (const std::runtime_error& error) {
        message = error.what();
    }

    check(message.find("Not a valid MiniGit repository") != std::string::npos,
          "addFile() outside a repository should say so");
    check(!fs::exists(folder.path() / ".minigit"),
          "addFile() outside a repository should not create .minigit");
}

int main() {
    testStoresProjectRoot();
    testBuildsRepositoryLocations();
    testCreatesNothingOnDisk();
    testPathsIsStable();
    testEmptyProjectIsNotInitialized();
    testRequiredRepositoryParts();
    testInitializeCreatesLayout();
    testInitializeRejectsDamagedRepository();
    testInitializeRejectsMissingHead();
    testRequireRepository();
    testCheckFileToAddAcceptsFiles();
    testCheckFileToAddCleansPaths();
    testCheckFileToAddRejectsMissingAndFolders();
    testCheckFileToAddRejectsUnsafePaths();
    testCheckFileToAddChangesNothing();
    testAddFileNewFile();
    testAddFileAgain();
    testAddFileSubfolderAndSorting();
    testAddFileErrorsChangeNothing();
    testAddFileNeedsRepository();

    if (failures > 0) {
        std::cerr << failures << " MiniGitRepository check(s) failed\n";
        return 1;
    }
    std::cout << "All MiniGitRepository tests passed\n";
    return 0;
}
