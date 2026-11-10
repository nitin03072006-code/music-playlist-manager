# Music Playlist Manager Using a Doubly Circular Linked List

A menu-driven C++17 console application that manages a music playlist. The playlist is stored in a **doubly circular linked list** written from scratch. There are no arrays, vectors or STL containers holding the songs.

> **Student:** _your name_ · **Roll no.:** _your roll number_ · **Course / Subject:** Data Structures and Algorithms · **College:** _your college_

---

## Problem statement

A music player must let the user build a playlist, move to the next or previous song, and keep going round the list when it reaches either end. Design and implement a playlist manager that stores each song's **ID, title, artist and duration** in a *doubly circular linked list* and supports adding, removing, searching, selecting and navigating songs, with safe handling of empty and single-song playlists.

## Objectives

* Show a practical, real-world use of a circular linked list.
* Implement insertion, deletion, searching, traversal and navigation **manually** with pointers.
* Keep the circular links correct after every insertion and deletion.
* Allocate nodes with `new`, free them with `delete`, and leave no memory leaks.
* Handle bad input without crashing.

## Data structure used

**Doubly circular linked list**: every node stores a song and two pointers, `next` and `prev`.

```
        +------+  next   +------+  next   +------+
   +--> | S1   | ------> | S2   | ------> | S3   | ---+
   |    |      | <------ |      | <------ |      |    |
   |    +------+  prev   +------+  prev   +------+    |
   |       ^ head                                      |
   |       +---- S1->prev points to the LAST node -----+
   +-------------- S3->next points to the FIRST node
```

* last node's `next` -> first node, and first node's `prev` -> last node
* `head` = first song, `current` = selected song, `count` = number of songs
* the last song is always `head->prev`, so adding at the end is O(1) and no `tail` pointer is needed

| Class | Role |
|---|---|
| `SongNode` | One song (ID, title, artist, duration in seconds) plus `next` and `prev` pointers |
| `MusicPlaylist` | Owns `head`, `current`, `count` and contains all the linked-list logic |

The menu functions only call `MusicPlaylist` methods. They never touch the pointers.

## Features

| # | Menu option | What it does |
|---|---|---|
| 1 | Add Song | Adds a song at the end. Duplicate IDs are rejected. |
| 2 | Remove Song | Deletes by ID or by title. |
| 3 | Display Playlist | Shows all songs; `>>` marks the current song. |
| 4 | Play Next Song | Selects the next song; after the last it wraps to the first. |
| 5 | Play Previous Song | Selects the previous song; before the first it wraps to the last. |
| 6 | Search Song | Finds a song by ID or by title (case-insensitive). |
| 7 | Select Song | Makes a song the current song. |
| 8 | Display Current Song | Shows the selected song and its song number. |
| 9 | Display Total Songs | Shows the song count and the total playing time. |
| 10 | Repeat Playlist | Plays N songs in a row, going round the circle. |
| 11 | Delete All Songs | Frees every node and resets the playlist (asks y/n first). |
| 12 | Exit | Leaves the program and releases all memory. |

Also included: clear messages for empty playlists and missing songs, wrap-around notices, and validation of menu choices, IDs, text fields and `mm:ss` durations.

## Project files

```
music-playlist-manager/
├── main.cpp              the complete program
├── PROJECT_GUIDE.md      architecture, function explanations, sample I/O, complexity, test cases, viva Q&A
├── README.md             this file
└── tests/
    └── unit_tests.cpp    optional automated tests
```

## How to run

**Requirements:** a C++17 compiler (MinGW-w64 `g++` 11 or newer, installed through MSYS2) and, optionally, VS Code with the Microsoft **C/C++** extension.

### From a terminal (works anywhere)

```
g++ -std=c++17 -Wall -Wextra -g main.cpp -o playlist.exe
.\playlist.exe              (PowerShell)
playlist                    (Command Prompt)
```

On Linux or macOS: `g++ -std=c++17 -Wall -Wextra -g main.cpp -o playlist` then `./playlist`.

### In VS Code

1. *File -> Open Folder...* and choose this folder.
2. Add a build task (full `tasks.json` and the MinGW installation steps are in `PROJECT_GUIDE.md`, section 4), then press `Ctrl+Shift+B`.
3. Run `.\playlist.exe` in the **integrated terminal**. Do not use an Output-panel runner, because the program needs keyboard input.

### Optional automated tests

```
g++ -std=c++17 -Wall -Wextra -g tests/unit_tests.cpp -o unit_tests.exe
.\unit_tests.exe
```

Expected last line: `==== 83 passed, 0 failed ====`

## Time complexity (n = number of songs)

| Operation | Time |
|---|---|
| Insert at end (pointer work) | O(1) |
| Add a song (includes the duplicate-ID check) | O(n) |
| Delete a node you already hold | O(1) |
| Remove by ID or title (search + unlink) | O(n) |
| Search by ID or title | O(n) |
| Traversal / display | O(n) |
| Next / previous song | O(1) |
| Total songs | O(1) |

## Testing summary

* 28 menu-level test cases (normal use, edge cases, invalid input) were run against the compiled program. All pass.
* 83 direct checks of the linked-list class plus 20,000 random operations, with the ring integrity verified after every step. All pass.
* Built with `-Wall -Wextra -pedantic` (no warnings). Run under AddressSanitizer, UBSan and Valgrind: no errors and no leaks (every `new` matched by a `delete`).
* Compiled and tested with `g++` 13.3 on Linux. The program uses only standard C++17, so it should build unchanged with MinGW-w64.

## Limitations

The playlist lives in memory only (it is not saved to a file). Songs are kept in the order they were added. Repeat mode plays a chosen number of songs so the console never hangs.
