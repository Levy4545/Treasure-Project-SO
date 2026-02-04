# Treasure Hunt Management System

A C-based treasure hunt management system that demonstrates advanced operating system concepts including process management, inter-process communication (IPC), signals, file I/O, and system calls.

## 📋 Table of Contents

- [Overview](#overview)
- [Features](#features)
- [System Requirements](#system-requirements)
- [Installation](#installation)
- [Usage](#usage)
  - [Treasure Manager](#treasure-manager)
  - [Treasure Hub](#treasure-hub)
- [Project Structure](#project-structure)
- [Key Operating System Concepts](#key-operating-system-concepts)
- [Build Information](#build-information)
- [Contributing](#contributing)

## 🎯 Overview

This project is a System and Operating Systems (SO) coursework that implements a treasure hunt tracking system. It manages multiple treasure hunts, each containing treasures with geographic coordinates, clues, and point values. The system demonstrates practical implementation of various OS concepts including:

- Process creation and management (fork, exec)
- Inter-process communication using pipes
- Signal handling for process coordination
- File I/O operations using system calls
- File system operations (directories, symlinks)

## ✨ Features

### Treasure Management
- **Add Treasures**: Create treasures with ID, username, coordinates (latitude/longitude), clues, and point values
- **List Treasures**: View all treasures in a hunt with metadata (file size, last modified time)
- **View Treasure**: Display detailed information about specific treasures
- **Remove Treasures**: Delete individual treasures from a hunt
- **Remove Hunts**: Delete entire treasure hunt directories

### Monitoring System
- **Background Monitor Process**: Fork-based monitoring process that runs independently
- **Signal-Based Communication**: Uses UNIX signals (SIGUSR1, SIGUSR2, SIGTERM, SIGHUP) for IPC
- **Real-time Hunt Monitoring**: List all active hunts and their treasure counts
- **Treasure Inspection**: View treasure details through the monitor process

### Scoring System
- **Score Calculation**: Aggregate scores for all users across all hunts
- **Multi-Process Execution**: Uses fork/exec to calculate scores for each hunt in parallel
- **Pipe-Based Output**: Parent process collects results from child processes via pipes

### Logging System
- **Operation Logging**: Automatic logging of all operations with timestamps
- **Symbolic Links**: Creates hunt-specific log file symlinks for easy access

## 💻 System Requirements

- **Operating System**: Linux/Unix-based system
- **Compiler**: GCC (GNU Compiler Collection)
- **Make**: GNU Make build system

## 🔧 Installation

1. Clone the repository:
```bash
git clone https://github.com/Levy4545/Treasure-Project-SO.git
cd Treasure-Project-SO
```

2. Build the project:
```bash
make
```

This will compile three executables:
- `treasure_manager` - Command-line tool for managing treasures
- `treasure_hub` - Interactive hub for monitoring and scoring
- `calculate_score_exec` - Score calculation utility

3. Clean build artifacts (optional):
```bash
make clean
```

## 📖 Usage

### Treasure Manager

The `treasure_manager` executable provides command-line interface for treasure operations:

#### Add a Treasure
```bash
./treasure_manager --add <hunt_id>
```
Interactive prompts will ask for:
- Treasure ID (integer)
- Username (max 32 characters)
- Latitude (-90 to 90)
- Longitude (-180 to 180)
- Clue (max 128 characters)
- Value (points, positive integer)

#### List Treasures
```bash
./treasure_manager --list <hunt_id>
```

#### View Specific Treasure
```bash
./treasure_manager --view <hunt_id> <treasure_id>
```

#### Remove a Treasure
```bash
./treasure_manager --remove_treasure <hunt_id> <treasure_id>
```

#### Remove an Entire Hunt
```bash
./treasure_manager --remove_hunt <hunt_id>
```

### Treasure Hub

The `treasure_hub` provides an interactive shell for monitoring and managing hunts:

```bash
./treasure_hub
```

Available commands:
- `start_monitor` - Start the background monitoring process
- `list_hunts` - Display all hunts and treasure counts (requires monitor)
- `list_treasures <hunt_id>` - List all treasures in a hunt (requires monitor)
- `view_treasure <hunt_id> <treasure_id>` - View treasure details (requires monitor)
- `stop_monitor` - Stop the background monitor process
- `calculate_score` - Calculate and display scores for all users across all hunts
- `exit` - Exit the hub (monitor must be stopped first)

#### Example Session
```
Welcome to Treasure Hub.
hub> start_monitor
[Monitor] Ready.
hub> list_hunts
[Monitor] Hunts found:
hunt1: 5 treasures
hunt2: 3 treasures
hub> list_treasures hunt1
[Monitor] Treasures in hunt1:
ID: 1 | Alice | 100 pts
ID: 2 | Bob | 50 pts
...
hub> calculate_score
--- Scores for hunt1 ---
Alice: 250
Bob: 150
--- Scores for hunt2 ---
Charlie: 300
hub> stop_monitor
[Monitor] Exiting...
hub> exit
```

## 📁 Project Structure

```
Treasure-Project-SO/
├── treasure.h              # Header file with structure definitions and function declarations
├── treasure.c              # Core treasure management functions implementation
├── treasure_manager.c      # Command-line interface for treasure operations
├── treasure_hub.c          # Interactive hub with monitoring and scoring
├── calculate_score_exec.c  # Score calculation utility (called by hub)
├── Makefile                # Build configuration
└── README.md               # This file
```

### Data Structure

Each treasure is stored as a binary structure:
```c
typedef struct {
    int id;                      // Unique identifier
    char username[32];           // User who found/created the treasure
    float latitude;              // Geographic latitude
    float longitude;             // Geographic longitude
    char clue[128];              // Treasure clue
    int value;                   // Point value
} Treasure;
```

### File Organization

Each hunt creates a directory structure:
```
<hunt_id>/
├── treasures.dat            # Binary file storing treasure data
└── logged_hunt              # Operation log file
logged_hunt-<hunt_id>        # Symbolic link to log file
```

## 🔑 Key Operating System Concepts

This project demonstrates the following OS concepts:

1. **Process Management**
   - `fork()`: Creating child processes for monitoring and score calculation
   - `exec()`: Replacing process image for score calculation
   - `waitpid()`: Parent process waiting for child completion

2. **Inter-Process Communication**
   - **Pipes**: Bi-directional communication between hub and monitor
   - **Signals**: Asynchronous communication using SIGUSR1, SIGUSR2, SIGTERM, SIGHUP
   - **Files**: Shared command file for passing parameters to monitor

3. **File I/O**
   - Low-level system calls: `open()`, `read()`, `write()`, `close()`
   - File descriptors and file duplication (`dup2()`)
   - Binary file operations for treasure storage

4. **File System Operations**
   - Directory operations: `mkdir()`, `rmdir()`, `opendir()`, `readdir()`
   - Symbolic links: `symlink()`
   - File metadata: `stat()`, `fstat()`

5. **Signal Handling**
   - Signal handlers using `sigaction()`
   - Signal masks and custom signal handlers
   - Process termination via signals

6. **Synchronization**
   - `select()` for non-blocking I/O on pipes
   - Signal-based synchronization between processes

## 🛠 Build Information

The project uses a Makefile with the following targets:

- `make` or `make all`: Builds all executables
- `make treasure_manager`: Builds only the manager
- `make treasure_hub`: Builds only the hub
- `make calculate_score_exec`: Builds only the score calculator
- `make clean`: Removes all executables and temporary files

### Compilation Flags
- `-Wall`: Enable all compiler warnings
- GCC standard compilation with no optimization flags

## 🤝 Contributing

This is a System and Operating Systems (SO) coursework project. Updates are made regularly, typically every Sunday.

For questions or suggestions, please open an issue in the repository.

## 📝 Notes

- All treasures are validated before storage (ID >= 0, valid coordinates, value >= 0)
- The monitor process must be started before using monitor-dependent commands
- The monitor process must be stopped before exiting the hub
- Binary treasure data files use the native system's endianness
- Log files use human-readable timestamps

## 🔒 Safety Features

- Input validation for all treasure data
- Proper error handling for file operations
- Safe signal handling with proper cleanup
- Process cleanup on exit
- Atomic file operations using temporary files

---

**Course**: System and Operating Systems  
**Last Updated**: Check commit history for latest updates