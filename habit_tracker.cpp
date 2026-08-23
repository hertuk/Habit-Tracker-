// habit_tracker.cpp
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <ctime>
#include <iomanip>
#include <random>
#include <algorithm>
#include <nlohmann/json.hpp>

using namespace std;
using json = nlohmann::json;

string generateId() {
    const char* hex = "0123456789abcdef";
    string id;
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(0, 15);
    for (int i=0; i<8; i++) id += hex[dis(gen)];
    return id;
}

string currentDate() {
    time_t t = time(nullptr);
    char buf[11];
    strftime(buf, sizeof(buf), "%Y-%m-%d", localtime(&t));
    return string(buf);
}

string currentTime() {
    time_t t = time(nullptr);
    char buf[30];
    strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S%z", localtime(&t));
    return string(buf);
}

struct Habit {
    string id;
    string name;
    string description;
    int streak;
    int longestStreak;
    string lastLogDate;
    vector<string> history;
    string createdAt;

    Habit() : streak(0), longestStreak(0) {}
    Habit(const string& n, const string& desc = "") : id(generateId()), name(n), description(desc),
        streak(0), longestStreak(0), createdAt(currentTime()) {}

    bool log(const string& logDate) {
        string d = logDate.empty() ? currentDate() : logDate;
        if (find(history.begin(), history.end(), d) != history.end()) {
            cout << "⚠️ Already logged for " << d << "\n";
            return false;
        }
        if (!lastLogDate.empty()) {
            struct tm lastTm = {}, curTm = {};
            strptime(lastLogDate.c_str(), "%Y-%m-%d", &lastTm);
            strptime(d.c_str(), "%Y-%m-%d", &curTm);
            time_t last = mktime(&lastTm);
            time_t cur = mktime(&curTm);
            int days = (int)difftime(cur, last) / (24*3600);
            if (days == 1) streak++;
            else if (days > 1) streak = 1;
        } else {
            streak = 1;
        }
        if (streak > longestStreak) longestStreak = streak;
        lastLogDate = d;
        history.push_back(d);
        cout << "✅ Logged '" << name << "' for " << d << " (streak: " << streak << " days)\n";
        return true;
    }

    void reset() {
        streak = 0;
        lastLogDate = "";
        cout << "🔄 Reset streak for '" << name << "'\n";
    }
};

class Tracker {
private:
    vector<Habit> habits;
    string dataFile = "habits.json";

    void load() {
        ifstream f(dataFile);
        if (!f.is_open()) return;
        json j;
        f >> j;
        for (auto& item : j) {
            Habit h;
            h.id = item["id"];
            h.name = item["name"];
            h.description = item["description"];
            h.streak = item["streak"];
            h.longestStreak = item["longest_streak"];
            h.lastLogDate = item["last_log_date"];
            h.history = item["history"].get<vector<string>>();
            h.createdAt = item["created_at"];
            habits.push_back(h);
        }
    }

    void save() {
        json j = json::array();
        for (auto& h : habits) {
            j.push_back({
                {"id", h.id},
                {"name", h.name},
                {"description", h.description},
                {"streak", h.streak},
                {"longest_streak", h.longestStreak},
                {"last_log_date", h.lastLogDate},
                {"history", h.history},
                {"created_at", h.createdAt}
            });
        }
        ofstream f(dataFile);
        f << setw(2) << j << endl;
    }

    Habit* getHabit(const string& id) {
        for (auto& h : habits) {
            if (h.id == id) return &h;
        }
        return nullptr;
    }

public:
    Tracker() { load(); }

    void add(const string& name, const string& description) {
        Habit h(name, description);
        habits.push_back(h);
        save();
        cout << "✅ Habit added: " << h.name << " (ID: " << h.id << ")\n";
    }

    void log(const string& id, const string& date) {
        Habit* h = getHabit(id);
        if (!h) {
            cout << "❌ Habit " << id << " not found.\n";
            return;
        }
        h->log(date);
        save();
    }

    void list() {
        if (habits.empty()) {
            cout << "No habits yet. Add one with 'add'\n";
            return;
        }
        cout << "\n🔥 Habit Tracker\n";
        cout << "📋 Your Habits:\n";
        for (size_t i=0; i<habits.size(); i++) {
            auto& h = habits[i];
            string status = h.streak > 0 ? "✅" : "⏳";
            string days = h.streak > 0 ? "🔥" : "💤";
            cout << "  " << i+1 << ". " << status << " " << h.name
                 << " (" << days << " " << h.streak << " days) – Best: " << h.longestStreak << " days\n";
        }
    }

    void stats() {
        if (habits.empty()) {
            cout << "No habits yet.\n";
            return;
        }
        int total = habits.size();
        int totalLogs = 0;
        int sumStreak = 0;
        int longest = 0;
        string bestName;
        for (auto& h : habits) {
            totalLogs += h.history.size();
            sumStreak += h.streak;
            if (h.longestStreak > longest) {
                longest = h.longestStreak;
                bestName = h.name;
            }
        }
        double avg = (double)sumStreak / total;
        cout << "\n📊 Statistics:\n";
        cout << "  Total habits: " << total << "\n";
        cout << "  Total completions: " << totalLogs << "\n";
        cout << "  Average streak: " << fixed << setprecision(1) << avg << " days\n";
        cout << "  Longest streak: " << longest << " days (" << bestName << ")\n";
    }

    void reset(const string& id) {
        Habit* h = getHabit(id);
        if (!h) {
            cout << "❌ Habit " << id << " not found.\n";
            return;
        }
        h->reset();
        save();
    }

    void history(const string& id) {
        Habit* h = getHabit(id);
        if (!h) {
            cout << "❌ Habit " << id << " not found.\n";
            return;
        }
        cout << "\n📜 History for '" << h->name << "':\n";
        cout << "  Current streak: " << h->streak << " days\n";
        cout << "  Longest streak: " << h->longestStreak << " days\n";
        if (!h->history.empty()) {
            cout << "  Recent logs:\n";
            int start = max(0, (int)h->history.size() - 10);
            for (int i=start; i<(int)h->history.size(); i++) {
                cout << "    - " << h->history[i] << "\n";
            }
        } else {
            cout << "  No logs yet.\n";
        }
    }
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Usage: habit_tracker <command> [options]\n";
        return 1;
    }
    Tracker t;
    string cmd = argv[1];

    if (cmd == "add") {
        if (argc < 3) { cerr << "add <name> [--desc TEXT]\n"; return 1; }
        string name = argv[2];
        string desc;
        for (int i=3; i<argc; i++) {
            if (string(argv[i]) == "--desc" && i+1 < argc) desc = argv[++i];
        }
        t.add(name, desc);
    } else if (cmd == "log") {
        if (argc < 3) { cerr << "log <id> [--date YYYY-MM-DD]\n"; return 1; }
        string id = argv[2];
        string date;
        for (int i=3; i<argc; i++) {
            if (string(argv[i]) == "--date" && i+1 < argc) date = argv[++i];
        }
        t.log(id, date);
    } else if (cmd == "list") {
        t.list();
    } else if (cmd == "stats") {
        t.stats();
    } else if (cmd == "reset") {
        if (argc < 3) { cerr << "reset <id>\n"; return 1; }
        t.reset(argv[2]);
    } else if (cmd == "history") {
        if (argc < 3) { cerr << "history <id>\n"; return 1; }
        t.history(argv[2]);
    } else {
        cerr << "Unknown command.\n";
        return 1;
    }
    return 0;
}
