// habit_tracker.go
package main

import (
	"encoding/json"
	"flag"
	"fmt"
	"os"
	"time"
	"github.com/google/uuid"
)

type Habit struct {
	ID            string   `json:"id"`
	Name          string   `json:"name"`
	Description   string   `json:"description"`
	Streak        int      `json:"streak"`
	LongestStreak int      `json:"longest_streak"`
	LastLogDate   string   `json:"last_log_date"`
	History       []string `json:"history"`
	CreatedAt     string   `json:"created_at"`
}

func NewHabit(name, desc string) Habit {
	return Habit{
		ID:          uuid.New().String()[:8],
		Name:        name,
		Description: desc,
		Streak:      0,
		History:     []string{},
		CreatedAt:   time.Now().Format(time.RFC3339),
	}
}

func (h *Habit) Log(logDate string) bool {
	if logDate == "" {
		logDate = time.Now().Format("2006-01-02")
	}
	// Check if already logged today
	for _, d := range h.History {
		if d == logDate {
			fmt.Printf("⚠️ Already logged for %s\n", logDate)
			return false
		}
	}
	if h.LastLogDate != "" {
		last, _ := time.Parse("2006-01-02", h.LastLogDate)
		current, _ := time.Parse("2006-01-02", logDate)
		days := int(current.Sub(last).Hours() / 24)
		if days == 1 {
			h.Streak++
		} else if days > 1 {
			h.Streak = 1
		} else if days == 0 {
			// Same day, shouldn't happen due to duplicate check
		}
	} else {
		h.Streak = 1
	}
	if h.Streak > h.LongestStreak {
		h.LongestStreak = h.Streak
	}
	h.LastLogDate = logDate
	h.History = append(h.History, logDate)
	fmt.Printf("✅ Logged '%s' for %s (streak: %d days)\n", h.Name, logDate, h.Streak)
	return true
}

func (h *Habit) Reset() {
	h.Streak = 0
	h.LastLogDate = ""
	fmt.Printf("🔄 Reset streak for '%s'\n", h.Name)
}

type Tracker struct {
	Habits []Habit `json:"habits"`
}

var dataFile = "habits.json"

func (t *Tracker) load() {
	data, err := os.ReadFile(dataFile)
	if err != nil {
		return
	}
	json.Unmarshal(data, t)
}

func (t *Tracker) save() {
	data, _ := json.MarshalIndent(t, "", "  ")
	os.WriteFile(dataFile, data, 0644)
}

func (t *Tracker) getHabit(id string) *Habit {
	for i := range t.Habits {
		if t.Habits[i].ID == id {
			return &t.Habits[i]
		}
	}
	return nil
}

func (t *Tracker) add(name, desc string) {
	h := NewHabit(name, desc)
	t.Habits = append(t.Habits, h)
	t.save()
	fmt.Printf("✅ Habit added: %s (ID: %s)\n", h.Name, h.ID)
}

func (t *Tracker) log(id, date string) {
	h := t.getHabit(id)
	if h == nil {
		fmt.Printf("❌ Habit %s not found.\n", id)
		return
	}
	h.Log(date)
	t.save()
}

func (t *Tracker) list() {
	if len(t.Habits) == 0 {
		fmt.Println("No habits yet. Add one with 'add'")
		return
	}
	fmt.Println("\n🔥 Habit Tracker\n")
	fmt.Println("📋 Your Habits:")
	for i, h := range t.Habits {
		status := "✅"
		if h.Streak == 0 {
			status = "⏳"
		}
		days := "🔥"
		if h.Streak == 0 {
			days = "💤"
		}
		fmt.Printf("  %d. %s %s (%s %d days) – Best: %d days\n", i+1, status, h.Name, days, h.Streak, h.LongestStreak)
	}
}

func (t *Tracker) stats() {
	if len(t.Habits) == 0 {
		fmt.Println("No habits yet.")
		return
	}
	total := len(t.Habits)
	totalLogs := 0
	sumStreak := 0
	longest := 0
	bestHabit := ""
	for _, h := range t.Habits {
		totalLogs += len(h.History)
		sumStreak += h.Streak
		if h.LongestStreak > longest {
			longest = h.LongestStreak
			bestHabit = h.Name
		}
	}
	avg := float64(sumStreak) / float64(total)
	fmt.Println("\n📊 Statistics:")
	fmt.Printf("  Total habits: %d\n", total)
	fmt.Printf("  Total completions: %d\n", totalLogs)
	fmt.Printf("  Average streak: %.1f days\n", avg)
	fmt.Printf("  Longest streak: %d days (%s)\n", longest, bestHabit)
}

func (t *Tracker) reset(id string) {
	h := t.getHabit(id)
	if h == nil {
		fmt.Printf("❌ Habit %s not found.\n", id)
		return
	}
	h.Reset()
	t.save()
}

func (t *Tracker) history(id string) {
	h := t.getHabit(id)
	if h == nil {
		fmt.Printf("❌ Habit %s not found.\n", id)
		return
	}
	fmt.Printf("\n📜 History for '%s':\n", h.Name)
	fmt.Printf("  Current streak: %d days\n", h.Streak)
	fmt.Printf("  Longest streak: %d days\n", h.LongestStreak)
	if len(h.History) > 0 {
		fmt.Println("  Recent logs:")
		start := 0
		if len(h.History) > 10 {
			start = len(h.History) - 10
		}
		for _, d := range h.History[start:] {
			fmt.Printf("    - %s\n", d)
		}
	} else {
		fmt.Println("  No logs yet.")
	}
}

func main() {
	if len(os.Args) < 2 {
		fmt.Println("Usage: habit_tracker <command> [options]")
		return
	}
	t := &Tracker{}
	t.load()
	cmd := os.Args[1]

	switch cmd {
	case "add":
		addCmd := flag.NewFlagSet("add", flag.ExitOnError)
		name := addCmd.String("name", "", "")
		desc := addCmd.String("desc", "", "")
		addCmd.Parse(os.Args[2:])
		if *name == "" && len(addCmd.Args()) > 0 {
			*name = addCmd.Args()[0]
		}
		if *name == "" {
			fmt.Println("add requires a name")
			return
		}
		t.add(*name, *desc)

	case "log":
		logCmd := flag.NewFlagSet("log", flag.ExitOnError)
		id := logCmd.String("id", "", "")
		date := logCmd.String("date", "", "")
		logCmd.Parse(os.Args[2:])
		if *id == "" && len(logCmd.Args()) > 0 {
			*id = logCmd.Args()[0]
		}
		if *id == "" {
			fmt.Println("log requires an ID")
			return
		}
		t.log(*id, *date)

	case "list":
		t.list()

	case "stats":
		t.stats()

	case "reset":
		if len(os.Args) < 3 {
			fmt.Println("reset <id>")
			return
		}
		t.reset(os.Args[2])

	case "history":
		if len(os.Args) < 3 {
			fmt.Println("history <id>")
			return
		}
		t.history(os.Args[2])

	default:
		fmt.Println("Unknown command.")
	}
}
