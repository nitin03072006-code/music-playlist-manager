#include <algorithm>
#include <cctype>
#include <iomanip>
#include <iostream>
#include <string>

using namespace std;

string trim(const string& text) {
    size_t first = text.find_first_not_of(" \t\r\n");
    if (first == string::npos) return "";
    size_t last = text.find_last_not_of(" \t\r\n");
    return text.substr(first, last - first + 1);
}

string toLowerCase(string text) {
    transform(text.begin(), text.end(), text.begin(),
              [](unsigned char c) { return static_cast<char>(tolower(c)); });
    return text;
}

string formatDuration(long long totalSeconds) {
    string minutes = to_string(totalSeconds / 60);
    string seconds = to_string(totalSeconds % 60);
    if (minutes.size() < 2) minutes = "0" + minutes;
    if (seconds.size() < 2) seconds = "0" + seconds;
    return minutes + ":" + seconds;
}

class MusicPlaylist;

class SongNode {
private:
    int id;
    string title;
    string artist;
    int durationSeconds;
    SongNode* next;
    SongNode* prev;

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

    int getId() const { return id; }
    const string& getTitle() const { return title; }
    const string& getArtist() const { return artist; }
    int getDurationSeconds() const { return durationSeconds; }
};

class MusicPlaylist {
private:
    SongNode* head;
    SongNode* current;
    int count;

    SongNode* findNodeById(int id) const {
        if (head == nullptr) return nullptr;
        SongNode* temp = head;
        do {

            if (temp->id == id) return temp;
            temp = temp->next;
        } while (temp != head);
        return nullptr;
    }

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

    void unlinkAndDelete(SongNode* target) {
        if (count == 1) {

            head = nullptr;
            current = nullptr;
        } else {

            target->prev->next = target->next;
            target->next->prev = target->prev;

            if (target == head) head = target->next;

            if (target == current) current = target->next;
        }
        delete target;
        count--;
    }

public:
    MusicPlaylist() : head(nullptr), current(nullptr), count(0) {}

    ~MusicPlaylist() { deleteAllSongs(); }

    MusicPlaylist(const MusicPlaylist&) = delete;
    MusicPlaylist& operator=(const MusicPlaylist&) = delete;

    bool addSong(int id, const string& title, const string& artist,
                 int durationSeconds) {
        if (id <= 0 || title.empty() || artist.empty() || durationSeconds <= 0)
            return false;
        if (findNodeById(id) != nullptr) return false;

        SongNode* newNode = new SongNode(id, title, artist, durationSeconds);

        if (head == nullptr) {

            newNode->next = newNode;
            newNode->prev = newNode;
            head = newNode;
            current = newNode;
        } else {

            SongNode* tail = head->prev;

            newNode->next = head;
            newNode->prev = tail;
            tail->next = newNode;
            head->prev = newNode;
        }
        count++;
        return true;
    }

    bool removeSongById(int id) {
        SongNode* target = findNodeById(id);
        if (target == nullptr) return false;
        unlinkAndDelete(target);
        return true;
    }

    bool removeSongByTitle(const string& title) {
        SongNode* target = findNodeByTitle(title);
        if (target == nullptr) return false;
        unlinkAndDelete(target);
        return true;
    }

    void deleteAllSongs() {
        if (head == nullptr) return;

        head->prev->next = nullptr;

        while (head != nullptr) {
            SongNode* toDelete = head;
            head = head->next;
            delete toDelete;
        }
        head = nullptr;
        current = nullptr;
        count = 0;
    }

    const SongNode* findById(int id) const { return findNodeById(id); }
    const SongNode* findByTitle(const string& title) const {
        return findNodeByTitle(title);
    }

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

    const SongNode* nextSong(bool& wrapped) {
        wrapped = false;
        if (current == nullptr) return nullptr;
        wrapped = (current == head->prev);
        current = current->next;
        return current;
    }

    const SongNode* previousSong(bool& wrapped) {
        wrapped = false;
        if (current == nullptr) return nullptr;
        wrapped = (current == head);
        current = current->prev;
        return current;
    }

    const SongNode* getCurrentSong() const { return current; }
    int getTotalSongs() const { return count; }
    bool isEmpty() const { return count == 0; }

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

struct InputEnded {};

const int MAX_TITLE_LENGTH = 30;
const int MAX_ARTIST_LENGTH = 20;
const int MAX_SONG_ID = 999999;

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

bool parseInteger(const string& text, int minValue, int maxValue, int& result) {
    if (!isAllDigits(text) || text.size() > 9) return false;
    int value = stoi(text);
    if (value < minValue || value > maxValue) return false;
    result = value;
    return true;
}

bool tryReadInt(const string& prompt, int minValue, int maxValue, int& result) {
    string line = readLine(prompt);
    if (parseInteger(line, minValue, maxValue, result)) return true;
    cout << "  Invalid input! Please enter a whole number from " << minValue
         << " to " << maxValue << ".\n";
    return false;
}

int readInt(const string& prompt, int minValue, int maxValue) {
    int value = 0;
    while (!tryReadInt(prompt, minValue, maxValue, value)) {
    }
    return value;
}

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
    if (total <= 0) return false;
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

void showSong(const MusicPlaylist& playlist, const SongNode* song) {
    cout << "  Song No. : " << playlist.getPosition(song) << " of "
         << playlist.getTotalSongs() << "\n";
    cout << "  Song ID  : " << song->getId() << "\n";
    cout << "  Title    : " << song->getTitle() << "\n";
    cout << "  Artist   : " << song->getArtist() << "\n";
    cout << "  Duration : " << formatDuration(song->getDurationSeconds()) << "\n";
}

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

void menuAddSong(MusicPlaylist& playlist) {
    cout << "--- Add Song ---\n";
    int id = readInt("Enter Song ID (1-999999): ", 1, MAX_SONG_ID);

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

void removeAndReport(MusicPlaylist& playlist, int id) {
    const SongNode* song = playlist.findById(id);
    if (song == nullptr) {
        cout << "Song with ID " << id << " was not found.\n";
        return;
    }

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

int main() {
    MusicPlaylist playlist;

    try {
        runMenu(playlist);
    } catch (const InputEnded&) {
        cout << "\nInput stream closed. Exiting...\n";
    }

    playlist.deleteAllSongs();
    cout << "All playlist memory has been released. Goodbye!\n";
    return 0;
}
