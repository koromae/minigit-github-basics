# MiniGit storage format

Everything MiniGit stores lives in `.minigit/` at the project root. `status` and `log` only read these files; they never change anything.

## Layout

```
.minigit/
├── HEAD               ID of the latest commit, empty if there is none
├── index.txt          tracked paths, one per line, sorted
├── next-id.txt        ID the next commit will get
├── stage/             copies of the files chosen by add
└── commits/
    └── <id>/
        ├── files/         saved copy of every tracked file
        ├── message.txt    the commit message, exactly as typed
        ├── parent.txt     ID of the previous commit, empty for the first
        ├── time.txt       when the commit was made
        └── index.txt      tracked paths at that commit
```

## Every file and folder

| Path | Type | What it holds | Created by | Changed by |
| --- | --- | --- | --- | --- |
| `.minigit/` | Folder | All repository data | `init` | — |
| `.minigit/HEAD` | File | ID of the latest commit, empty if there is none | `init` (empty) | `commit` |
| `.minigit/index.txt` | File | Tracked relative paths, one per line, sorted | `init` (empty) | `add` |
| `.minigit/next-id.txt` | File | ID the next commit will get | `init` (`1`) | `commit` |
| `.minigit/stage/` | Folder | Copies of the files chosen by `add`, at their relative paths | `init` (empty) | `add` copies in, `commit` clears it |
| `.minigit/commits/` | Folder | One folder per commit | `init` (empty) | `commit` |
| `.minigit/commits/<id>/files/` | Folder | Saved copy of every tracked file at that commit | `commit` | never |
| `.minigit/commits/<id>/message.txt` | File | The commit message, exactly as typed | `commit` | never |
| `.minigit/commits/<id>/parent.txt` | File | ID of the previous commit, empty for the first | `commit` | never |
| `.minigit/commits/<id>/time.txt` | File | When the commit was made | `commit` | never |
| `.minigit/commits/<id>/index.txt` | File | Tracked paths at that commit | `commit` | never |

## Rules

- `init` creates `.minigit/`, `stage/`, `commits/`, an empty `HEAD`, an empty `index.txt` and `next-id.txt` containing `1`.
- Running `init` in a folder that is already a valid repository prints a message and changes nothing.
- Running `init` when `.minigit/` exists but a part is missing (a damaged repository) prints `Error: ...`, exits 1 and changes nothing. MiniGit never repairs a damaged repository.
- A folder is a valid repository only if `.minigit/`, `stage/`, `commits/`, `HEAD`, `index.txt` and `next-id.txt` all exist.
- Commit IDs are whole numbers written as text: `1`, `2`, `3`, ...
- Files inside a commit are never changed after the commit is created.

## Where each path comes from

| Path | RepositoryPaths function |
| --- | --- |
| `.minigit/` | `metadataRoot()` |
| `.minigit/HEAD` | `headFile()` |
| `.minigit/index.txt` | `indexFile()` |
| `.minigit/next-id.txt` | `metadataRoot() / "next-id.txt"` |
| `.minigit/stage/` | `metadataRoot() / "stage"` (a file inside it: `stageFile(path)`) |
| `.minigit/commits/` | `metadataRoot() / "commits"` |
| `.minigit/commits/<id>/` | `checkpointDirectory(id)` |
| `.minigit/commits/<id>/files/<path>` | `commitFile(id, path)` |
