# MiniGit

A minimal, self-contained Git-like version control system written in modern C++23.  
MiniGit implements the core workflows of Git from scratch — commits, branching, checkout, diff, and three-way merge — without any external dependencies.

---

## Features

| Feature | Description |
|---|---|
| `init` | Initialise a new repository in the current directory |
| `add` | Stage files for the next commit |
| `commit` | Snapshot staged files and record the commit |
| `log` | Show the commit history of the current branch |
| `status` | Show the current branch and staged files |
| `branch` | Create a new branch pointing at HEAD |
| `checkout` | Switch to a different branch |
| `diff` | Show line-level differences between HEAD and the working tree |
| `merge` | Three-way merge another branch into the current branch |

---

## How it Works

### Object Storage
Commits are serialised as binary blobs under `.mgit/commits/<sha256>.bin`.  
Each commit stores:
- A SHA-256 content hash (computed in pure C++, no OpenSSL required)
- The commit message, date, and time
- A full snapshot of every tracked file (path → content map)
- Parent commit IDs (one for regular commits, two for merge commits)

Branches are plain text files under `.mgit/refs/heads/<name>`, containing the SHA-256 of the tip commit.  
`HEAD` points to the current branch via a `ref: refs/heads/<name>` pointer.

### Diff Algorithm — Myers O(ND) Linear-Space
`myersDiff()` implements the divide-and-conquer linear-space refinement of Eugene Myers algorithm (Section 4b of "An O(ND) Difference Algorithm and Its Variations", 1986).

The algorithm uses two FurthestReaching arrays (forward and reverse) with negative-index wrapping, expands both search frontiers in lockstep, and returns as soon as an overlap is detected. This gives O(N*D) time and O(N) space.

The path is then converted into a sequence of Equal, Insert, and Delete hunks which are used by both `diff` (display) and `threeWayMerge`.

### Three-Way Merge
`threeWayMerge(base, ours, theirs)`:
1. Runs `myersDiff(base, ours)` and `myersDiff(base, theirs)` to get two independent edit lists.
2. Walks both edit lists in parallel, tracking a conflict region.
3. Expands the region greedily until no more overlapping edits remain.
4. If both sides produce the same output → auto-accepted (no conflict).
5. Otherwise → conflict markers emitted:

```
<<<<<<< ours
...ours version...
=======
...theirs version...
>>>>>>> theirs
```

### Merge Command
`mgit merge <branch>`:
- Detects fast-forward merges.
- Finds the merge base (LCA of commit DAGs via BFS).
- Performs per-file three-way merges.
- On clean merge: auto-commits with two parent IDs.
- On conflict: writes conflict markers to disk for manual resolution.

---

## Building

Requirements: GCC 12+ with C++23 support. No external libraries required.

```bash
# Build the mgit binary
mingw32-make        # Windows (MSYS2)
make                # Linux / macOS

# Run tests
mingw32-make test
```

---

## Usage

```bash
mgit init
mgit add file.txt
mgit add .
mgit commit "your message"
mgit log
mgit diff
mgit diff file.txt
mgit branch feature-x
mgit checkout feature-x
mgit merge main
```

---

## .mgitignore

```
*.o
*.exe
build/
```

---

## Project Structure

```
minigit/
├── include/
│   ├── core/         Commit.h, Repository.h, branch.hxx
│   └── fileUtils/    diff.h, hash.h, serialization.hxx, utils.h
├── src/
│   ├── main.cpp
│   ├── core/         Commit.cpp, Branch.cpp, Repository.cpp
│   └── fileUtils/    diff.cpp, hash.cpp, serialization.cpp, utils.cpp
├── tests/
│   ├── test_diff.cpp
│   └── test_merge.cpp
└── Makefile
```

---

## Known Limitations and Potential Improvements

| Area | Possible Improvement |
|---|---|
| Staging area | Implement a proper index with blob hashing |
| File storage | Content-addressed object store (deduplicate blobs) |
| Merge base | Recursive merge-base for criss-cross merges |
| Diff output | Add context lines to unified diff (default 3 like git) |
| Large files | Stream-based diff instead of loading all content to memory |
| Remotes | push/pull/fetch over SSH or HTTP |
| Tags | Lightweight and annotated tags |
| Rebase/cherry-pick | Replay commits onto a new base |
| Ignore patterns | Full .gitignore-style glob matching |

---

## Algorithm Reference

- Myers, E. W. (1986). An O(ND) Difference Algorithm and Its Variations. Algorithmica.
- Coglan, J. (2017). Myers diff in linear space: https://blog.jcoglan.com/2017/04/25/myers-diff-in-linear-space-theory/
- V8 JavaScript Engine liveedit-diff.cc (production reference).
