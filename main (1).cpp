/*
 * ============================================================================
 *  MUSIC PLAYLIST MANAGER USING A DOUBLY CIRCULAR LINKED LIST   (C++17)
 * ============================================================================
 *
 *  The playlist is stored ONLY as a doubly circular linked list:
 *
 *        +------+  next   +------+  next   +------+
 *   +--> | S1   | ------> | S2   | ------> | S3   | ---+
 *   |    |      | <------ |      | <------ |      |    |
 *   |    +------+  prev   +------+  prev   +------+    |
 *   |       ^ head                                      |
 *   |       |                                           |
 *   |       +---- S1->prev points to the LAST node -----+  (circular)
 *   +-------------- S3->next points to the FIRST node
 *
 *  - No arrays, vectors or STL containers are used to hold the songs.
 *  - Every song node is created with 'new' and removed with 'delete'.
 *  - Songs are stored in the order they were added (new songs go to the end).
 *
 *  File layout:
 *    1. Small utility functions (trim, lower-case, duration formatting)
 *    2. SongNode      - one node of the list
 *    3. MusicPlaylist - ALL linked-list logic (insert, delete, search, ...)
 *    4. Console UI    - input helpers + menu functions (no pointer surgery here)
 *    5. main()
 * ============================================================================
 */

#include <algorithm>
#include <cctype>
#include <iomanip>
#include <iostream>
#include <string>

using namespace std;

// ============================================================================
// 1. UTILITY FUNCTIONS
// ============================================================================

// Removes spaces / tabs from both ends of a string.
string trim(const string& text) {
    size_t first = text.find_first_not_of(" \t\r\n");
    if (first == string::npos) return "";
    size_t last = text.find_last_not_of(" \t\r\n");
    return text.substr(first, last - first + 1);
}

// Returns a lower-case copy (used for case-insensitive title matching).
string toLowerCase(string text) {
    transform(text.begin(), text.end(), text.begin(),
              [](unsigned char c) { return static_cast<char>(tolower(c)); });
    return text;
}

// Converts a number of seconds into "mm:ss" (e.g. 225 -> "03:45").
string formatDuration(long long totalSeconds) {
    string minutes = to_string(totalSeconds / 60);
    string seconds = to_string(totalSeconds % 60);
    if (minutes.size() < 2) minutes = "0" + minutes;
    if (seconds.size() < 2) seconds = "0" + seconds;
    return minutes + ":" + seconds;
}

// ============================================================================
// 2. SongNode  -  one node of the doubly circular linked list
// ============================================================================
class MusicPlaylist;  // forward declaration so SongNode can name it as a friend

class SongNode {
private:
    int id;               // unique Song ID
    string title;         // Song title
    string artist;        // Artist name
    int durationSeconds;  // Duration stored in seconds (shown as mm:ss)
    SongNode* next;       // pointer to the next song in the circle
    SongNode* prev;       // pointer to the previous song in the circle

    // Only the playlist is allowed to touch the next/prev pointers.
    friend class MusicPlaylist;

public:
    SongNode(int songId, const string& songTitle, const string& songArtist,
             int songDurationSeconds)
        : id(songId),
          title(songTitle),
          artist(songArtist),
          durationSeconds(songDurationSeconds),
          next(nullptr),
          prev(nullptr) {}

    // Read-only access for the outside world.
    int getId() const { return id; }
    const string& getTitle() const { return title; }
    const string& getArtist() const { return artist; }
    int getDurationSeconds() const { return durationSeconds; }
};

// ============================================================================
// 3. MusicPlaylist  -  manages the doubly circular linked list
// ============================================================================
class MusicPlaylist {
private:
    SongNode* head;     // first song of the playlist (nullptr when empty)
    SongNode* current;  // currently selected song   (nullptr when empty)
    int count;          // number of songs in the list

    // ---- internal helpers (linked-list logic) -----------------------------

