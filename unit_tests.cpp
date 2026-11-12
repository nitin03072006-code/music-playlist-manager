#define main app_main
#include "../main.cpp"
#undef main

#include <cstdlib>
#include <vector>

static int passed = 0, failed = 0;
#define CHECK(cond, name)                                              \
    do {                                                               \
        if (cond) { passed++; cout << "PASS  " << name << "\n"; }      \
        else      { failed++; cout << "FAIL  " << name << "  (line " << __LINE__ << ")\n"; } \
    } while (0)

static vector<int> idsInOrder(MusicPlaylist& p) {
    vector<int> ids;
    if (p.isEmpty()) return ids;
    const SongNode* cur = p.getCurrentSong();
    bool w;

    while (p.getPosition(p.getCurrentSong()) != 1) p.previousSong(w);
    for (int i = 0; i < p.getTotalSongs(); i++) {
        ids.push_back(p.getCurrentSong()->getId());
        p.nextSong(w);
    }

    p.selectById(cur->getId());
    return ids;
}

int main() {
    bool w = false;

    {
        MusicPlaylist p;
        CHECK(p.isEmpty() && p.getTotalSongs() == 0, "T1 empty: count 0");
        CHECK(p.getCurrentSong() == nullptr, "T1 empty: current null");
        CHECK(p.nextSong(w) == nullptr && p.previousSong(w) == nullptr, "T1 empty: next/prev return null");
        CHECK(p.findById(1) == nullptr && p.findByTitle("x") == nullptr, "T1 empty: search returns null");
        CHECK(!p.removeSongById(1) && !p.removeSongByTitle("x"), "T1 empty: remove returns false");
        CHECK(!p.selectById(1) && !p.selectByTitle("x"), "T1 empty: select returns false");
        p.deleteAllSongs();
        CHECK(p.verifyIntegrity(), "T1 empty: deleteAll on empty is safe");
        CHECK(p.getTotalDurationSeconds() == 0 && p.getPosition(nullptr) == 0, "T1 empty: totals zero");
    }

    {
        MusicPlaylist p;
        CHECK(p.addSong(10, "A", "ArtA", 100), "T2 add first song");
        CHECK(p.verifyIntegrity() && p.getTotalSongs() == 1, "T2 single node self-circular");
        CHECK(p.getCurrentSong()->getId() == 10, "T2 first song becomes current");
        bool wr = false;
        p.nextSong(wr);
        CHECK(p.getCurrentSong()->getId() == 10 && wr, "T2 single-song next stays on same song");
        p.previousSong(wr);
        CHECK(p.getCurrentSong()->getId() == 10 && wr, "T2 single-song prev stays on same song");
        CHECK(p.addSong(20, "B", "ArtB", 200) && p.addSong(30, "C", "ArtC", 300), "T2 add more songs");
        CHECK(p.verifyIntegrity() && p.getTotalSongs() == 3, "T2 3 songs, links valid");
        CHECK((idsInOrder(p) == vector<int>{10, 20, 30}), "T2 order is insertion order");
        CHECK(p.getCurrentSong()->getId() == 10, "T2 current unchanged by adds");
        CHECK(p.getTotalDurationSeconds() == 600, "T2 total duration");
    }

    {
        MusicPlaylist p;
        p.addSong(1, "A", "X", 60);
        CHECK(!p.addSong(1, "Other", "Y", 70), "T3 duplicate ID rejected");
        CHECK(p.getTotalSongs() == 1 && p.verifyIntegrity(), "T3 duplicate left list unchanged");
        CHECK(!p.addSong(0, "A", "X", 60) && !p.addSong(-5, "A", "X", 60), "T3 non-positive ID rejected");
        CHECK(!p.addSong(2, "", "X", 60) && !p.addSong(2, "A", "", 60), "T3 empty title/artist rejected");
        CHECK(!p.addSong(2, "A", "X", 0) && !p.addSong(2, "A", "X", -1), "T3 non-positive duration rejected");
        CHECK(p.getTotalSongs() == 1, "T3 count still 1");
    }

    {
        MusicPlaylist p;
        p.addSong(1, "A", "X", 60); p.addSong(2, "B", "X", 60); p.addSong(3, "C", "X", 60);
        p.nextSong(w);  CHECK(p.getCurrentSong()->getId() == 2 && !w, "T4 next 1->2 no wrap");
        p.nextSong(w);  CHECK(p.getCurrentSong()->getId() == 3 && !w, "T4 next 2->3 no wrap");
        p.nextSong(w);  CHECK(p.getCurrentSong()->getId() == 1 && w,  "T4 next 3->1 WRAPS");
        p.previousSong(w); CHECK(p.getCurrentSong()->getId() == 3 && w,  "T4 prev 1->3 WRAPS");
        p.previousSong(w); CHECK(p.getCurrentSong()->getId() == 2 && !w, "T4 prev 3->2 no wrap");

        int start = p.getCurrentSong()->getId();
        for (int i = 0; i < 3; i++) p.nextSong(w);
        CHECK(p.getCurrentSong()->getId() == start, "T4 N nexts return to the same song");
        for (int i = 0; i < 3; i++) p.previousSong(w);
        CHECK(p.getCurrentSong()->getId() == start, "T4 N prevs return to the same song");
        CHECK(p.verifyIntegrity(), "T4 links valid after navigation");
    }

    {
        MusicPlaylist p;
        p.addSong(1, "Shape of You", "Ed", 233); p.addSong(2, "Believer", "Imagine", 204);
        p.addSong(3, "believer", "Cover Band", 210);
        CHECK(p.findById(2) != nullptr && p.findById(2)->getTitle() == "Believer", "T5 find by id");
        CHECK(p.findById(99) == nullptr, "T5 find missing id -> null");
        CHECK(p.findByTitle("SHAPE OF YOU") != nullptr, "T5 title search is case-insensitive");
        CHECK(p.findByTitle("Nope") == nullptr, "T5 find missing title -> null");
        CHECK(p.countByTitle("believer") == 2 && p.countByTitle("shape of you") == 1 && p.countByTitle("zzz") == 0, "T5 countByTitle");
        CHECK(p.selectById(3) && p.getCurrentSong()->getId() == 3, "T5 select by id");
        CHECK(!p.selectById(99) && p.getCurrentSong()->getId() == 3, "T5 select missing id keeps selection");
        CHECK(p.selectByTitle("shape of you") && p.getCurrentSong()->getId() == 1, "T5 select by title");
        CHECK(!p.selectByTitle("none") && p.getCurrentSong()->getId() == 1, "T5 select missing title keeps selection");
        CHECK(p.getPosition(p.findById(3)) == 3, "T5 getPosition");
    }

    {
        MusicPlaylist p;
        for (int i = 1; i <= 5; i++) p.addSong(i, "S" + to_string(i), "X", 60);
        p.selectById(3);
        CHECK(p.removeSongById(1), "T6 delete first");
        CHECK(p.verifyIntegrity() && (idsInOrder(p) == vector<int>{2, 3, 4, 5}), "T6 delete first: links+order valid");
        CHECK(p.getPosition(p.findById(2)) == 1, "T6 new head is song 2");
        CHECK(p.removeSongById(5), "T6 delete last");
        CHECK(p.verifyIntegrity() && (idsInOrder(p) == vector<int>{2, 3, 4}), "T6 delete last: links+order valid");
        bool wr; p.selectById(4); p.nextSong(wr);
        CHECK(p.getCurrentSong()->getId() == 2 && wr, "T6 after deleting last, 4 -> next wraps to 2");
        CHECK(p.getCurrentSong()->getId() == 2, "T6 current valid");
        CHECK(p.removeSongById(3), "T6 delete middle");
        CHECK(p.verifyIntegrity() && (idsInOrder(p) == vector<int>{2, 4}), "T6 delete middle: links+order valid");
        CHECK(!p.removeSongById(3) && !p.removeSongById(777), "T6 delete nonexistent returns false");
        CHECK(p.getTotalSongs() == 2, "T6 count correct");
        CHECK(p.removeSongById(2), "T6 delete -> one song left");
        CHECK(p.verifyIntegrity() && p.getTotalSongs() == 1 && p.getCurrentSong()->getId() == 4, "T6 one left, current valid");
        bool wr2; p.nextSong(wr2);
        CHECK(p.getCurrentSong()->getId() == 4, "T6 single song next stays");
        CHECK(p.removeSongById(4), "T6 delete only song");
        CHECK(p.isEmpty() && p.getCurrentSong() == nullptr && p.verifyIntegrity(), "T6 only song deleted -> empty, current null");
        CHECK(p.addSong(8, "New", "X", 60) && p.getCurrentSong()->getId() == 8 && p.verifyIntegrity(), "T6 can add again after emptying");
    }

    {
        MusicPlaylist p;
        for (int i = 1; i <= 3; i++) p.addSong(i, "S" + to_string(i), "X", 60);
        p.selectById(2); p.removeSongById(2);
        CHECK(p.getCurrentSong() != nullptr && p.getCurrentSong()->getId() == 3, "T7 deleting current (middle) -> next selected");
        p.selectById(3); p.removeSongById(3);
        CHECK(p.getCurrentSong() != nullptr && p.getCurrentSong()->getId() == 1, "T7 deleting current (last) -> wraps to first");
        p.addSong(4, "S4", "X", 60);
        p.selectById(1); p.removeSongById(1);
        CHECK(p.getCurrentSong() != nullptr && p.getCurrentSong()->getId() == 4 && p.verifyIntegrity(), "T7 deleting current (first) -> next selected");
        p.addSong(5, "S5", "X", 60);
        p.selectById(5); p.removeSongByTitle("S4");
        CHECK(p.getCurrentSong()->getId() == 5 && p.getTotalSongs() == 1 && p.verifyIntegrity(), "T7 delete other song by title keeps current");
    }

    {
        MusicPlaylist p;
        for (int i = 1; i <= 6; i++) p.addSong(i, "S" + to_string(i), "X", 60);
        p.nextSong(w);
        p.deleteAllSongs();
        CHECK(p.isEmpty() && p.getTotalSongs() == 0, "T8 deleteAll: count 0");
        CHECK(p.getCurrentSong() == nullptr, "T8 deleteAll: current null");
        CHECK(p.findById(1) == nullptr && p.verifyIntegrity(), "T8 deleteAll: head reset, list valid");
        p.deleteAllSongs();
        CHECK(p.verifyIntegrity(), "T8 deleteAll twice is safe");
        CHECK(p.addSong(1, "A", "X", 60) && p.verifyIntegrity(), "T8 reuse after deleteAll");

        MusicPlaylist q; q.addSong(1, "A", "X", 60); q.deleteAllSongs();
        CHECK(q.isEmpty() && q.verifyIntegrity(), "T8 deleteAll with single node");
    }

    {
        int v = 0;
        CHECK(parseInteger("5", 1, 12, v) && v == 5, "T9 parseInteger valid");
        CHECK(!parseInteger("0", 1, 12, v) && !parseInteger("13", 1, 12, v), "T9 parseInteger out of range");
        CHECK(!parseInteger("abc", 1, 12, v) && !parseInteger("", 1, 12, v) && !parseInteger("-3", 1, 12, v) && !parseInteger("3.5", 1, 12, v) && !parseInteger("1 2", 1, 12, v), "T9 parseInteger rejects junk");
        CHECK(!parseInteger("99999999999999999999", 1, 999999, v), "T9 parseInteger rejects overflow-size numbers");
        int s = 0;
        CHECK(parseDuration("3:45", s) && s == 225, "T9 duration 3:45 = 225s");
        CHECK(parseDuration("03:05", s) && s == 185, "T9 duration 03:05");
        CHECK(parseDuration("0:01", s) && s == 1, "T9 duration 0:01");
        CHECK(parseDuration(" 4 : 30 ", s) && s == 270, "T9 duration tolerates spaces");
        CHECK(!parseDuration("0:00", s), "T9 duration 0:00 rejected");
        CHECK(!parseDuration("3:60", s) && !parseDuration("3:75", s), "T9 duration seconds>=60 rejected");
        CHECK(!parseDuration("abc", s) && !parseDuration("3", s) && !parseDuration(":30", s) && !parseDuration("3:", s) && !parseDuration("3:4:5", s) && !parseDuration("-1:30", s) && !parseDuration("3:-5", s), "T9 duration rejects junk");
        CHECK(!parseDuration("1000:00", s), "T9 duration minutes > 999 rejected");
        CHECK(formatDuration(225) == "03:45" && formatDuration(5) == "00:05" && formatDuration(3600) == "60:00", "T9 formatDuration");
    }

    {
        srand(12345);
        MusicPlaylist p;
        vector<int> model;
        bool ok = true;
        for (int step = 0; step < 20000 && ok; step++) {
            int op = rand() % 10;
            if (op < 4) {
                int id = 1 + rand() % 300;
                bool exists = false;
                for (int x : model) if (x == id) exists = true;
                bool added = p.addSong(id, "T" + to_string(id), "A", 60 + id);
                if (added == exists) ok = false;
                if (added) model.push_back(id);
            } else if (op < 7 && !model.empty()) {
                int idx = rand() % model.size();
                int id = model[idx];

                bool wasCurrent = p.getCurrentSong()->getId() == id;
                int expectedNext = model[(idx + 1) % model.size()];
                if (!p.removeSongById(id)) ok = false;
                model.erase(model.begin() + idx);
                if (!model.empty() && wasCurrent && p.getCurrentSong()->getId() != expectedNext) ok = false;
            } else if (op == 7) {
                p.nextSong(w);
            } else if (op == 8) {
                p.previousSong(w);
            } else if (op == 9 && !model.empty()) {
                p.selectById(model[rand() % model.size()]);
            }
            if (!p.verifyIntegrity()) ok = false;
            if (p.getTotalSongs() != (int)model.size()) ok = false;
            if (!model.empty() && idsInOrder(p) != model) ok = false;
            if (rand() % 997 == 0) { p.deleteAllSongs(); model.clear(); }
        }
        CHECK(ok, "T10 20000 random operations: links, order, count, current always valid");
    }

    cout << "\n==== " << passed << " passed, " << failed << " failed ====\n";
    return failed == 0 ? 0 : 1;
}
