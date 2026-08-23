// HabitTracker.cs
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text.Json;
using System.Text.Json.Serialization;

class Habit
{
    [JsonPropertyName("id")] public string Id { get; set; }
    [JsonPropertyName("name")] public string Name { get; set; }
    [JsonPropertyName("description")] public string Description { get; set; }
    [JsonPropertyName("streak")] public int Streak { get; set; }
    [JsonPropertyName("longest_streak")] public int LongestStreak { get; set; }
    [JsonPropertyName("last_log_date")] public string LastLogDate { get; set; }
    [JsonPropertyName("history")] public List<string> History { get; set; } = new List<string>();
    [JsonPropertyName("created_at")] public string CreatedAt { get; set; }

    public Habit() { }
    public Habit(string name, string description)
    {
        Id = Guid.NewGuid().ToString().Substring(0,8);
        Name = name;
        Description = description;
        Streak = 0;
        LongestStreak = 0;
        LastLogDate = null;
        CreatedAt = DateTime.Now.ToString("o");
    }

    public bool Log(string logDate)
    {
        if (string.IsNullOrEmpty(logDate)) logDate = DateTime.Now.ToString("yyyy-MM-dd");
        if (History.Contains(logDate))
        {
            Console.WriteLine($"⚠️ Already logged for {logDate}");
            return false;
        }
        if (!string.IsNullOrEmpty(LastLogDate))
        {
            var last = DateTime.Parse(LastLogDate);
            var current = DateTime.Parse(logDate);
            var days = (current - last).Days;
            if (days == 1) Streak++;
            else if (days > 1) Streak = 1;
        }
        else
        {
            Streak = 1;
        }
        if (Streak > LongestStreak) LongestStreak = Streak;
        LastLogDate = logDate;
        History.Add(logDate);
        Console.WriteLine($"✅ Logged '{Name}' for {logDate} (streak: {Streak} days)");
        return true;
    }

    public void Reset()
    {
        Streak = 0;
        LastLogDate = null;
        Console.WriteLine($"🔄 Reset streak for '{Name}'");
    }
}

class Tracker
{
    private List<Habit> habits = new List<Habit>();
    private readonly string dataFile = "habits.json";
    private readonly JsonSerializerOptions options = new JsonSerializerOptions { WriteIndented = true };

    public Tracker() => Load();

    private void Load()
    {
        if (!File.Exists(dataFile)) return;
        string json = File.ReadAllText(dataFile);
        habits = JsonSerializer.Deserialize<List<Habit>>(json) ?? new List<Habit>();
    }

    private void Save()
    {
        string json = JsonSerializer.Serialize(habits, options);
        File.WriteAllText(dataFile, json);
    }

    private Habit GetHabit(string id) => habits.FirstOrDefault(h => h.Id == id);

    public void Add(string name, string description)
    {
        var h = new Habit(name, description);
        habits.Add(h);
        Save();
        Console.WriteLine($"✅ Habit added: {h.Name} (ID: {h.Id})");
    }

    public void Log(string id, string date)
    {
        var h = GetHabit(id);
        if (h == null)
        {
            Console.WriteLine($"❌ Habit {id} not found.");
            return;
        }
        h.Log(date);
        Save();
    }

    public void List()
    {
        if (!habits.Any())
        {
            Console.WriteLine("No habits yet. Add one with 'add'");
            return;
        }
        Console.WriteLine("\n🔥 Habit Tracker\n");
        Console.WriteLine("📋 Your Habits:");
        for (int i = 0; i < habits.Count; i++)
        {
            var h = habits[i];
            string status = h.Streak > 0 ? "✅" : "⏳";
            string days = h.Streak > 0 ? "🔥" : "💤";
            Console.WriteLine($"  {i+1}. {status} {h.Name} ({days} {h.Streak} days) – Best: {h.LongestStreak} days");
        }
    }

    public void Stats()
    {
        if (!habits.Any())
        {
            Console.WriteLine("No habits yet.");
            return;
        }
        int total = habits.Count;
        int totalLogs = habits.Sum(h => h.History.Count);
        double avgStreak = habits.Average(h => h.Streak);
        int longest = habits.Max(h => h.LongestStreak);
        var best = habits.OrderByDescending(h => h.LongestStreak).First();
        Console.WriteLine("\n📊 Statistics:");
        Console.WriteLine($"  Total habits: {total}");
        Console.WriteLine($"  Total completions: {totalLogs}");
        Console.WriteLine($"  Average streak: {avgStreak:F1} days");
        Console.WriteLine($"  Longest streak: {longest} days ({best.Name})");
    }

    public void Reset(string id)
    {
        var h = GetHabit(id);
        if (h == null)
        {
            Console.WriteLine($"❌ Habit {id} not found.");
            return;
        }
        h.Reset();
        Save();
    }

    public void History(string id)
    {
        var h = GetHabit(id);
        if (h == null)
        {
            Console.WriteLine($"❌ Habit {id} not found.");
            return;
        }
        Console.WriteLine($"\n📜 History for '{h.Name}':");
        Console.WriteLine($"  Current streak: {h.Streak} days");
        Console.WriteLine($"  Longest streak: {h.LongestStreak} days");
        if (h.History.Any())
        {
            Console.WriteLine("  Recent logs:");
            var recent = h.History.Skip(Math.Max(0, h.History.Count - 10));
            foreach (var d in recent) Console.WriteLine($"    - {d}");
        }
        else
        {
            Console.WriteLine("  No logs yet.");
        }
    }

    static void Main(string[] args)
    {
        if (args.Length < 1)
        {
            Console.WriteLine("Usage: HabitTracker <command> [options]");
            return;
        }
        var t = new Tracker();
        var parsed = ParseArgs(args);
        string cmd = args[0];
        switch (cmd)
        {
            case "add":
                if (args.Length < 2) { Console.WriteLine("add <name> [--desc TEXT]"); return; }
                t.Add(args[1], parsed.GetValueOrDefault("desc", ""));
                break;
            case "log":
                if (args.Length < 2) { Console.WriteLine("log <id> [--date YYYY-MM-DD]"); return; }
                t.Log(args[1], parsed.GetValueOrDefault("date"));
                break;
            case "list":
                t.List();
                break;
            case "stats":
                t.Stats();
                break;
            case "reset":
                if (args.Length < 2) { Console.WriteLine("reset <id>"); return; }
                t.Reset(args[1]);
                break;
            case "history":
                if (args.Length < 2) { Console.WriteLine("history <id>"); return; }
                t.History(args[1]);
                break;
            default:
                Console.WriteLine("Unknown command.");
                break;
        }
    }

    static Dictionary<string, string> ParseArgs(string[] args)
    {
        var dict = new Dictionary<string, string>();
        for (int i = 1; i < args.Length; i++)
        {
            if (args[i].StartsWith("--") && i + 1 < args.Length)
                dict[args[i].Substring(2)] = args[++i];
        }
        return dict;
    }
}