    // Searches the circle for a song ID. Returns nullptr if not found.
    SongNode* findNodeById(int id) const {
        if (head == nullptr) return nullptr;  // empty playlist
        SongNode* temp = head;
        do {  // do-while: a circular list has no nullptr to stop at,
              // so we stop when we come back to head.
            if (temp->id == id) return temp;
            temp = temp->next;
        } while (temp != head);
        return nullptr;
    }

    // Searches for the FIRST song with this title (case-insensitive).
    SongNode* findNodeByTitle(const string& title) const {
        if (head == nullptr) return nullptr;
        string wanted = toLowerCase(title);
        SongNode* temp = head;
        do {
            if (toLowerCase(temp->title) == wanted) return temp;
            temp = temp->next;
        } while (temp != head);
        return nullptr;
    }

    // Removes ONE node from the circle and frees its memory.
    // Keeps head, current and count correct.
    void unlinkAndDelete(SongNode* target) {
        if (count == 1) {
            // Only song: after deletion the playlist is completely empty.
            head = nullptr;
            current = nullptr;
        } else {
            // Bypass the target: its neighbours now point to each other.
            target->prev->next = target->next;
            target->next->prev = target->prev;

            // If we are deleting the first song, the next one becomes head.
            if (target == head) head = target->next;

            // If we are deleting the selected song, move selection forward
            // so 'current' never points to freed memory.
            if (target == current) current = target->next;
        }
        delete target;  // give the memory back
        count--;
    }

public:
    MusicPlaylist() : head(nullptr), current(nullptr), count(0) {}

    // Destructor: frees every node when the playlist object is destroyed.
    ~MusicPlaylist() { deleteAllSongs(); }

    // A playlist owns raw pointers, so copying it would make two objects
    // delete the same nodes. Copying is therefore forbidden.
    MusicPlaylist(const MusicPlaylist&) = delete;
    MusicPlaylist& operator=(const MusicPlaylist&) = delete;

    // ---- insertion --------------------------------------------------------

    // Adds a song at the END of the playlist. Returns false if the song could
    // not be added (duplicate ID or invalid data).
    bool addSong(int id, const string& title, const string& artist,
                 int durationSeconds) {
        if (id <= 0 || title.empty() || artist.empty() || durationSeconds <= 0)
            return false;
        if (findNodeById(id) != nullptr) return false;  // duplicate ID

        SongNode* newNode = new SongNode(id, title, artist, durationSeconds);

        if (head == nullptr) {
            // CASE 1: empty playlist. The new node is the first AND last
            // node, so it points to itself in both directions.
            newNode->next = newNode;
            newNode->prev = newNode;
            head = newNode;
            current = newNode;  // first song becomes the selected song
        } else {
            // CASE 2: non-empty playlist. In a circular list the tail is
            // simply head->prev, so we do not need to traverse the list.
            SongNode* tail = head->prev;

            newNode->next = head;  // new last node points forward to the first
            newNode->prev = tail;  // and backward to the old last node
            tail->next = newNode;  // old last node points forward to new node
            head->prev = newNode;  // first node points backward to new node
        }
        count++;
        return true;
    }

    // ---- deletion ---------------------------------------------------------

    bool removeSongById(int id) {
        SongNode* target = findNodeById(id);
        if (target == nullptr) return false;
        unlinkAndDelete(target);
        return true;
    }

    // Removes the first song with this title (case-insensitive).
    bool removeSongByTitle(const string& title) {
        SongNode* target = findNodeByTitle(title);
        if (target == nullptr) return false;
        unlinkAndDelete(target);
        return true;
    }

    // Deletes every node, then resets head, current and count.
    void deleteAllSongs() {
        if (head == nullptr) return;  // nothing to delete

        // Break the circle first (last node's next = nullptr) so that an
        // ordinary "walk until nullptr" loop can free every node.
        head->prev->next = nullptr;

        while (head != nullptr) {
            SongNode* toDelete = head;
            head = head->next;  // move forward BEFORE deleting
            delete toDelete;
        }
        head = nullptr;
        current = nullptr;
        count = 0;
    }

    // ---- searching --------------------------------------------------------

