// HabitTracker.java
import java.io.*;
import java.nio.file.*;
import java.time.*;
import java.util.*;
import com.google.gson.*;

class Habit {
    String id;
    String name;
    String description;
    int streak;
    int longestStreak;
    String lastLogDate;
    List<String> history;
    String createdAt;

    Habit() {}
    Habit(String name, String description) {
        this.id = UUID.randomUUID().toString().substring(0,8);
        this.name = name;
        this.description = description;
        this.streak = 0;
        this.longestStreak = 0;
        this.lastLogDate = null;
        this.history = new ArrayList<>();
        this.createdAt = Instant.now().toString();
    }

    boolean log(String logDate) {
        if (logDate == null) logDate = LocalDate.now().toString();
        if (history.contains(logDate)) {
            System.out.printf("⚠️ Already logged for %s%n", logDate);
            return false;
        }
        if (lastLogDate != null) {
            LocalDate last = LocalDate.parse(lastLogDate);
            LocalDate current = LocalDate.parse(logDate);
            long days = ChronoUnit.DAYS.between(last, current);
            if (days == 1) streak++;
            else if (days > 1) streak = 1;
        } else {
            streak = 1;
        }
        if (streak > longestStreak) longestStreak = streak;
        lastLogDate = logDate;
        history.add(logDate);
        System.out.printf("✅ Logged '%s' for %s (streak: %d days)%n", name, logDate, streak);
        return true;
    }

    void reset() {
        streak = 0;
        lastLogDate = null;
        System.out.printf("🔄 Reset streak for '%s'%n", name);
    }
}

public class HabitTracker {
    private List<Habit> habits = new ArrayList<>();
    private final String dataFile = "habits.json";
    private final Gson gson = new GsonBuilder().setPrettyPrinting().create();

    public HabitTracker() { load(); }

    private void load() {
        try {
            Path path = Paths.get(dataFile);
            if (Files.exists(path)) {
                String json = new String(Files.readAllBytes(path));
                Habit[] arr = gson.fromJson(json, Habit[].class);
                habits = Arrays.asList(arr);
            }
        } catch (Exception e) {}
    }

    private void save() {
        try {
            Files.write(Paths.get(dataFile), gson.toJson(habits).getBytes());
        } catch (Exception e) {}
    }

    private Habit getHabit(String id) {
        for (Habit h : habits) {
            if (h.id.equals(id)) return h;
        }
        return null;
    }

    public void add(String name, String description) {
        Habit h = new Habit(name, description);
        habits.add(h);
        save();
        System.out.printf("✅ Habit added: %s (ID: %s)%n", h.name, h.id);
    }

    public void log(String id, String date) {
        Habit h = getHabit(id);
        if (h == null) {
            System.out.printf("❌ Habit %s not found.%n", id);
            return;
        }
        h.log(date);
        save();
    }

    public void list() {
        if (habits.isEmpty()) {
            System.out.println("No habits yet. Add one with 'add'");
            return;
        }
        System.out.println("\n🔥 Habit Tracker\n");
        System.out.println("📋 Your Habits:");
        for (int i = 0; i < habits.size(); i++) {
            Habit h = habits.get(i);
            String status = h.streak > 0 ? "✅" : "⏳";
            String days = h.streak > 0 ? "🔥" : "💤";
            System.out.printf("  %d. %s %s (%s %d days) – Best: %d days%n", i+1, status, h.name, days, h.streak, h.longestStreak);
        }
    }

    public void stats() {
        if (habits.isEmpty()) {
            System.out.println("No habits yet.");
            return;
        }
        int total = habits.size();
        int totalLogs = 0;
        int sumStreak = 0;
        int longest = 0;
        String bestName = "";
        for (Habit h : habits) {
            totalLogs += h.history.size();
            sumStreak += h.streak;
            if (h.longestStreak > longest) {
                longest = h.longestStreak;
                bestName = h.name;
            }
        }
        double avg = (double) sumStreak / total;
        System.out.println("\n📊 Statistics:");
        System.out.printf("  Total habits: %d%n", total);
        System.out.printf("  Total completions: %d%n", totalLogs);
        System.out.printf("  Average streak: %.1f days%n", avg);
        System.out.printf("  Longest streak: %d days (%s)%n", longest, bestName);
    }

    public void reset(String id) {
        Habit h = getHabit(id);
        if (h == null) {
            System.out.printf("❌ Habit %s not found.%n", id);
            return;
        }
        h.reset();
        save();
    }

    public void history(String id) {
        Habit h = getHabit(id);
        if (h == null) {
            System.out.printf("❌ Habit %s not found.%n", id);
            return;
        }
        System.out.printf("\n📜 History for '%s':%n", h.name);
        System.out.printf("  Current streak: %d days%n", h.streak);
        System.out.printf("  Longest streak: %d days%n", h.longestStreak);
        if (!h.history.isEmpty()) {
            System.out.println("  Recent logs:");
            int start = Math.max(0, h.history.size() - 10);
            for (int i = start; i < h.history.size(); i++) {
                System.out.printf("    - %s%n", h.history.get(i));
            }
        } else {
            System.out.println("  No logs yet.");
        }
    }

    public static void main(String[] args) throws Exception {
        if (args.length < 1) {
            System.out.println("Usage: HabitTracker <command> [options]");
            return;
        }
        HabitTracker t = new HabitTracker();
        String cmd = args[0];
        Map<String, String> params = new HashMap<>();
        for (int i=1; i<args.length; i++) {
            if (args[i].startsWith("--") && i+1 < args.length) {
                params.put(args[i].substring(2), args[++i]);
            }
        }
        switch (cmd) {
            case "add":
                if (args.length < 2) { System.out.println("add <name> [--desc TEXT]"); return; }
                t.add(args[1], params.getOrDefault("desc", ""));
                break;
            case "log":
                if (args.length < 2) { System.out.println("log <id> [--date YYYY-MM-DD]"); return; }
                t.log(args[1], params.get("date"));
                break;
            case "list":
                t.list();
                break;
            case "stats":
                t.stats();
                break;
            case "reset":
                if (args.length < 2) { System.out.println("reset <id>"); return; }
                t.reset(args[1]);
                break;
            case "history":
                if (args.length < 2) { System.out.println("history <id>"); return; }
                t.history(args[1]);
                break;
            default:
                System.out.println("Unknown command.");
        }
    }
}
