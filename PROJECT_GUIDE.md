# Music Playlist Manager Using a Doubly Circular Linked List
### Project Guide: architecture, function explanations, how to run, sample output, complexity, test cases, viva Q&A

> Language: **C++17** · Files: `main.cpp` (the whole program) and `tests/unit_tests.cpp` (optional automated tests) · Tested with `g++` 13.3 using `-Wall -Wextra -pedantic` (zero warnings), AddressSanitizer, UBSan and Valgrind (zero errors, zero leaks).

---

## Contents

1. [Project architecture](#1-project-architecture)
2. [How the doubly circular linked list works](#2-how-the-doubly-circular-linked-list-works)
3. [Explanation of every major function](#3-explanation-of-every-major-function)
4. [Compile and run in VS Code with MinGW](#4-compile-and-run-in-vs-code-with-mingw)
5. [Sample input and output](#5-sample-input-and-output)
6. [Time complexity](#6-time-complexity)
7. [Test cases (28)](#7-test-cases)
8. [Top 20 viva questions and answers](#8-top-20-viva-questions-and-answers)
9. [Known limitations and ideas for extension](#9-known-limitations-and-ideas-for-extension)

(The short README is in its own file, `README.md`.)

---

## 1. Project architecture

`main.cpp` is organised in five layers. Only layer 3 touches pointers, and nothing in layer 4 ever follows a `next` or `prev` pointer itself. That is the "separate the linked-list logic from the menu" rule from the assignment.

```
+----------------------------------------------------------------------+
| 5. main()          creates one MusicPlaylist, runs the menu loop     |
+----------------------------------------------------------------------+
| 4. Console UI      readLine / readInt / readText / readDuration      |
|                    menuAddSong, menuRemoveSong, ... runMenu          |
|                    (asks the user, validates input, calls layer 3)   |
+----------------------------------------------------------------------+
| 3. MusicPlaylist   ALL linked-list logic: add, remove, find, next,   |
|                    previous, select, delete all, display, verify     |
|                    owns:  head  |  current  |  count                 |
+----------------------------------------------------------------------+
| 2. SongNode        one song + next pointer + prev pointer            |
+----------------------------------------------------------------------+
| 1. Utility funcs   trim, toLowerCase, formatDuration                 |
+----------------------------------------------------------------------+
```

### The two classes

**`SongNode`** is one node of the list.

| Field | Type | Meaning |
|---|---|---|
| `id` | `int` | Unique Song ID |
| `title` | `string` | Song title |
| `artist` | `string` | Artist name |
| `durationSeconds` | `int` | Length in seconds (displayed as `mm:ss`) |
| `next` | `SongNode*` | Next song in the circle |
| `prev` | `SongNode*` | Previous song in the circle |

The pointers are `private`, and `MusicPlaylist` is declared a `friend`. So only the playlist can rewire the list. The rest of the program can read a song through the getters (`getId()`, `getTitle()`, ...) but cannot damage the links.

**`MusicPlaylist`** manages the list. It stores just three things:

| Member | Meaning |
|---|---|
| `head` | The first song (`nullptr` when the playlist is empty) |
| `current` | The currently selected song (`nullptr` when empty) |
| `count` | How many songs are in the list |

There is deliberately **no `tail` pointer**. In a doubly circular list the last node is always `head->prev`.

### Why a doubly circular list for a playlist?

| Need | How the structure answers it |
|---|---|
| "After the last song, go to the first" | `last->next` **is** the first node. No `if (at end)` code is needed. |
| "Before the first song, go to the last" | `first->prev` **is** the last node. |
| Next *and* previous in constant time | Each node has both `next` and `prev`. |
| Add at the end without walking the list | The tail is `head->prev`, so it is found in one step. |
| Remove a node you already hold | Its neighbours are reachable through `prev` and `next`, so no second pointer is needed while walking. |

---

## 2. How the doubly circular linked list works

### 2.1 The picture

```
        +------+  next   +------+  next   +------+
   +--> | S1   | ------> | S2   | ------> | S3   | ---+
   |    |      | <------ |      | <------ |      |    |
   |    +------+  prev   +------+  prev   +------+    |
   |       ^ head                                      |
   |       |                                           |
   |       +---- S1->prev points to the LAST node -----+  (circular)
   +-------------- S3->next points to the FIRST node
```

The two rules that make it "doubly circular":

* last node's `next` = first node
* first node's `prev` = last node

And the rule that must hold for **every** node (this is exactly what `verifyIntegrity()` checks):

```
node->next->prev == node        and        node->prev->next == node
```

### 2.2 Special shapes

| Situation | How it is represented |
|---|---|
| Empty playlist | `head == nullptr`, `current == nullptr`, `count == 0` |
| One song | The node points to **itself**: `node->next == node` and `node->prev == node` |
| Many songs | A closed ring in both directions |

### 2.3 Insertion at the end (`addSong`)

The playlist already holds `S1 <-> S2`, and we add `S3`. Let `tail = head->prev`. Here `head` is `S1` and `tail` is `S2`.

```
BEFORE:   S1.next = S2   S1.prev = S2
          S2.next = S1   S2.prev = S1

 1.  newNode->next = head;      // S3.next = S1   (new last node points forward to the first)
 2.  newNode->prev = tail;      // S3.prev = S2   (and backward to the old last node)
 3.  tail->next    = newNode;   // S2.next = S3   (old last node now points to the new node)
 4.  head->prev    = newNode;   // S1.prev = S3   (first node's "previous" is now the new last)

AFTER:    S1.next = S2   S1.prev = S3
          S2.next = S3   S2.prev = S1
          S3.next = S1   S3.prev = S2        <- the ring is closed again
```

`tail` is read into a variable **before** step 4, because step 4 changes `head->prev`.

Adding to an **empty** list is a separate case. The new node points to itself (`next = prev = newNode`), and `head` and `current` are set to it.

### 2.4 Deletion (`unlinkAndDelete`)

To delete `S2` from `S1 <-> S2 <-> S3`, make its two neighbours point to each other, then free it:

```
 target->prev->next = target->next;   // S1.next = S3
 target->next->prev = target->prev;   // S3.prev = S1
 delete target;                       // S2's memory is returned
```

Three extra things can go wrong, and the code handles each one:

| Case | Problem | Fix in the code |
|---|---|---|
| Target is the **first** song (`head`) | `head` would point to freed memory | `if (target == head) head = target->next;` |
| Target is the **current** song | `current` would point to freed memory | `if (target == current) current = target->next;` The next song becomes selected. |
| Target is the **only** song | `target->prev` and `target->next` are `target` itself, so the bypass would be meaningless | `if (count == 1)` set `head = current = nullptr`, then delete |

`count` is decreased after every delete.

### 2.5 Traversal without a `nullptr`

A circular list never ends in `nullptr`, so a normal `while (temp != nullptr)` loop would run forever. The code uses a **do-while** that stops when it arrives back at `head`:

```cpp
SongNode* temp = head;
do {
    // ... visit temp ...
    temp = temp->next;
} while (temp != head);      // back where we started = one full lap
```

A `do-while` is needed because the loop must run once before the test. With `while (temp != head)` the body would never execute, since `temp` starts equal to `head`.

### 2.6 Navigation and wrap-around

```cpp
wrapped = (current == head->prev);   // are we on the LAST song?
current = current->next;             // the ring itself does the wrap-around
```

Moving forward from the last song lands on the first because `last->next == head`. Moving backward from the first song lands on the last because `head->prev` is the last. The `wrapped` flag only exists so the menu can print the "Wrapping around" message.

---

## 3. Explanation of every major function

### 3.1 Utility functions

| Function | What it does |
|---|---|
| `trim(text)` | Removes spaces, tabs and `\r\n` from both ends. `"  Believer "` becomes `"Believer"`. It also removes the stray `\r` that Windows line endings can leave. |
| `toLowerCase(text)` | Returns a lower-case copy, used so title searches ignore case (`BELIEVER` finds `Believer`). |
| `formatDuration(seconds)` | Converts seconds to `mm:ss`, for example `225` becomes `03:45`. |

### 3.2 `SongNode`

| Member | What it does |
|---|---|
| Constructor | Stores id, title, artist and duration and sets both pointers to `nullptr`. |
| `getId()`, `getTitle()`, `getArtist()`, `getDurationSeconds()` | Read-only access for the rest of the program. |
| `friend class MusicPlaylist` | Lets only the playlist touch `next` and `prev`. |

### 3.3 `MusicPlaylist`: private helpers

| Function | What it does |
|---|---|
| `findNodeById(id)` | Walks the ring once with a do-while. Returns the node or `nullptr`. |
| `findNodeByTitle(title)` | Same, but compares lower-cased titles and returns the **first** match. |
| `unlinkAndDelete(target)` | The one place where a node is removed. It does the bypass, fixes `head` and `current`, calls `delete`, and decreases `count` (section 2.4). Every remove function goes through it, so the deletion logic exists once. |

### 3.4 `MusicPlaylist`: public functions

| Function | What it does |
|---|---|
| Constructor | Starts empty: `head = current = nullptr`, `count = 0`. |
| Destructor `~MusicPlaylist()` | Calls `deleteAllSongs()`, so memory is freed even if the program ends unexpectedly. |
| Copy constructor and assignment (`= delete`) | Copying a playlist would make two objects own the same raw pointers and delete them twice. Copying is forbidden at compile time. |
| `addSong(id, title, artist, seconds)` | Rejects invalid data and duplicate IDs (returns `false`). Otherwise allocates a node with `new` and links it at the end (section 2.3). The first song added also becomes the current song. |
| `removeSongById(id)` | Finds the node, then `unlinkAndDelete`. Returns `false` if the ID does not exist. |
| `removeSongByTitle(title)` | Same, using the first song with that title. (The menu does not call this directly: it first checks whether several songs share the title and then removes by ID, so the right song is always deleted. The function stays in the class as part of its interface and is covered by the unit tests.) |
| `deleteAllSongs()` | Breaks the ring with `head->prev->next = nullptr`, then deletes node after node while moving `head` forward. It then resets `head`, `current` and `count`. Safe to call on an empty list and safe to call twice. |
| `findById`, `findByTitle` | Public search. They return a `const SongNode*`, so callers can read the song but not change it. |
| `countByTitle(title)` | Counts songs with that title. Titles may repeat, while IDs may not. |
| `selectById`, `selectByTitle` | Move `current` to the found song. Return `false` and leave the selection unchanged if not found. |
| `nextSong(wrapped)` | Moves `current` forward by one and sets `wrapped = true` when it jumped from the last song to the first. Returns `nullptr` on an empty list. |
| `previousSong(wrapped)` | Moves `current` backward by one and sets `wrapped = true` when it jumped from the first song to the last. |
| `getCurrentSong()`, `getTotalSongs()`, `isEmpty()` | Simple getters, O(1). |
| `getPosition(node)` | Walks the ring to find "song number 1, 2, 3 ...". Returns 0 if the node is not found. |
| `getTotalDurationSeconds()` | Adds up every duration. |
| `displayPlaylist()` | Prints the table from first to last song, with `>>` marking the current song. |
| `displayMatchesByTitle(title)` | Prints every song with that title (used when a title is ambiguous). |
| `verifyIntegrity()` | Self-check used in testing. It checks `node->next->prev == node` and `node->prev->next == node` for every node, that `count` steps forward return to `head`, and that `current` is a real node. |

### 3.5 Console interface

| Function | What it does |
|---|---|
| `InputEnded` | A small exception thrown when input ends (Ctrl+Z on Windows, Ctrl+D on Linux/macOS). `main()` catches it and still frees the memory. |
| `readLine(prompt)` | Prints a prompt and reads a **whole line** with `getline`. |
| `isAllDigits`, `parseInteger` | Accept only digits, at most 9 of them (so `stoi` cannot overflow), inside the allowed range. |
| `tryReadInt` | Asks once. Returns `false` and prints an error on bad input. |
| `readInt` | Keeps asking until the number is valid. |
| `readText` | Keeps asking until the text is non-empty and not too long (title max 30, artist max 20). |
| `parseDuration`, `readDuration` | Accept `m:ss` or `mm:ss` (seconds 00 to 59, total above 0, minutes up to 3 digits) and return seconds. |
| `showSong(playlist, song)` | Prints `Song No. : 2 of 3`, ID, title, artist and duration. |
| `askSearchMode(action)` | The "by ID / by Title / Cancel" sub-menu shared by remove, search and select. |
| `printMenu()` | Prints the 12-option menu. |
| `menuAddSong` | Reads the ID first and rejects a duplicate immediately, then reads the other fields. |
| `removeAndReport`, `menuRemoveSong` | Remove by ID or title. Copy the title **before** deleting (the node is freed afterwards) and tell the user if the current song moved. |
| `menuDisplayPlaylist` | Calls `displayPlaylist()`. |
| `menuNextSong`, `menuPreviousSong` | Call `nextSong` / `previousSong` and print the wrap-around message when `wrapped` is true. |
| `menuSearchSong`, `menuSelectSong` | Search or select by ID or title, with "not found" and "several songs share this title" messages. |
| `menuCurrentSong`, `menuTotalSongs` | Show the current song, or the song count plus total playing time. |
| `menuRepeatPlaylist` | Plays N songs in a row from the current song, going around the ring as often as needed (see viva question 17). |
| `menuDeleteAll` | Asks `y/n` first, then calls `deleteAllSongs()`. |
| `runMenu` | The main loop: print menu, read choice (1 to 12), `switch` to the right function. |
| `main()` | Creates the playlist, runs the menu, then frees all memory and prints a goodbye message. |

> **Design note.** `displayPlaylist()` and `displayMatchesByTitle()` print from *inside* the class, because they are the "traversal" part of the assignment and keeping them next to the other traversals makes the loop easy to explain in a viva. Everything that *asks the user* or *decides what to print after an action* is outside the class.

---

## 4. Compile and run in VS Code with MinGW

> **What was and was not tested.** I compiled and tested the program on Linux with `g++` 13.3. The code uses only standard C++17 headers (`<iostream>`, `<string>`, `<iomanip>`, `<algorithm>`, `<cctype>`) and plain ASCII output, so it should build the same way with MinGW-w64 `g++`. I could not run it on Windows itself. If anything behaves differently on your machine, the troubleshooting list at the end of this section covers the usual causes.

### Step 1: Install MinGW-w64 (the `g++` compiler)

1. Install **MSYS2** from <https://www.msys2.org> (default options).
2. Open the **"MSYS2 UCRT64"** terminal from the Start menu and run:
   ```
   pacman -S --needed base-devel mingw-w64-ucrt-x86_64-toolchain
   ```
   Press Enter to accept all packages.
3. Add the compiler to Windows `PATH`: *Start → "Edit the system environment variables" → Environment Variables → Path → Edit → New* and add `C:\msys64\ucrt64\bin`. Click OK on every window.
4. Open a **new** Command Prompt or PowerShell and check:
   ```
   g++ --version
   ```
   You should see a version number. Use a recent one (GCC 11 or newer) so C++17 works fully. Very old MinGW installs (for example the one bundled with old Code::Blocks) can be too old.

### Step 2: Install VS Code and the C/C++ extension

1. Install VS Code from <https://code.visualstudio.com>.
2. Open the Extensions panel (`Ctrl+Shift+X`) and install **C/C++** (by Microsoft).

### Step 3: Open the project

1. Put `main.cpp` in a folder, for example `music-playlist-manager`.
2. In VS Code choose *File → Open Folder…* and select that folder.

### Step 4: Create the build task (once)

1. Create a folder `.vscode` inside the project, with a file `tasks.json` inside it:

```json
{
  "version": "2.0.0",
  "tasks": [
    {
      "type": "cppbuild",
      "label": "Build Music Playlist (g++ C++17)",
      "command": "g++",
      "args": ["-std=c++17", "-Wall", "-Wextra", "-g", "main.cpp", "-o", "playlist.exe"],
      "options": { "cwd": "${workspaceFolder}" },
      "group": { "kind": "build", "isDefault": true },
      "problemMatcher": ["$gcc"]
    }
  ]
}
```

2. Press **`Ctrl+Shift+B`** to build. A new file `playlist.exe` appears, and the terminal should show no errors or warnings.

### Step 5: Run it (in the *terminal*, not the Output panel)

Open the terminal (*Terminal → New Terminal*) and type:

```
.\playlist.exe          (PowerShell)
playlist                (Command Prompt)
```

The program needs keyboard input, so it **must** run in a real terminal. Extensions such as "Code Runner" that show output in the read-only *Output* panel will appear to hang.

### Alternative: one command, no tasks.json

```
g++ -std=c++17 -Wall -Wextra -g main.cpp -o playlist.exe
.\playlist.exe
```

### Optional: run the automated tests

```
g++ -std=c++17 -Wall -Wextra -g tests/unit_tests.cpp -o unit_tests.exe
.\unit_tests.exe
```

Expected last line: `==== 83 passed, 0 failed ====`

### Troubleshooting

| Problem | Fix |
|---|---|
| `'g++' is not recognized` | The `PATH` entry is missing or the terminal was opened before you added it. Re-check step 1.3 and open a **new** terminal. |
| The program hangs or does not accept input | You are running it in the Output panel. Run it in the terminal instead. |
| You want to leave without choosing Exit | Press `Ctrl+Z` then `Enter` (Windows). The program prints a message, frees the memory and exits cleanly. |
| Error about `-std=c++17` | The compiler is too old. Re-install through MSYS2 (step 1). |
| `Permission denied` when building | The old `playlist.exe` is still running. Close it first. |

---

## 5. Sample input and output

These are **real terminal sessions** captured from the compiled program. What you type is shown after each `Enter your choice:` or prompt. To keep the pages short, the 12-option menu is printed in full once per session and then replaced by a marker line (`[ ...the 12-option menu is shown again... ]`). The real program prints the full menu every time.

### 5.1 Session 1: adding songs, duplicate-ID protection, display, totals

```text
========== MUSIC PLAYLIST MANAGER ==========
 1. Add Song
 2. Remove Song
 3. Display Playlist
 4. Play Next Song
 5. Play Previous Song
 6. Search Song
 7. Select Song
 8. Display Current Song
 9. Display Total Songs
10. Repeat Playlist
11. Delete All Songs
12. Exit
============================================
Enter your choice: 1

--- Add Song ---
Enter Song ID (1-999999): 101
Enter Song Title: Shape of You
Enter Artist Name: Ed Sheeran
Enter Duration (mm:ss, e.g. 3:45): 3:53
Song "Shape of You" added successfully as song no. 1.

        [ ...the 12-option menu is shown again... ]
Enter your choice: 1

--- Add Song ---
Enter Song ID (1-999999): 102
Enter Song Title: Blinding Lights
Enter Artist Name: The Weeknd
Enter Duration (mm:ss, e.g. 3:45): 3:20
Song "Blinding Lights" added successfully as song no. 2.

        [ ...the 12-option menu is shown again... ]
Enter your choice: 1

--- Add Song ---
Enter Song ID (1-999999): 103
Enter Song Title: Believer
Enter Artist Name: Imagine Dragons
Enter Duration (mm:ss, e.g. 3:45): 3:24
Song "Believer" added successfully as song no. 3.

        [ ...the 12-option menu is shown again... ]
Enter your choice: 1

--- Add Song ---
Enter Song ID (1-999999): 102
Error: a song with ID 102 already exists. Song not added.

        [ ...the 12-option menu is shown again... ]
Enter your choice: 3

--- Playlist ---

-----------------------------------------------------------------------------
   No. ID      Title                           Artist                Duration
-----------------------------------------------------------------------------
>> 1   101     Shape of You                    Ed Sheeran            03:53
   2   102     Blinding Lights                 The Weeknd            03:20
   3   103     Believer                        Imagine Dragons       03:24
-----------------------------------------------------------------------------
Total songs: 3   ('>>' marks the currently selected song)

        [ ...the 12-option menu is shown again... ]
Enter your choice: 9

--- Total Songs ---
Total songs in playlist : 3
Total playing time      : 10:37 (mm:ss)

        [ ...the 12-option menu is shown again... ]
Enter your choice: 12

All playlist memory has been released. Goodbye!
```

### 5.2 Session 2: next, previous, wrap-around, search, select, current song

The three songs from session 1 are added first.

```text
========== MUSIC PLAYLIST MANAGER ==========
 1. Add Song
 2. Remove Song
 3. Display Playlist
 4. Play Next Song
 5. Play Previous Song
 6. Search Song
 7. Select Song
 8. Display Current Song
 9. Display Total Songs
10. Repeat Playlist
11. Delete All Songs
12. Exit
============================================

        [ songs 101, 102 and 103 are added exactly as in Session 1 ]

        [ ...the 12-option menu is shown again... ]
Enter your choice: 4

Now playing (next song):
  Song No. : 2 of 3
  Song ID  : 102
  Title    : Blinding Lights
  Artist   : The Weeknd
  Duration : 03:20

        [ ...the 12-option menu is shown again... ]
Enter your choice: 4

Now playing (next song):
  Song No. : 3 of 3
  Song ID  : 103
  Title    : Believer
  Artist   : Imagine Dragons
  Duration : 03:24

        [ ...the 12-option menu is shown again... ]
Enter your choice: 4

** End of playlist reached. Wrapping around to the FIRST song. **
Now playing (next song):
  Song No. : 1 of 3
  Song ID  : 101
  Title    : Shape of You
  Artist   : Ed Sheeran
  Duration : 03:53

        [ ...the 12-option menu is shown again... ]
Enter your choice: 5

** Start of playlist reached. Wrapping around to the LAST song. **
Now playing (previous song):
  Song No. : 3 of 3
  Song ID  : 103
  Title    : Believer
  Artist   : Imagine Dragons
  Duration : 03:24

        [ ...the 12-option menu is shown again... ]
Enter your choice: 6

--- Search Song ---
Search by:
  1. Song ID
  2. Song Title
  3. Cancel
Enter your choice: 2
Enter Song Title to search: blinding lights
Song found:
  Song No. : 2 of 3
  Song ID  : 102
  Title    : Blinding Lights
  Artist   : The Weeknd
  Duration : 03:20

        [ ...the 12-option menu is shown again... ]
Enter your choice: 6

--- Search Song ---
Search by:
  1. Song ID
  2. Song Title
  3. Cancel
Enter your choice: 1
Enter Song ID to search: 999
Song with ID 999 was not found.

        [ ...the 12-option menu is shown again... ]
Enter your choice: 7

--- Select Song ---
Select by:
  1. Song ID
  2. Song Title
  3. Cancel
Enter your choice: 1
Enter Song ID to select: 102
Song selected:
  Song No. : 2 of 3
  Song ID  : 102
  Title    : Blinding Lights
  Artist   : The Weeknd
  Duration : 03:20

        [ ...the 12-option menu is shown again... ]
Enter your choice: 8

--- Current Song ---
  Song No. : 2 of 3
  Song ID  : 102
  Title    : Blinding Lights
  Artist   : The Weeknd
  Duration : 03:20

        [ ...the 12-option menu is shown again... ]
Enter your choice: 12

All playlist memory has been released. Goodbye!
```

### 5.3 Session 3: repeat mode, removing the current song, removing by title and by ID, delete all

The same three songs are added first. The playlist is positioned on song 1 when repeat mode starts.

```text
========== MUSIC PLAYLIST MANAGER ==========
 1. Add Song
 2. Remove Song
 3. Display Playlist
 4. Play Next Song
 5. Play Previous Song
 6. Search Song
 7. Select Song
 8. Display Current Song
 9. Display Total Songs
10. Repeat Playlist
11. Delete All Songs
12. Exit
============================================

        [ songs 101, 102 and 103 are added exactly as in Session 1 ]

        [ ...the 12-option menu is shown again... ]
Enter your choice: 10

--- Repeat Playlist ---
How many songs to play in repeat mode (1-100)? 5

Repeat mode started: 5 song(s), beginning with the current song.

[1/5] Now playing:
  Song No. : 1 of 3
  Song ID  : 101
  Title    : Shape of You
  Artist   : Ed Sheeran
  Duration : 03:53

[2/5] Now playing:
  Song No. : 2 of 3
  Song ID  : 102
  Title    : Blinding Lights
  Artist   : The Weeknd
  Duration : 03:20

[3/5] Now playing:
  Song No. : 3 of 3
  Song ID  : 103
  Title    : Believer
  Artist   : Imagine Dragons
  Duration : 03:24

** End of playlist reached. Repeating from the FIRST song. **

[4/5] Now playing:
  Song No. : 1 of 3
  Song ID  : 101
  Title    : Shape of You
  Artist   : Ed Sheeran
  Duration : 03:53

[5/5] Now playing:
  Song No. : 2 of 3
  Song ID  : 102
  Title    : Blinding Lights
  Artist   : The Weeknd
  Duration : 03:20

Repeat mode finished. The last played song is now selected.

        [ ...the 12-option menu is shown again... ]
Enter your choice: 2

--- Remove Song ---
Remove by:
  1. Song ID
  2. Song Title
  3. Cancel
Enter your choice: 1
Enter Song ID to remove: 102
Song "Blinding Lights" (ID 102) removed successfully.
The removed song was the current song. The next song is now selected:
  Song No. : 2 of 2
  Song ID  : 103
  Title    : Believer
  Artist   : Imagine Dragons
  Duration : 03:24

        [ ...the 12-option menu is shown again... ]
Enter your choice: 2

--- Remove Song ---
Remove by:
  1. Song ID
  2. Song Title
  3. Cancel
Enter your choice: 2
Enter Song Title to remove: No Such Song
No song titled "No Such Song" was found.

        [ ...the 12-option menu is shown again... ]
Enter your choice: 2

--- Remove Song ---
Remove by:
  1. Song ID
  2. Song Title
  3. Cancel
Enter your choice: 1
Enter Song ID to remove: 101
Song "Shape of You" (ID 101) removed successfully.

        [ ...the 12-option menu is shown again... ]
Enter your choice: 3

--- Playlist ---

-----------------------------------------------------------------------------
   No. ID      Title                           Artist                Duration
-----------------------------------------------------------------------------
>> 1   103     Believer                        Imagine Dragons       03:24
-----------------------------------------------------------------------------
Total songs: 1   ('>>' marks the currently selected song)

        [ ...the 12-option menu is shown again... ]
Enter your choice: 11

--- Delete All Songs ---
Delete all 1 song(s)? (y/n): y
All songs deleted. The playlist has been reset.

        [ ...the 12-option menu is shown again... ]
Enter your choice: 3

--- Playlist ---
The playlist is empty.

        [ ...the 12-option menu is shown again... ]
Enter your choice: 12

All playlist memory has been released. Goodbye!
```

### 5.4 Session 4: invalid input (the program never crashes)

```text
========== MUSIC PLAYLIST MANAGER ==========
 1. Add Song
 2. Remove Song
 3. Display Playlist
 4. Play Next Song
 5. Play Previous Song
 6. Search Song
 7. Select Song
 8. Display Current Song
 9. Display Total Songs
10. Repeat Playlist
11. Delete All Songs
12. Exit
============================================
Enter your choice: 7

--- Select Song ---
The playlist is empty. Nothing to select.

        [ ...the 12-option menu is shown again... ]
Enter your choice: abc
  Invalid input! Please enter a whole number from 1 to 12.

        [ ...the 12-option menu is shown again... ]
Enter your choice: 13
  Invalid input! Please enter a whole number from 1 to 12.

        [ ...the 12-option menu is shown again... ]
Enter your choice: 1

--- Add Song ---
Enter Song ID (1-999999): 0
  Invalid input! Please enter a whole number from 1 to 999999.
Enter Song ID (1-999999): 5
Enter Song Title: 
  Title cannot be empty!
Enter Song Title: Long Title Long Title Long Title Long Title 
  Title is too long (maximum 30 characters)!
Enter Song Title: My Song
Enter Artist Name: Artist
Enter Duration (mm:ss, e.g. 3:45): 3:75
  Invalid duration! Use mm:ss with seconds from 00 to 59, and a length above 00:00.
Enter Duration (mm:ss, e.g. 3:45): abc
  Invalid duration! Use mm:ss with seconds from 00 to 59, and a length above 00:00.
Enter Duration (mm:ss, e.g. 3:45): 3:30
Song "My Song" added successfully as song no. 1.

        [ ...the 12-option menu is shown again... ]
Enter your choice: 3

--- Playlist ---

-----------------------------------------------------------------------------
   No. ID      Title                           Artist                Duration
-----------------------------------------------------------------------------
>> 1   5       My Song                         Artist                03:30
-----------------------------------------------------------------------------
Total songs: 1   ('>>' marks the currently selected song)

        [ ...the 12-option menu is shown again... ]
Enter your choice: 12

All playlist memory has been released. Goodbye!
```

---

## 6. Time complexity

`n` is the number of songs in the playlist.

| Operation | Linked-list work | Time | Notes |
|---|---|---|---|
| Insert at end (the 4 pointer changes) | `tail = head->prev`, then 4 assignments | **O(1)** | No traversal is needed because of the circular link. |
| `addSong` as a whole | Duplicate-ID check, then the O(1) insert | **O(n)** | The check walks the ring once, so the check dominates. |
| Delete a node you already hold a pointer to | Bypass with `prev` and `next` | **O(1)** | Doubly linked: no need to track a "previous" pointer while walking. |
| Remove by ID or title | Search, then the O(1) unlink | **O(n)** | Dominated by the search. Title search costs about O(n x L) for title length L because of lower-casing. |
| Search by ID | Walk the ring | **O(n)** | Best case O(1) (first song), worst case n. |
| Search by title | Walk the ring | **O(n)** | |
| Select by ID or title | Search, then one assignment | **O(n)** | |
| Traversal / display playlist | Visit every node once | **O(n)** | |
| Next song / previous song | `current = current->next` / `->prev` | **O(1)** | Wrap-around costs nothing extra. |
| Current song (the pointer) | Return `current` | **O(1)** | Printing "song no. X of N" adds an O(n) position lookup. |
| Total songs | Return `count` | **O(1)** | The counter is kept up to date. |
| Total duration | Visit every node | **O(n)** | |
| Delete all | Visit and free every node | **O(n)** | |
| Repeat mode for N songs | N times (O(1) move + O(n) position lookup for printing) | **O(N x n)** | The navigation itself is O(1) per step. |
| Space | One node per song, with 2 pointers each | **O(n)** | |

**Comparison with an array:** an array gives O(1) access by index but O(n) insertion or removal in the middle (elements must shift), and it needs wrap-around code such as `(i + 1) % size`. The circular list gives O(1) next/previous with built-in wrap-around and O(1) unlinking once the node is found. Its cost is O(n) search and two extra pointers per song.

---

## 7. Test cases

All 28 cases below were **run against the compiled program** (each one is a fixed list of keystrokes, with the output checked for the exact expected lines). All 28 pass. In the table, "3 songs" means IDs 101 *Shape of You*, 102 *Blinding Lights* and 103 *Believer* have been added, with song 101 selected.

### 7.1 Normal operations

| # | Test | Input / action | Expected result | Result |
|---|---|---|---|---|
| TC01 | Add the first song to an empty playlist | Add ID 101; then Display | `added successfully as song no. 1`; table shows `>> 1  101` and `Total songs: 1` | Pass |
| TC02 | Add three songs | Add 101, 102, 103; Display | Songs appear in the order added; `>>` is on song 1; `Total songs: 3` | Pass |
| TC04 | Next wraps from last to first | 3 songs; Next x3 | Third press prints `Wrapping around to the FIRST song` and shows `Song No. : 1 of 3` | Pass |
| TC05 | Previous wraps from first to last | 3 songs; Previous x1 | `Wrapping around to the LAST song`; shows `Song No. : 3 of 3` (Believer) | Pass |
| TC13 | Remove by title, ignoring case | 3 songs; Remove, by title, `believer` | `Song "Believer" (ID 103) removed successfully.`; `Total songs: 2` | Pass |
| TC15 | Search by ID (found and not found) | 3 songs; Search ID 102; Search ID 999 | `Song found:` with *Blinding Lights*; then `Song with ID 999 was not found.` | Pass |
| TC16 | Search by title, case and spaces ignored | 3 songs; Search by title `  SHAPE of you  ` | `Song found:` with `Song ID  : 101` | Pass |
| TC17 | Select a song, then show the current song | 3 songs; Select ID 103; Display Current | `Song selected:` then `Song No. : 3 of 3` (shown twice) | Pass |
| TC19 | Repeat mode goes around the ring more than once | 3 songs; Repeat, 7 songs | Plays `[1/7]` ... `[7/7]`; the message `Repeating from the FIRST song` appears twice | Pass |
| TC27 | Total songs and total time | 3 songs; Display Total Songs | `Total songs in playlist : 3` and `Total playing time : 10:37 (mm:ss)` | Pass |

### 7.2 Deletion, circular links and the current-song pointer

| # | Test | Input / action | Expected result | Result |
|---|---|---|---|---|
| TC09 | Remove the **first** song while another is selected | 3 songs; Select 103; Remove ID 101; Display | 101 removed; `>>` stays on 103 (now song 2 of 2) | Pass |
| TC10 | Remove the **last** song | 3 songs; Remove ID 103; Next, Next | The ring still closes: the second Next prints `Wrapping around to the FIRST song`, `Song No. : 1 of 2` | Pass |
| TC11 | Remove the **only** song, then add again | Add 101; Remove 101; Display; Add 102; Display | `The playlist is now empty.`, then `The playlist is empty.`, then 102 is added as `>> 1` | Pass |
| TC12 | Remove the **current** song | 3 songs; Next (current = 102); Remove ID 102 | `The removed song was the current song. The next song is now selected:` shows 103 as `Song No. : 2 of 2` | Pass |
| TC20 | Delete All: cancel, confirm, reuse | 3 songs; Delete All `n`; Total; Delete All `y`; Display; Total; Add 101; Display | `Cancelled. No songs were deleted.`; then `All songs deleted. The playlist has been reset.`; `The playlist is empty.`; total 0; new song becomes `>> 1` | Pass |

### 7.3 Edge cases

| # | Test | Input / action | Expected result | Result |
|---|---|---|---|---|
| TC06 | Display an empty playlist | Display | `The playlist is empty.` | Pass |
| TC07 | Every operation on an empty playlist | Next, Previous, Current, Search, Select, Remove, Repeat, Delete All, Total | A clear message for each (for example `The playlist is empty. Please add songs first.`); total shows 0; no crash | Pass |
| TC08 | One-song playlist | Add one song; Next; Previous | Both print `This is the only song in the playlist, so it plays again.` and stay on `Song No. : 1 of 1` | Pass |
| TC14 | Remove a song that does not exist | 3 songs; Remove ID 999; Remove title `No Such Song`; Total | `Song with ID 999 was not found.`; `No song titled "No Such Song" was found.`; total still 3 | Pass |
| TC18 | Select a song that does not exist | 3 songs; Select ID 999; Current | `Song with ID 999 was not found. Selection unchanged.`; current is still 101 | Pass |
| TC25 | Two songs share one title | 3 songs plus ID 104 titled `believer`; Search/Select/Remove by title `believer`; then Remove ID 104 | Search lists both; Select and Remove refuse and ask for the **ID**; removing ID 104 removes the right song; total 3 | Pass |
| TC26 | Input ends without choosing Exit (Ctrl+Z / Ctrl+D) | Add one song, then end the input | `Input stream closed. Exiting...` then `All playlist memory has been released. Goodbye!` | Pass |
| TC28 | Cancel and invalid choice inside a sub-menu | 3 songs; Remove, option 3 (Cancel); Remove, option 9; Search, Cancel; Total | `Invalid input! Please enter a whole number from 1 to 3.` once; nothing deleted; total 3 | Pass |

### 7.4 Invalid input

| # | Test | Input / action | Expected result | Result |
|---|---|---|---|---|
| TC03 | Duplicate Song ID | 3 songs; Add, ID 101 | `Error: a song with ID 101 already exists. Song not added.`; total still 3 | Pass |
| TC21 | Bad menu choices | `abc`, `0`, `13`, `-1`, empty line, `3.5`, `99999999999999999999` | Each prints `Invalid input! Please enter a whole number from 1 to 12.` (7 times) and the menu reappears | Pass |
| TC22 | Bad Song IDs | Add, then `abc`, `0`, `1000000`, `-5`, then `5` | 4 x `Invalid input! Please enter a whole number from 1 to 999999.`; the valid ID is accepted | Pass |
| TC23 | Empty or too-long title/artist | Title: empty, then 31 letters, then a valid one; Artist: empty, then 21 letters, then a valid one | `Title cannot be empty!`; `Title is too long (maximum 30 characters)!`; `Artist name cannot be empty!`; `Artist name is too long (maximum 20 characters)!`; song then added | Pass |
| TC24 | Bad durations | `abc`, `0:00`, `3:60`, `3`, `-1:30`, `1000:00`, `3:4:5`, then `3:30` | 7 x `Invalid duration!`; `3:30` is accepted and shows as `03:30` | Pass |

(TC numbers are in the order the checks were written, so the groups above are not in strict numeric order.)

### 7.5 Extra verification beyond the 28 menu tests

| Check | Result |
|---|---|
| `tests/unit_tests.cpp`: 83 direct checks of the `MusicPlaylist` class (empty, single node, first/last/middle deletion, `current` after deletion, delete-all and reuse, search, select, parsing helpers) | 83 passed, 0 failed |
| Same file: **20,000 random** add / remove / next / previous / select / delete-all operations, with `verifyIntegrity()`, order, count and `current` checked after **every** step | Passed |
| Program rebuilt with AddressSanitizer + UndefinedBehaviorSanitizer, all 28 menu cases replayed | 28/28 passed, no errors |
| Valgrind on a long session (add 5, navigate, select, remove first / middle / last / current, repeat, delete all, add again, exit) and on an abrupt end of input | `66 allocs, 66 frees`; `All heap blocks were freed -- no leaks are possible`; `0 errors` |
| Compiler warnings with `-std=c++17 -Wall -Wextra -pedantic` | None |

---

## 8. Top 20 viva questions and answers

**1. What is a doubly circular linked list, and how does it differ from the other linked lists?**
It is a list in which every node has a `next` and a `prev` pointer, and the ends are joined: the last node's `next` is the first node, and the first node's `prev` is the last node. A *singly linear* list has only `next` and ends at `nullptr`. A *singly circular* list has the ring but only one direction. A *doubly linear* list has both pointers but the ends are `nullptr`. Ours has both properties, so there is no `nullptr` inside a non-empty list.

**2. Why did you choose it for a music playlist?**
A playlist needs "next", "previous", and wrap-around at both ends. In this structure `last->next` is the first song and `first->prev` is the last song, so wrap-around needs no special code. Both directions are O(1). An array would need modulo arithmetic and shifting on removal. A singly linked list could not go backward in O(1).

**3. What does each node contain?**
`id`, `title`, `artist`, `durationSeconds`, a `next` pointer and a `prev` pointer (class `SongNode`). The pointers are private and only `MusicPlaylist` (a friend class) can change them.

**4. How do you represent an empty list and a one-song list?**
Empty: `head == nullptr`, `current == nullptr`, `count == 0`. One song: the single node points to itself, so `node->next == node` and `node->prev == node`. The ring is still valid, and the code treats that case separately when deleting.

**5. Explain insertion at the end. Why is it O(1) and why do you not traverse to the tail?**
The tail is always `head->prev`, so it is found in one step. Four assignments: `newNode->next = head; newNode->prev = tail; tail->next = newNode; head->prev = newNode;`. I store `tail` in a variable first, because the last assignment changes `head->prev`. (The whole `addSong` is O(n) only because of the duplicate-ID check.)

**6. Why do you use `do-while` for traversal instead of `while`?**
There is no `nullptr` at the end, so the loop must stop when it comes back to `head`. A traversal starts with `temp = head`, so `while (temp != head)` would be false immediately and the body would never run. `do { ... temp = temp->next; } while (temp != head);` runs once first, then stops after exactly one lap.

**7. Walk me through deleting a node in the middle.**
For `S1 <-> S2 <-> S3`, deleting `S2`: `S2->prev->next = S2->next` (so `S1.next = S3`), then `S2->next->prev = S2->prev` (so `S3.prev = S1`), then `delete S2` and `count--`. The two neighbours now point to each other and the ring stays closed.

**8. What special cases does deletion have?**
Three. (a) Deleting the **first** song: `head` must move to `target->next`. (b) Deleting the **current** song: `current` must move to `target->next`, otherwise it would dangle. (c) Deleting the **only** song: `prev` and `next` are the node itself, so the bypass is meaningless; the code sets `head` and `current` to `nullptr` and then deletes the node.

**9. What is a dangling pointer, and where could it appear in your program?**
A pointer to memory that has already been freed. It could appear in `head` (if the first node is deleted), in `current` (if the selected song is deleted), or in the UI (if I print a song after deleting it). The code updates `head` and `current` before `delete`, and `removeAndReport` copies the title into a local `string` *before* calling remove, so nothing reads the freed node.

**10. How does `deleteAllSongs` free a circular list without looping forever or deleting a node twice?**
It first breaks the ring with `head->prev->next = nullptr`, which turns the list into an ordinary linear list. Then a normal loop runs: save the node, move `head = head->next`, `delete` the saved node, until `head` is `nullptr`. Finally it resets `current = nullptr` and `count = 0`. Moving `head` forward **before** deleting avoids reading from freed memory.

**11. How are next and previous implemented, and how do you detect wrap-around?**
`current = current->next` and `current = current->prev`. The ring does the wrap. For the message, I check before moving: going forward `wrapped = (current == head->prev)` (are we on the last song?), going backward `wrapped = (current == head)` (are we on the first?). With one song the program prints "plays again" instead.

**12. Give the time complexity of the main operations.**
Next/previous: O(1). Insert at end: O(1) pointer work (O(n) with the duplicate check). Delete a known node: O(1). Search by ID or title, remove by ID/title, select: O(n). Display/traversal: O(n). Total songs: O(1) because `count` is maintained. Delete all: O(n). Space: O(n). See the table in section 6.

**13. A doubly linked list has two pointers, so why is search still O(n)?**
Extra pointers give direction, not ordering. The songs are stored in the order they were added, not sorted by ID, so every song may have to be checked. Faster search would need a different organisation, such as a sorted list with binary search (which linked lists do not support efficiently) or an additional hash table or tree keyed by ID. The assignment asks for the plain list.

**14. How is memory managed?**
Every song is created with `new` in `addSong` and destroyed with `delete` in `unlinkAndDelete` or `deleteAllSongs`. The destructor calls `deleteAllSongs()` so nothing leaks even if the playlist object goes out of scope unexpectedly. Copying is disabled (`= delete`) because two copies would hold the same raw pointers and double-delete them. Valgrind confirms equal numbers of allocations and frees.

**15. Where did you use encapsulation?**
`SongNode`'s fields and pointers are private. `MusicPlaylist` is a friend, so it alone can change `next` and `prev`. `MusicPlaylist`'s `head`, `current` and `count` are private. Outside code gets only `const SongNode*` pointers and getters, so it can read a song but cannot unlink or modify a node.

**16. Why must IDs be unique, and how do you handle duplicate titles?**
The ID identifies a song, so `addSong` rejects an ID that already exists (and the menu checks this right after the ID is typed). Titles can legitimately repeat (a cover version, for example). If a title search finds several songs, the program lists them all and asks the user to use the **ID** for select or remove, so it never deletes the wrong song.

**17. How does Repeat Playlist work? Is it really "continuous"?**
It starts at the current song and calls `nextSong` repeatedly, so after the last song it wraps back to the first. Because a console program that never stops would hang the terminal, the user chooses how many songs (1 to 100) to play. Conceptually the ring has no end, so a real player would just keep calling `nextSong` until a stop key is pressed. Next/Previous already give "continuous circular navigation" at any time, because the playlist never runs out.

**18. How do you stop bad input from crashing the program?**
All input is read with `getline` as text, never with `cin >> number` (which would put `cin` into a failed state on a letter). Numbers are accepted only if they are all digits, at most 9 digits (no `stoi` overflow) and in range. Durations are parsed as `m:ss` with seconds 0 to 59 and a total above zero. Empty and too-long text is rejected. If the input stream ends (Ctrl+Z / Ctrl+D), an `InputEnded` exception lets `main()` free the memory and exit cleanly.

**19. How did you verify that your pointer updates are correct?**
Three ways. (1) `verifyIntegrity()` checks for every node that `node->next->prev == node` and `node->prev->next == node`, that `count` steps forward return to `head`, and that `current` is a real node. (2) A test runs 20,000 random add / remove / navigate operations and calls that check after each one. (3) 28 menu test cases, plus Valgrind and AddressSanitizer for memory errors and leaks.

**20. What are the limitations, and what real-world uses do circular lists have?**
Limitations: random access and search are O(n); each node carries two extra pointers; the playlist is not saved to a file; songs are kept in insertion order only. Possible extensions: save/load to a file, shuffle, sort by title, remove duplicates, and a real audio player. Circular lists are used wherever "go round and round" is natural: CPU **round-robin scheduling**, turn-taking in multiplayer games, the **Josephus problem**, image carousels, and buffers that wrap.

---

## 9. Known limitations and ideas for extension

* Songs are stored only in insertion order, and the playlist is not saved to disk, so it is empty each time you start the program.
* Title and artist limits (30 and 20) count bytes. Non-English (UTF-8) characters take several bytes each, so the display columns may not line up perfectly for such titles.
* Seconds in a duration can be one or two digits, so `3:5` is read as `3:05`.
* Repeat mode stops after the number of songs you choose, so the console never hangs.
* Ideas: save/load the playlist to a text file, sort by title or artist, shuffle, "move song up/down", and edit a song's details.