    const SongNode* findById(int id) const { return findNodeById(id); }
    const SongNode* findByTitle(const string& title) const {
        return findNodeByTitle(title);
    }

    // How many songs have this title? (titles may repeat, IDs may not)
    int countByTitle(const string& title) const {
        if (head == nullptr) return 0;
        string wanted = toLowerCase(title);
        int matches = 0;
        const SongNode* temp = head;
        do {
            if (toLowerCase(temp->title) == wanted) matches++;
            temp = temp->next;
        } while (temp != head);
        return matches;
    }

    // ---- selection and navigation -----------------------------------------

    bool selectById(int id) {
        SongNode* found = findNodeById(id);
        if (found == nullptr) return false;
        current = found;
        return true;
    }

    bool selectByTitle(const string& title) {
        SongNode* found = findNodeByTitle(title);
        if (found == nullptr) return false;
        current = found;
        return true;
    }

    // Moves 'current' one step forward. 'wrapped' becomes true when we move
    // from the LAST song back to the FIRST song. Returns nullptr if empty.
    const SongNode* nextSong(bool& wrapped) {
        wrapped = false;
        if (current == nullptr) return nullptr;
        wrapped = (current == head->prev);  // head->prev is the last song
        current = current->next;            // the circle makes wrap-around free
        return current;
    }

    // Moves 'current' one step backward. 'wrapped' becomes true when we move
    // from the FIRST song back to the LAST song. Returns nullptr if empty.
    const SongNode* previousSong(bool& wrapped) {
        wrapped = false;
        if (current == nullptr) return nullptr;
        wrapped = (current == head);
        current = current->prev;
        return current;
    }

    // ---- information ------------------------------------------------------

    const SongNode* getCurrentSong() const { return current; }
    int getTotalSongs() const { return count; }
    bool isEmpty() const { return count == 0; }

    // Position (1, 2, 3 ...) of a node in the playlist; 0 if not found.
    int getPosition(const SongNode* node) const {
        if (node == nullptr || head == nullptr) return 0;
        int position = 1;
        const SongNode* temp = head;
        do {
            if (temp == node) return position;
            temp = temp->next;
            position++;
        } while (temp != head);
        return 0;
    }

    // Sum of all song durations in seconds.
    long long getTotalDurationSeconds() const {
        if (head == nullptr) return 0;
        long long total = 0;
        const SongNode* temp = head;
        do {
            total += temp->durationSeconds;
            temp = temp->next;
        } while (temp != head);
        return total;
    }

    // ---- display ----------------------------------------------------------

    // Prints the whole playlist from first to last; '>>' marks current song.
    void displayPlaylist() const {
        if (head == nullptr) {
            cout << "The playlist is empty.\n";
            return;
        }
        const string line(77, '-');
        cout << "\n" << line << "\n";
        cout << left << "   " << setw(4) << "No." << setw(8) << "ID"
             << setw(32) << "Title" << setw(22) << "Artist" << "Duration\n";
        cout << line << "\n";

        const SongNode* temp = head;
        int position = 1;
        do {
            cout << (temp == current ? ">> " : "   ") << left << setw(4)
                 << position << setw(8) << temp->id << setw(32) << temp->title
                 << setw(22) << temp->artist
                 << formatDuration(temp->durationSeconds) << "\n";
            temp = temp->next;
            position++;
        } while (temp != head);

        cout << line << "\n";
        cout << "Total songs: " << count
             << "   ('>>' marks the currently selected song)\n";
    }

    // Prints a short table of all songs with the given title.
    void displayMatchesByTitle(const string& title) const {
        if (head == nullptr) return;
        string wanted = toLowerCase(title);
        const SongNode* temp = head;
        do {
            if (toLowerCase(temp->title) == wanted) {
                cout << "   No. " << getPosition(temp) << " | ID " << temp->id
                     << " | " << temp->title << " | " << temp->artist << " | "
                     << formatDuration(temp->durationSeconds) << "\n";
            }
            temp = temp->next;
        } while (temp != head);
    }

    // ---- debugging helper (used for testing / viva demonstration) ---------

