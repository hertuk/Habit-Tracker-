# habit_tracker.py
import json
import os
import sys
import argparse
import uuid
from datetime import datetime, timedelta, date

DATA_FILE = "habits.json"

class Habit:
    def __init__(self, name, description="", habit_id=None):
        self.id = habit_id or str(uuid.uuid4())[:8]
        self.name = name
        self.description = description
        self.streak = 0
        self.longest_streak = 0
        self.last_log_date = None
        self.history = []  # list of ISO date strings
        self.created_at = datetime.now().isoformat()

    def to_dict(self):
        return {
            "id": self.id,
            "name": self.name,
            "description": self.description,
            "streak": self.streak,
            "longest_streak": self.longest_streak,
            "last_log_date": self.last_log_date,
            "history": self.history,
            "created_at": self.created_at
        }

    @classmethod
    def from_dict(cls, data):
        habit = cls(data["name"], data.get("description", ""), data.get("id"))
        habit.streak = data.get("streak", 0)
        habit.longest_streak = data.get("longest_streak", 0)
        habit.last_log_date = data.get("last_log_date")
        habit.history = data.get("history", [])
        habit.created_at = data.get("created_at", datetime.now().isoformat())
        return habit

    def log(self, log_date=None):
        if log_date is None:
            log_date = date.today().isoformat()
        # Check if already logged today
        if log_date in self.history:
            print(f"⚠️ Already logged for {log_date}")
            return False
        # Check if this is a continuation or restart
        if self.last_log_date:
            last = datetime.fromisoformat(self.last_log_date).date()
            current = datetime.fromisoformat(log_date).date()
            # If logged yesterday or earlier in the same day, increment streak
            if (current - last).days == 1:
                self.streak += 1
            elif (current - last).days > 1:
                # Gap detected, reset streak
                self.streak = 1
            elif (current - last).days == 0:
                # Same day, should have been caught above
                self.streak = self.streak
        else:
            # First time logging
            self.streak = 1
        # Update longest streak
        if self.streak > self.longest_streak:
            self.longest_streak = self.streak
        self.last_log_date = log_date
        self.history.append(log_date)
        print(f"✅ Logged '{self.name}' for {log_date} (streak: {self.streak} days)")
        return True

    def reset(self):
        self.streak = 0
        self.last_log_date = None
        print(f"🔄 Reset streak for '{self.name}'")

class HabitTracker:
    def __init__(self):
        self.habits = []
        self.load()

    def load(self):
        if os.path.exists(DATA_FILE):
            with open(DATA_FILE, "r") as f:
                data = json.load(f)
                self.habits = [Habit.from_dict(h) for h in data]

    def save(self):
        with open(DATA_FILE, "w") as f:
            json.dump([h.to_dict() for h in self.habits], f, indent=2)

    def get_habit(self, habit_id):
        for h in self.habits:
            if h.id == habit_id:
                return h
        return None

    def add(self, name, description=""):
        habit = Habit(name, description)
        self.habits.append(habit)
        self.save()
        print(f"✅ Habit added: {habit.name} (ID: {habit.id})")

    def log(self, habit_id, log_date=None):
        habit = self.get_habit(habit_id)
        if not habit:
            print(f"❌ Habit {habit_id} not found.")
            return
        habit.log(log_date)
        self.save()

    def list(self):
        if not self.habits:
            print("No habits yet. Add one with 'add'")
            return
        print("\n🔥 Habit Tracker\n")
        print("📋 Your Habits:")
        for i, h in enumerate(self.habits, 1):
            status = "✅" if h.streak > 0 else "⏳"
            days = "🔥" if h.streak > 0 else "💤"
            print(f"  {i}. {status} {h.name} ({days} {h.streak} days) – Best: {h.longest_streak} days")

    def stats(self):
        if not self.habits:
            print("No habits yet.")
            return
        total = len(self.habits)
        total_logs = sum(len(h.history) for h in self.habits)
        avg_streak = sum(h.streak for h in self.habits) / total
        longest = max(h.longest_streak for h in self.habits)
        best_habit = max(self.habits, key=lambda h: h.longest_streak)
        print("\n📊 Statistics:")
        print(f"  Total habits: {total}")
        print(f"  Total completions: {total_logs}")
        print(f"  Average streak: {avg_streak:.1f} days")
        print(f"  Longest streak: {longest} days ({best_habit.name})")

    def reset(self, habit_id):
        habit = self.get_habit(habit_id)
        if not habit:
            print(f"❌ Habit {habit_id} not found.")
            return
        habit.reset()
        self.save()

    def history(self, habit_id):
        habit = self.get_habit(habit_id)
        if not habit:
            print(f"❌ Habit {habit_id} not found.")
            return
        print(f"\n📜 History for '{habit.name}':")
        print(f"  Current streak: {habit.streak} days")
        print(f"  Longest streak: {habit.longest_streak} days")
        if habit.history:
            print("  Recent logs:")
            for log_date in habit.history[-10:]:
                print(f"    - {log_date}")
        else:
            print("  No logs yet.")

def main():
    parser = argparse.ArgumentParser(description="Habit Tracker")
    subparsers = parser.add_subparsers(dest="cmd", required=True)

    add_parser = subparsers.add_parser("add")
    add_parser.add_argument("name")
    add_parser.add_argument("--desc", default="")

    log_parser = subparsers.add_parser("log")
    log_parser.add_argument("habit_id")
    log_parser.add_argument("--date", help="YYYY-MM-DD")

    subparsers.add_parser("list")
    subparsers.add_parser("stats")

    reset_parser = subparsers.add_parser("reset")
    reset_parser.add_argument("habit_id")

    history_parser = subparsers.add_parser("history")
    history_parser.add_argument("habit_id")

    args = parser.parse_args()
    tracker = HabitTracker()

    if args.cmd == "add":
        tracker.add(args.name, args.desc)
    elif args.cmd == "log":
        tracker.log(args.habit_id, args.date)
    elif args.cmd == "list":
        tracker.list()
    elif args.cmd == "stats":
        tracker.stats()
    elif args.cmd == "reset":
        tracker.reset(args.habit_id)
    elif args.cmd == "history":
        tracker.history(args.habit_id)

if __name__ == "__main__":
    main()
