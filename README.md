# minigit

**minigit** is a minimal, custom Version Control System (VCS) heavily inspired by Git. It is written in C++23 and provides basic functionalities for version tracking, branching, and logging, built from scratch as an educational project in object-oriented programming.

## 🚀 Project Purpose

The primary purpose of **minigit** is to demonstrate a foundational understanding of Version Control Systems. It implements key concepts like directory snapshots, hashing, commit tracking, and branch management without relying on the actual `.git` ecosystem. It takes a lightweight approach to track files, manage state, and navigate history locally.

## 📈 Current Progress

**minigit** currently supports a robust set of core VCS capabilities:

- **Repository Initialization**: Set up a custom `.mgit` tracking directory.
- **Commit Management**: Create file snapshots and save them to the repository history with commit messages.
- **History Logging**: View the history of commits on the current branch.
- **Branching System**: Create new branches to diverge development lines.
- **Checking Out**: Switch between different branches and update the working directory to match the target branch state.

### Implemented Commands:
- `mgit init`
- `mgit add <file>`
- `mgit commit <message>`
- `mgit log`
- `mgit branch <branch_name>`
- `mgit checkout <branch_name>`
- `mgit show <commit_id> <filename>`
- `mgit merge <branch_name>`

## 📁 File Structure

The project is structured into clear object-oriented modules:

```text
├── .gitignore
├── .mgitignore          # Custom ignore file for the minigit VCS
├── Makefile             # Build instructions for GNU Make
├── include/
│   ├── core/            # Header files for core VCS logic (Repository, Commit, Branch)
│   └── fileUtils/       # Header files for hashing and file I/O utilities
│
└── src/
    ├── main.cpp         # CLI application entry point and command router
    ├── core/
    │   ├── Branch.cpp     # Branch creation and manipulation logic
    │   ├── Commit.cpp     # Snapshot generation and commit structures
    │   └── Repository.cpp # General repository orchestration (.mgit creation, routing)
    └── fileUtils/
        ├── hash.cpp       # Hashing logic (using OpenSSL)
        └── utils.cpp      # Generic string, time, and file I/O helpers
```

## 🛠️ Setup & Execution

### Prerequisites

To compile and run **minigit**, your environment must have:
- A C++ Compiler supporting **C++23** (e.g., GCC 13+ / g++)
- **OpenSSL** development libraries installed (`-lssl`, `-lcrypto`).
  - *Ubuntu/Debian:* `sudo apt install libssl-dev`
  - *Windows (MSYS2/MinGW):* `pacman -S mingw-w64-x86_64-openssl`
- **Make** build system (`GNU Make`)

### Building the Project (Optional)

**Windows Note:** A pre-compiled `mgit.exe` is already included in the root directory. You can skip the build step and run it directly!

If you wish to compile it yourself from source:
1. Open your terminal (e.g. bash, WSL, or MSYS2) and navigate to the project root directory.
2. Compile the project using Make. 
   - **For MSYS2/MinGW Windows users:** Run `mingw32-make` instead of `make`.
   - **For Linux/macOS/WSL users:** Run `make`.
   ```bash
   make # or mingw32-make
   ```
3. This process binds all the objects and outputs an executable named `mgit` (or `mgit.exe` on Windows).

> **Note:** To clean previous builds and object files, you can run `make clean`.

### Running minigit

Once built (or using the provided `mgit.exe`), interact with `minigit` exactly like you would with git through your command line or PowerShell!

```powershell
# 1. Initialize an empty minigit repository
.\mgit.exe init

# 2. Stage files to be committed (Required!)
.\mgit.exe add .
# (or stage a specific file like .\mgit.exe add strings.txt)

# 3. Perform a commit
# Note: Ensure you put your message in quotes
.\mgit.exe commit "Initial commit"

# 4. View the git logs
.\mgit.exe log

# 5. Create a new branch
.\mgit.exe branch feature-gui

# 6. Switch out to the newly created branch
.\mgit.exe checkout feature-gui
```

*(If you are on Linux/macOS, use `./mgit` instead of `.\mgit.exe`)*

## ⚠️ Common Issues & Troubleshooting

- **"No files staged" error on commit:** Like the real Git, `minigit` uses a staging area. If you simply run `commit` with a new file, it won't know about it. You **must** run `.\mgit.exe add <filename>` (or `.\mgit.exe add .` to stage everything) before running your commit command.
- **`make` command not found (Windows / MSYS2 / MinGW):** If you are compiling from source on Windows using MSYS2/MinGW, the make utility is often named `mingw32-make`. Run `mingw32-make` instead of `make`.