    // Checks that the list is a perfect doubly circular list:
    //  - for every node:  node->next->prev == node  and  node->prev->next == node
    //  - walking 'count' steps forward returns exactly to head
    //  - 'current' is one of the nodes
    bool verifyIntegrity() const {
        if (head == nullptr) return count == 0 && current == nullptr;
        if (count <= 0 || current == nullptr) return false;

        const SongNode* temp = head;
        bool currentFound = false;
        for (int i = 0; i < count; i++) {
            if (temp->next == nullptr || temp->prev == nullptr) return false;
            if (temp->next->prev != temp) return false;
            if (temp->prev->next != temp) return false;
            if (temp == current) currentFound = true;
            temp = temp->next;
        }
        return temp == head && currentFound;
    }
};

// ============================================================================
// 4. CONSOLE USER INTERFACE  (input helpers + menu functions)
// ============================================================================

// Thrown when the input stream ends (Ctrl+Z / Ctrl+D). main() catches it so
// the program can still exit cleanly and free the playlist.
struct InputEnded {};

const int MAX_TITLE_LENGTH = 30;
const int MAX_ARTIST_LENGTH = 20;
const int MAX_SONG_ID = 999999;

// Prints a prompt and reads one full line (trimmed).
string readLine(const string& prompt) {
    cout << prompt;
    string line;
    if (!getline(cin, line)) throw InputEnded();
    return trim(line);
}

bool isAllDigits(const string& text) {
    if (text.empty()) return false;
    for (unsigned char c : text) {
        if (!isdigit(c)) return false;
    }
    return true;
}

// Converts text to an int in [minValue, maxValue]. Returns false if invalid.
bool parseInteger(const string& text, int minValue, int maxValue, int& result) {
    if (!isAllDigits(text) || text.size() > 9) return false;  // 9 digits fit in int
    int value = stoi(text);
    if (value < minValue || value > maxValue) return false;
    result = value;
    return true;
}

// Asks ONCE. On bad input prints an error and returns false.
bool tryReadInt(const string& prompt, int minValue, int maxValue, int& result) {
    string line = readLine(prompt);
    if (parseInteger(line, minValue, maxValue, result)) return true;
    cout << "  Invalid input! Please enter a whole number from " << minValue
         << " to " << maxValue << ".\n";
    return false;
}

// Keeps asking until a valid number is entered.
int readInt(const string& prompt, int minValue, int maxValue) {
    int value = 0;
    while (!tryReadInt(prompt, minValue, maxValue, value)) {
    }
    return value;
}

// Keeps asking until a non-empty text that is not too long is entered.
string readText(const string& prompt, const string& fieldName, size_t maxLength) {
    while (true) {
        string text = readLine(prompt);
        if (text.empty()) {
            cout << "  " << fieldName << " cannot be empty!\n";
        } else if (text.size() > maxLength) {
            cout << "  " << fieldName << " is too long (maximum " << maxLength
                 << " characters)!\n";
        } else {
            return text;
        }
    }
}

// Converts "mm:ss" text into seconds. Returns false for invalid durations.
bool parseDuration(const string& text, int& seconds) {
    size_t colon = text.find(':');
    if (colon == string::npos) return false;
    string minutesText = trim(text.substr(0, colon));
    string secondsText = trim(text.substr(colon + 1));
    if (!isAllDigits(minutesText) || !isAllDigits(secondsText)) return false;
    if (minutesText.size() > 3 || secondsText.size() > 2) return false;

    int minutes = stoi(minutesText);
    int secs = stoi(secondsText);
    if (secs > 59) return false;
    int total = minutes * 60 + secs;
    if (total <= 0) return false;  // a song cannot be 00:00 long
    seconds = total;
    return true;
}

int readDuration() {
    while (true) {
        string text = readLine("Enter Duration (mm:ss, e.g. 3:45): ");
        int seconds = 0;
        if (parseDuration(text, seconds)) return seconds;
        cout << "  Invalid duration! Use mm:ss with seconds from 00 to 59, "
                "and a length above 00:00.\n";
    }
}

