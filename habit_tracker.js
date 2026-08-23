// habit_tracker.js
#!/usr/bin/env node
const fs = require('fs');
const { program } = require('commander');
const { v4: uuidv4 } = require('uuid');

const DATA_FILE = 'habits.json';

class Habit {
    constructor(name, description = '') {
        this.id = uuidv4().slice(0,8);
        this.name = name;
        this.description = description;
        this.streak = 0;
        this.longest_streak = 0;
        this.last_log_date = null;
        this.history = [];
        this.created_at = new Date().toISOString();
    }
}

class Tracker {
    constructor() {
        this.habits = [];
        this.load();
    }

    load() {
        if (fs.existsSync(DATA_FILE)) {
            try {
                this.habits = JSON.parse(fs.readFileSync(DATA_FILE));
            } catch (e) {}
        }
    }

    save() {
        fs.writeFileSync(DATA_FILE, JSON.stringify(this.habits, null, 2));
    }

    getHabit(id) {
        return this.habits.find(h => h.id === id);
    }

    add(name, description) {
        const h = new Habit(name, description);
        this.habits.push(h);
        this.save();
        console.log(`✅ Habit added: ${h.name} (ID: ${h.id})`);
    }

    log(id, date) {
        const h = this.getHabit(id);
        if (!h) {
            console.log(`❌ Habit ${id} not found.`);
            return;
        }
        const logDate = date || new Date().toISOString().slice(0,10);
        // Check for duplicate
        if (h.history.includes(logDate)) {
            console.log(`⚠️ Already logged for ${logDate}`);
            return;
        }
        if (h.last_log_date) {
            const last = new Date(h.last_log_date);
            const current = new Date(logDate);
            const days = Math.floor((current - last) / (1000*60*60*24));
            if (days === 1) h.streak++;
            else if (days > 1) h.streak = 1;
        } else {
            h.streak = 1;
        }
        if (h.streak > h.longest_streak) {
            h.longest_streak = h.streak;
        }
        h.last_log_date = logDate;
        h.history.push(logDate);
        this.save();
        console.log(`✅ Logged '${h.name}' for ${logDate} (streak: ${h.streak} days)`);
    }

    list() {
        if (!this.habits.length) {
            console.log('No habits yet. Add one with "add"');
            return;
        }
        console.log('\n🔥 Habit Tracker\n');
        console.log('📋 Your Habits:');
        this.habits.forEach((h, i) => {
            const status = h.streak > 0 ? '✅' : '⏳';
            const days = h.streak > 0 ? '🔥' : '💤';
            console.log(`  ${i+1}. ${status} ${h.name} (${days} ${h.streak} days) – Best: ${h.longest_streak} days`);
        });
    }

    stats() {
        if (!this.habits.length) {
            console.log('No habits yet.');
            return;
        }
        const total = this.habits.length;
        const totalLogs = this.habits.reduce((sum, h) => sum + h.history.length, 0);
        const avgStreak = this.habits.reduce((sum, h) => sum + h.streak, 0) / total;
        const longest = Math.max(...this.habits.map(h => h.longest_streak));
        const best = this.habits.reduce((a, b) => a.longest_streak > b.longest_streak ? a : b);
        console.log('\n📊 Statistics:');
        console.log(`  Total habits: ${total}`);
        console.log(`  Total completions: ${totalLogs}`);
        console.log(`  Average streak: ${avgStreak.toFixed(1)} days`);
        console.log(`  Longest streak: ${longest} days (${best.name})`);
    }

    reset(id) {
        const h = this.getHabit(id);
        if (!h) {
            console.log(`❌ Habit ${id} not found.`);
            return;
        }
        h.streak = 0;
        h.last_log_date = null;
        this.save();
        console.log(`🔄 Reset streak for '${h.name}'`);
    }

    history(id) {
        const h = this.getHabit(id);
        if (!h) {
            console.log(`❌ Habit ${id} not found.`);
            return;
        }
        console.log(`\n📜 History for '${h.name}':`);
        console.log(`  Current streak: ${h.streak} days`);
        console.log(`  Longest streak: ${h.longest_streak} days`);
        if (h.history.length) {
            console.log('  Recent logs:');
            const recent = h.history.slice(-10);
            recent.forEach(d => console.log(`    - ${d}`));
        } else {
            console.log('  No logs yet.');
        }
    }
}

program
    .command('add <name>')
    .option('--desc <description>', 'Description')
    .action((name, options) => {
        const t = new Tracker();
        t.add(name, options.desc || '');
    });

program
    .command('log <id>')
    .option('--date <date>', 'YYYY-MM-DD')
    .action((id, options) => {
        const t = new Tracker();
        t.log(id, options.date);
    });

program
    .command('list')
    .action(() => {
        const t = new Tracker();
        t.list();
    });

program
    .command('stats')
    .action(() => {
        const t = new Tracker();
        t.stats();
    });

program
    .command('reset <id>')
    .action((id) => {
        const t = new Tracker();
        t.reset(id);
    });

program
    .command('history <id>')
    .action((id) => {
        const t = new Tracker();
        t.history(id);
    });

program.parse(process.argv);
