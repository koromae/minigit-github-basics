cmake_minimum_required(VERSION 3.20)
project(MiniGitWeek1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# FS-17: Add the file-reading code to the library.
add_library(minigit_paths
    src/PathLab.cpp
    src/RepositoryPaths.cpp
    src/FileSystem.cpp
    # MR-1: MiniGitRepository, the class that runs each command.
    src/MiniGitRepository.cpp)
target_include_directories(minigit_paths PUBLIC include)

# BUILD-1: The minigit program. main.cpp is the entry point (CLA-4);
# every class lives in minigit_paths. New class sources (CommandLineApp.cpp,
# IndexTable.cpp, ...) go in the add_library list above, not here.
add_executable(minigit src/main.cpp)
target_link_libraries(minigit PRIVATE minigit_paths)

include(CTest)
if(BUILD_TESTING)
    add_executable(path_lab_tests tests/PathLabTests.cpp)
    target_link_libraries(path_lab_tests PRIVATE minigit_paths)
    add_test(NAME path_lab_tests COMMAND path_lab_tests)

    add_executable(repository_paths_tests tests/RepositoryPathsTests.cpp)
    target_link_libraries(repository_paths_tests PRIVATE minigit_paths)
    add_test(NAME repository_paths_tests COMMAND repository_paths_tests)

    # FS-17: Build and run the file-reading tests.
    add_executable(filesystem_tests tests/FileSystemTests.cpp)
    target_link_libraries(filesystem_tests PRIVATE minigit_paths)
    add_test(NAME filesystem_tests COMMAND filesystem_tests)

    # RP-14 (#32) and RP-15 (#34): Build and run the workingFile() and
    # commitFile() tests, so CTest checks them before every push.
    add_executable(working_and_commit_file_tests tests/WorkingAndCommitFileTests.cpp)
    target_link_libraries(working_and_commit_file_tests PRIVATE minigit_paths)
    add_test(NAME working_and_commit_file_tests COMMAND working_and_commit_file_tests)

    # HDR-1: Check that every M2-M5 interface header compiles.
    add_executable(header_compile_tests tests/HeaderCompileTests.cpp)
    target_link_libraries(header_compile_tests PRIVATE minigit_paths)
    add_test(NAME header_compile_tests COMMAND header_compile_tests)

    # MR-1: MiniGitRepository tests. MR-2 to MR-4 add their checks here too.
    add_executable(minigit_repository_tests tests/MiniGitRepositoryTests.cpp)
    target_link_libraries(minigit_repository_tests PRIVATE minigit_paths)
    add_test(NAME minigit_repository_tests COMMAND minigit_repository_tests)
endif()