// Prints the details of one song together with its position in the list.
void showSong(const MusicPlaylist& playlist, const SongNode* song) {
    cout << "  Song No. : " << playlist.getPosition(song) << " of "
         << playlist.getTotalSongs() << "\n";
    cout << "  Song ID  : " << song->getId() << "\n";
    cout << "  Title    : " << song->getTitle() << "\n";
    cout << "  Artist   : " << song->getArtist() << "\n";
    cout << "  Duration : " << formatDuration(song->getDurationSeconds()) << "\n";
}

// Asks whether the user wants to work by ID or by title.
// Returns 1 (ID), 2 (title) or 0 (cancel / invalid).
int askSearchMode(const string& action) {
    cout << action << " by:\n";
    cout << "  1. Song ID\n";
    cout << "  2. Song Title\n";
    cout << "  3. Cancel\n";
    int choice = 0;
    if (!tryReadInt("Enter your choice: ", 1, 3, choice)) return 0;
    return (choice == 3) ? 0 : choice;
}

void printMenu() {
    cout << "\n========== MUSIC PLAYLIST MANAGER ==========\n";
    cout << " 1. Add Song\n";
    cout << " 2. Remove Song\n";
    cout << " 3. Display Playlist\n";
    cout << " 4. Play Next Song\n";
    cout << " 5. Play Previous Song\n";
    cout << " 6. Search Song\n";
    cout << " 7. Select Song\n";
    cout << " 8. Display Current Song\n";
    cout << " 9. Display Total Songs\n";
    cout << "10. Repeat Playlist\n";
    cout << "11. Delete All Songs\n";
    cout << "12. Exit\n";
    cout << "============================================\n";
}

// ---- menu actions ---------------------------------------------------------

void menuAddSong(MusicPlaylist& playlist) {
    cout << "--- Add Song ---\n";
    int id = readInt("Enter Song ID (1-999999): ", 1, MAX_SONG_ID);

    // Check for a duplicate right away, before asking for more details.
    if (playlist.findById(id) != nullptr) {
        cout << "Error: a song with ID " << id
             << " already exists. Song not added.\n";
        return;
    }

    string title = readText("Enter Song Title: ", "Title", MAX_TITLE_LENGTH);
    string artist = readText("Enter Artist Name: ", "Artist name", MAX_ARTIST_LENGTH);
    int duration = readDuration();

    if (playlist.addSong(id, title, artist, duration)) {
        cout << "Song \"" << title << "\" added successfully as song no. "
             << playlist.getTotalSongs() << ".\n";
    } else {
        cout << "Error: the song could not be added.\n";
    }
}

// Removes a song (found by ID) and tells the user what happened.
void removeAndReport(MusicPlaylist& playlist, int id) {
    const SongNode* song = playlist.findById(id);
    if (song == nullptr) {
        cout << "Song with ID " << id << " was not found.\n";
        return;
    }
    // Copy the details BEFORE deleting; the node will be freed.
    string title = song->getTitle();
    bool wasCurrent = (song == playlist.getCurrentSong());

    playlist.removeSongById(id);
    cout << "Song \"" << title << "\" (ID " << id << ") removed successfully.\n";

    if (playlist.isEmpty()) {
        cout << "The playlist is now empty.\n";
    } else if (wasCurrent) {
        cout << "The removed song was the current song. "
                "The next song is now selected:\n";
        showSong(playlist, playlist.getCurrentSong());
    }
}

void menuRemoveSong(MusicPlaylist& playlist) {
    cout << "--- Remove Song ---\n";
    if (playlist.isEmpty()) {
        cout << "The playlist is empty. Nothing to remove.\n";
        return;
    }
    int mode = askSearchMode("Remove");
    if (mode == 0) return;

    if (mode == 1) {
        int id = readInt("Enter Song ID to remove: ", 1, MAX_SONG_ID);
        removeAndReport(playlist, id);
    } else {
        string title = readText("Enter Song Title to remove: ", "Title", MAX_TITLE_LENGTH);
        int matches = playlist.countByTitle(title);
        if (matches == 0) {
            cout << "No song titled \"" << title << "\" was found.\n";
        } else if (matches > 1) {
            cout << matches << " songs share this title:\n";
            playlist.displayMatchesByTitle(title);
            cout << "Please remove by Song ID so the right song is deleted.\n";
        } else {
            removeAndReport(playlist, playlist.findByTitle(title)->getId());
        }
    }
}

