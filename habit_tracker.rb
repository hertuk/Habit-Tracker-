# habit_tracker.rb
#!/usr/bin/env ruby
require 'json'
require 'securerandom'
require 'date'

DATA_FILE = 'habits.json'

class Habit
  attr_accessor :id, :name, :description, :streak, :longest_streak, :last_log_date, :history, :created_at

  def initialize(name, description = '')
    @id = SecureRandom.hex(4)
    @name = name
    @description = description
    @streak = 0
    @longest_streak = 0
    @last_log_date = nil
    @history = []
    @created_at = Time.now.iso8601
  end

  def to_hash
    {
      id: @id, name: @name, description: @description,
      streak: @streak, longest_streak: @longest_streak,
      last_log_date: @last_log_date, history: @history,
      created_at: @created_at
    }
  end

  def self.from_hash(h)
    habit = new(h['name'], h['description'])
    habit.id = h['id']
    habit.streak = h['streak'] || 0
    habit.longest_streak = h['longest_streak'] || 0
    habit.last_log_date = h['last_log_date']
    habit.history = h['history'] || []
    habit.created_at = h['created_at'] || Time.now.iso8601
    habit
  end

  def log(log_date = nil)
    log_date ||= Date.today.to_s
    if @history.include?(log_date)
      puts "⚠️ Already logged for #{log_date}"
      return false
    end
    if @last_log_date
      last = Date.parse(@last_log_date)
      current = Date.parse(log_date)
      days = (current - last).to_i
      if days == 1
        @streak += 1
      elsif days > 1
        @streak = 1
      end
    else
      @streak = 1
    end
    if @streak > @longest_streak
      @longest_streak = @streak
    end
    @last_log_date = log_date
    @history << log_date
    puts "✅ Logged '#{@name}' for #{log_date} (streak: #{@streak} days)"
    true
  end

  def reset
    @streak = 0
    @last_log_date = nil
    puts "🔄 Reset streak for '#{@name}'"
  end
end

class Tracker
  attr_reader :habits

  def initialize
    @habits = []
    load
  end

  def load
    if File.exist?(DATA_FILE)
      data = JSON.parse(File.read(DATA_FILE))
      @habits = data.map { |h| Habit.from_hash(h) }
    end
  end

  def save
    File.write(DATA_FILE, JSON.pretty_generate(@habits.map(&:to_hash)))
  end

  def get_habit(id)
    @habits.find { |h| h.id == id }
  end

  def add(name, description = '')
    h = Habit.new(name, description)
    @habits << h
    save
    puts "✅ Habit added: #{h.name} (ID: #{h.id})"
  end

  def log(id, log_date = nil)
    h = get_habit(id)
    unless h
      puts "❌ Habit #{id} not found."
      return
    end
    h.log(log_date)
    save
  end

  def list
    if @habits.empty?
      puts "No habits yet. Add one with 'add'"
      return
    end
    puts "\n🔥 Habit Tracker\n"
    puts "📋 Your Habits:"
    @habits.each_with_index do |h, i|
      status = h.streak > 0 ? '✅' : '⏳'
      days = h.streak > 0 ? '🔥' : '💤'
      puts "  #{i+1}. #{status} #{h.name} (#{days} #{h.streak} days) – Best: #{h.longest_streak} days"
    end
  end

  def stats
    if @habits.empty?
      puts "No habits yet."
      return
    end
    total = @habits.size
    total_logs = @habits.sum { |h| h.history.size }
    avg_streak = @habits.sum(&:streak).to_f / total
    longest = @habits.max_by(&:longest_streak)
    puts "\n📊 Statistics:"
    puts "  Total habits: #{total}"
    puts "  Total completions: #{total_logs}"
    puts "  Average streak: #{avg_streak.round(1)} days"
    puts "  Longest streak: #{longest.longest_streak} days (#{longest.name})"
  end

  def reset(id)
    h = get_habit(id)
    unless h
      puts "❌ Habit #{id} not found."
      return
    end
    h.reset
    save
  end

  def history(id)
    h = get_habit(id)
    unless h
      puts "❌ Habit #{id} not found."
      return
    end
    puts "\n📜 History for '#{h.name}':"
    puts "  Current streak: #{h.streak} days"
    puts "  Longest streak: #{h.longest_streak} days"
    if h.history.any?
      puts "  Recent logs:"
      h.history.last(10).each { |d| puts "    - #{d}" }
    else
      puts "  No logs yet."
    end
  end
end

if ARGV.empty?
  puts "Usage: habit_tracker.rb <command> [options]"
  exit
end

t = Tracker.new
cmd = ARGV.shift

case cmd
when 'add'
  if ARGV.empty?
    puts "add <name> [--desc TEXT]"
    exit
  end
  name = ARGV.shift
  desc = ''
  if ARGV.include?('--desc')
    idx = ARGV.index('--desc')
    desc = ARGV[idx+1] if idx
  end
  t.add(name, desc)

when 'log'
  if ARGV.empty?
    puts "log <id> [--date YYYY-MM-DD]"
    exit
  end
  id = ARGV.shift
  date = nil
  if ARGV.include?('--date')
    idx = ARGV.index('--date')
    date = ARGV[idx+1] if idx
  end
  t.log(id, date)

when 'list'
  t.list

when 'stats'
  t.stats

when 'reset'
  id = ARGV.shift
  if id.nil?
    puts "reset <id>"
    exit
  end
  t.reset(id)

when 'history'
  id = ARGV.shift
  if id.nil?
    puts "history <id>"
    exit
  end
  t.history(id)

else
  puts "Unknown command."
end
