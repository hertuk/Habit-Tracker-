🔥 Habit Tracker (Day Streaks) — Multi‑Language Habit Streak Manager
8 languages, one powerful habit tracker – build and maintain daily habits, track your streaks, and never break the chain – right from your terminal.

✨ Features
➕ Add habits – create new habits with a name and optional description

✅ Log completions – mark a habit as done for today (or any date)

📊 View streaks – see current streak length, longest streak, and history

📋 List all habits – with current streak, status, and progress

🔄 Reset streaks – manually reset a habit's streak (if you miss a day)

📈 Statistics – total habits, average streak, longest streak overall

💾 Persistent storage – all data saved in habits.json

🚀 Quick Start
All implementations follow the same CLI pattern:

bash
# Add a new habit
<command> add "Read 30 minutes" --desc "Daily reading"

# Log today's completion
<command> log 1

# Log completion for a specific date
<command> log 1 --date 2026-08-20

# List all habits with streaks
<command> list

# Show detailed stats
<command> stats

# Reset a habit's streak
<command> reset 1

# Show history of a habit
<command> history 1
Commands/Arguments:

add <name> [--desc TEXT] – add a new habit

log <id> [--date YYYY-MM-DD] – log completion for today or a specific date

list – show all habits with current streak

stats – show overall statistics

reset <id> – reset a habit's streak to 0

history <id> – show the completion history

📸 Example Output
text
🔥 Habit Tracker

📋 Your Habits:
  1. ✅ Read 30 minutes (🔥 7 days) – Best: 12 days
  2. ⏳ Exercise (🔥 3 days) – Best: 5 days
  3. ⏳ Meditation (🔥 0 days) – Best: 0 days

📊 Statistics:
  Total habits: 3
  Total completions: 45
  Average streak: 3.3 days
  Longest streak: 12 days (Read 30 minutes)
📁 Repository Structure
text
.
├── README.md
├── python/
│   └── habit_tracker.py
├── go/
│   └── habit_tracker.go
├── javascript/
│   └── habit_tracker.js
├── ruby/
│   └── habit_tracker.rb
├── php/
│   └── habit_tracker.php
├── java/
│   └── HabitTracker.java
├── csharp/
│   └── HabitTracker.cs
└── cpp/
    └── habit_tracker.cpp