void menuDisplayPlaylist(const MusicPlaylist& playlist) {
    cout << "--- Playlist ---\n";
    playlist.displayPlaylist();
}

void menuNextSong(MusicPlaylist& playlist) {
    if (playlist.isEmpty()) {
        cout << "The playlist is empty. Please add songs first.\n";
        return;
    }
    bool wrapped = false;
    const SongNode* song = playlist.nextSong(wrapped);

    if (playlist.getTotalSongs() == 1) {
        cout << "This is the only song in the playlist, so it plays again.\n";
    } else if (wrapped) {
        cout << "** End of playlist reached. Wrapping around to the FIRST song. **\n";
    }
    cout << "Now playing (next song):\n";
    showSong(playlist, song);
}

void menuPreviousSong(MusicPlaylist& playlist) {
    if (playlist.isEmpty()) {
        cout << "The playlist is empty. Please add songs first.\n";
        return;
    }
    bool wrapped = false;
    const SongNode* song = playlist.previousSong(wrapped);

    if (playlist.getTotalSongs() == 1) {
        cout << "This is the only song in the playlist, so it plays again.\n";
    } else if (wrapped) {
        cout << "** Start of playlist reached. Wrapping around to the LAST song. **\n";
    }
    cout << "Now playing (previous song):\n";
    showSong(playlist, song);
}

void menuSearchSong(const MusicPlaylist& playlist) {
    cout << "--- Search Song ---\n";
    if (playlist.isEmpty()) {
        cout << "The playlist is empty. Nothing to search.\n";
        return;
    }
    int mode = askSearchMode("Search");
    if (mode == 0) return;

    if (mode == 1) {
        int id = readInt("Enter Song ID to search: ", 1, MAX_SONG_ID);
        const SongNode* song = playlist.findById(id);
        if (song == nullptr) {
            cout << "Song with ID " << id << " was not found.\n";
        } else {
            cout << "Song found:\n";
            showSong(playlist, song);
        }
    } else {
        string title = readText("Enter Song Title to search: ", "Title", MAX_TITLE_LENGTH);
        int matches = playlist.countByTitle(title);
        if (matches == 0) {
            cout << "No song titled \"" << title << "\" was found.\n";
        } else if (matches == 1) {
            cout << "Song found:\n";
            showSong(playlist, playlist.findByTitle(title));
        } else {
            cout << matches << " songs found with this title:\n";
            playlist.displayMatchesByTitle(title);
        }
    }
}

void menuSelectSong(MusicPlaylist& playlist) {
    cout << "--- Select Song ---\n";
    if (playlist.isEmpty()) {
        cout << "The playlist is empty. Nothing to select.\n";
        return;
    }
    int mode = askSearchMode("Select");
    if (mode == 0) return;

    if (mode == 1) {
        int id = readInt("Enter Song ID to select: ", 1, MAX_SONG_ID);
        if (playlist.selectById(id)) {
            cout << "Song selected:\n";
            showSong(playlist, playlist.getCurrentSong());
        } else {
            cout << "Song with ID " << id << " was not found. Selection unchanged.\n";
        }
    } else {
        string title = readText("Enter Song Title to select: ", "Title", MAX_TITLE_LENGTH);
        int matches = playlist.countByTitle(title);
        if (matches == 0) {
            cout << "No song titled \"" << title << "\" was found. Selection unchanged.\n";
        } else if (matches > 1) {
            cout << matches << " songs share this title:\n";
            playlist.displayMatchesByTitle(title);
            cout << "Please select by Song ID instead.\n";
        } else {
            playlist.selectByTitle(title);
            cout << "Song selected:\n";
            showSong(playlist, playlist.getCurrentSong());
        }
    }
}

