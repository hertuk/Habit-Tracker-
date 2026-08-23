# habit_tracker.php
#!/usr/bin/env php
<?php

define('DATA_FILE', 'habits.json');

class Habit {
    public $id;
    public $name;
    public $description;
    public $streak;
    public $longest_streak;
    public $last_log_date;
    public $history;
    public $created_at;

    function __construct($name, $description = '') {
        $this->id = substr(bin2hex(random_bytes(4)), 0, 8);
        $this->name = $name;
        $this->description = $description;
        $this->streak = 0;
        $this->longest_streak = 0;
        $this->last_log_date = null;
        $this->history = [];
        $this->created_at = date('c');
    }

    function toArray() {
        return [
            'id' => $this->id,
            'name' => $this->name,
            'description' => $this->description,
            'streak' => $this->streak,
            'longest_streak' => $this->longest_streak,
            'last_log_date' => $this->last_log_date,
            'history' => $this->history,
            'created_at' => $this->created_at
        ];
    }

    static function fromArray($data) {
        $h = new self($data['name'], $data['description']);
        $h->id = $data['id'];
        $h->streak = $data['streak'] ?? 0;
        $h->longest_streak = $data['longest_streak'] ?? 0;
        $h->last_log_date = $data['last_log_date'] ?? null;
        $h->history = $data['history'] ?? [];
        $h->created_at = $data['created_at'] ?? date('c');
        return $h;
    }

    function log($logDate = null) {
        $logDate = $logDate ?: date('Y-m-d');
        if (in_array($logDate, $this->history)) {
            echo "⚠️ Already logged for $logDate\n";
            return false;
        }
        if ($this->last_log_date) {
            $last = new DateTime($this->last_log_date);
            $current = new DateTime($logDate);
            $diff = $current->diff($last)->days;
            if ($diff == 1) {
                $this->streak++;
            } elseif ($diff > 1) {
                $this->streak = 1;
            }
        } else {
            $this->streak = 1;
        }
        if ($this->streak > $this->longest_streak) {
            $this->longest_streak = $this->streak;
        }
        $this->last_log_date = $logDate;
        $this->history[] = $logDate;
        echo "✅ Logged '{$this->name}' for $logDate (streak: {$this->streak} days)\n";
        return true;
    }

    function reset() {
        $this->streak = 0;
        $this->last_log_date = null;
        echo "🔄 Reset streak for '{$this->name}'\n";
    }
}

class Tracker {
    private $habits = [];

    function __construct() {
        $this->load();
    }

    function load() {
        if (file_exists(DATA_FILE)) {
            $data = json_decode(file_get_contents(DATA_FILE), true);
            $this->habits = array_map(function($h) { return Habit::fromArray($h); }, $data);
        }
    }

    function save() {
        $data = array_map(function($h) { return $h->toArray(); }, $this->habits);
        file_put_contents(DATA_FILE, json_encode($data, JSON_PRETTY_PRINT));
    }

    function getHabit($id) {
        foreach ($this->habits as $h) {
            if ($h->id == $id) return $h;
        }
        return null;
    }

    function add($name, $description = '') {
        $h = new Habit($name, $description);
        $this->habits[] = $h;
        $this->save();
        echo "✅ Habit added: {$h->name} (ID: {$h->id})\n";
    }

    function log($id, $logDate = null) {
        $h = $this->getHabit($id);
        if (!$h) {
            echo "❌ Habit $id not found.\n";
            return;
        }
        $h->log($logDate);
        $this->save();
    }

    function list() {
        if (empty($this->habits)) {
            echo "No habits yet. Add one with 'add'\n";
            return;
        }
        echo "\n🔥 Habit Tracker\n";
        echo "📋 Your Habits:\n";
        foreach ($this->habits as $i => $h) {
            $status = $h->streak > 0 ? '✅' : '⏳';
            $days = $h->streak > 0 ? '🔥' : '💤';
            echo "  " . ($i+1) . ". $status {$h->name} ($days {$h->streak} days) – Best: {$h->longest_streak} days\n";
        }
    }

    function stats() {
        if (empty($this->habits)) {
            echo "No habits yet.\n";
            return;
        }
        $total = count($this->habits);
        $totalLogs = array_sum(array_map(function($h) { return count($h->history); }, $this->habits));
        $avgStreak = array_sum(array_map(function($h) { return $h->streak; }, $this->habits)) / $total;
        $longest = max(array_map(function($h) { return $h->longest_streak; }, $this->habits));
        $best = array_reduce($this->habits, function($a, $b) {
            return ($a->longest_streak > $b->longest_streak) ? $a : $b;
        });
        echo "\n📊 Statistics:\n";
        echo "  Total habits: $total\n";
        echo "  Total completions: $totalLogs\n";
        echo "  Average streak: " . round($avgStreak, 1) . " days\n";
        echo "  Longest streak: $longest days ({$best->name})\n";
    }

    function reset($id) {
        $h = $this->getHabit($id);
        if (!$h) {
            echo "❌ Habit $id not found.\n";
            return;
        }
        $h->reset();
        $this->save();
    }

    function history($id) {
        $h = $this->getHabit($id);
        if (!$h) {
            echo "❌ Habit $id not found.\n";
            return;
        }
        echo "\n📜 History for '{$h->name}':\n";
        echo "  Current streak: {$h->streak} days\n";
        echo "  Longest streak: {$h->longest_streak} days\n";
        if (!empty($h->history)) {
            echo "  Recent logs:\n";
            $recent = array_slice($h->history, -10);
            foreach ($recent as $d) {
                echo "    - $d\n";
            }
        } else {
            echo "  No logs yet.\n";
        }
    }
}

if ($argc < 2) {
    die("Usage: php habit_tracker.php <command> [options]\n");
}
$t = new Tracker();
$cmd = $argv[1];

switch ($cmd) {
    case 'add':
        if ($argc < 3) die("add <name> [--desc TEXT]\n");
        $name = $argv[2];
        $desc = '';
        for ($i=3; $i<$argc; $i++) {
            if ($argv[$i] == '--desc' && isset($argv[$i+1])) { $desc = $argv[++$i]; }
        }
        $t->add($name, $desc);
        break;

    case 'log':
        if ($argc < 3) die("log <id> [--date YYYY-MM-DD]\n");
        $id = $argv[2];
        $date = null;
        for ($i=3; $i<$argc; $i++) {
            if ($argv[$i] == '--date' && isset($argv[$i+1])) { $date = $argv[++$i]; }
        }
        $t->log($id, $date);
        break;

    case 'list':
        $t->list();
        break;

    case 'stats':
        $t->stats();
        break;

    case 'reset':
        if ($argc < 3) die("reset <id>\n");
        $t->reset($argv[2]);
        break;

    case 'history':
        if ($argc < 3) die("history <id>\n");
        $t->history($argv[2]);
        break;

    default:
        echo "Unknown command.\n";
}
?>