void menuCurrentSong(const MusicPlaylist& playlist) {
    cout << "--- Current Song ---\n";
    if (playlist.isEmpty()) {
        cout << "No song is selected because the playlist is empty.\n";
        return;
    }
    showSong(playlist, playlist.getCurrentSong());
}

void menuTotalSongs(const MusicPlaylist& playlist) {
    cout << "--- Total Songs ---\n";
    cout << "Total songs in playlist : " << playlist.getTotalSongs() << "\n";
    cout << "Total playing time      : "
         << formatDuration(playlist.getTotalDurationSeconds()) << " (mm:ss)\n";
}

// Repeat mode: plays N songs in a row starting at the current song, going
// around the circle again and again. (A real player would loop forever; the
// console version stops after N songs so the program does not hang.)
void menuRepeatPlaylist(MusicPlaylist& playlist) {
    cout << "--- Repeat Playlist ---\n";
    if (playlist.isEmpty()) {
        cout << "The playlist is empty. Please add songs first.\n";
        return;
    }
    int songsToPlay = readInt("How many songs to play in repeat mode (1-100)? ", 1, 100);
    cout << "\nRepeat mode started: " << songsToPlay
         << " song(s), beginning with the current song.\n";

    for (int i = 1; i <= songsToPlay; i++) {
        cout << "\n[" << i << "/" << songsToPlay << "] Now playing:\n";
        showSong(playlist, playlist.getCurrentSong());

        if (i < songsToPlay) {
            bool wrapped = false;
            playlist.nextSong(wrapped);
            if (wrapped && playlist.getTotalSongs() > 1) {
                cout << "\n** End of playlist reached. Repeating from the FIRST song. **\n";
            }
        }
    }
    cout << "\nRepeat mode finished. The last played song is now selected.\n";
}

void menuDeleteAll(MusicPlaylist& playlist) {
    cout << "--- Delete All Songs ---\n";
    if (playlist.isEmpty()) {
        cout << "The playlist is already empty.\n";
        return;
    }
    string answer = toLowerCase(readLine("Delete all " +
                                         to_string(playlist.getTotalSongs()) +
                                         " song(s)? (y/n): "));
    if (answer == "y" || answer == "yes") {
        playlist.deleteAllSongs();
        cout << "All songs deleted. The playlist has been reset.\n";
    } else {
        cout << "Cancelled. No songs were deleted.\n";
    }
}

// Main menu loop. Returns when the user chooses "Exit".
void runMenu(MusicPlaylist& playlist) {
    bool running = true;
    while (running) {
        printMenu();
        int choice = 0;
        if (!tryReadInt("Enter your choice: ", 1, 12, choice)) continue;
        cout << "\n";

        switch (choice) {
            case 1:  menuAddSong(playlist);        break;
            case 2:  menuRemoveSong(playlist);     break;
            case 3:  menuDisplayPlaylist(playlist); break;
            case 4:  menuNextSong(playlist);       break;
            case 5:  menuPreviousSong(playlist);   break;
            case 6:  menuSearchSong(playlist);     break;
            case 7:  menuSelectSong(playlist);     break;
            case 8:  menuCurrentSong(playlist);    break;
            case 9:  menuTotalSongs(playlist);     break;
            case 10: menuRepeatPlaylist(playlist); break;
            case 11: menuDeleteAll(playlist);      break;
            case 12: running = false;              break;
        }
    }
}

// ============================================================================
// 5. main()
// ============================================================================
int main() {
    MusicPlaylist playlist;

    try {
        runMenu(playlist);
    } catch (const InputEnded&) {
        cout << "\nInput stream closed. Exiting...\n";
    }

    // Free every song node. (The destructor would also do this automatically,
    // but calling it here lets us tell the user that memory was released.)
    playlist.deleteAllSongs();
    cout << "All playlist memory has been released. Goodbye!\n";
    return 0;
}
