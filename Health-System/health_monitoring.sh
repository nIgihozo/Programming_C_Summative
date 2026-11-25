#!/usr/bin/env bash
set -u

LOG_FILE="${HOME}/sys_monitor_activity.log"
touch "$LOG_FILE" 2>/dev/null || LOG_FILE="/tmp/sys_monitor_activity.log"

log_msg() { printf '[%s] %s\n' "$(date '+%F %T')" "$1" | tee -a "$LOG_FILE"; }

need_cmd() { command -v "$1" >/dev/null 2>&1 || { echo "Missing command: $1"; return 1; }; }

# Reading Numbers
read_num() {
  local prompt="$1" min="$2" max="$3" val
  read -r -p "$prompt" val
  [[ "$val" =~ ^[0-9]+$ ]] && (( val >= min && val <= max )) && { printf '%s' "$val"; return 0; }
  echo "Invalid number (expected ${min}-${max})." >&2; return 1
}

# System Information
sys_info() {
  uname -a
  echo "Hostname: $(hostname) | Uptime: $(uptime -p 2>/dev/null || uptime) | Date: $(date)"
  log_msg "INFO: Viewed system information"
}

# CPU Monitoring
cpu_mon() {
  need_cmd top || return
  local thresh; thresh=$(read_num "CPU warning threshold (1-100): " 1 100) || return
  local idle used
  idle=$(LC_ALL=C top -bn1 | awk -F'[, ]+' '/Cpu\(s\)|%Cpu/{for(i=1;i<=NF;i++) if($i~/id\./||$(i+1)=="id") print $(i-1)}' | head -n1)
  used=$(awk -v i="${idle:-0}" 'BEGIN{printf "%.1f", 100-i}')
  echo "CPU Usage: ${used}%"
  ps -eo pid,comm,%cpu,%memory --sort=-%cpu | head -n 6
  awk -v u="$used" -v t="$thresh" 'BEGIN{exit !(u>t)}' && log_msg "WARN: CPU ${used}% > ${thresh}%" || log_msg "INFO: CPU ${used}% checked"
}

# Memory Monitoring
mem_mon() {
  need_cmd free || return
  local thresh; thresh=$(read_num "Memory warning threshold (1-100): " 1 100) || return
  read -r total used avail pct < <(free -m | awk '/^Memory:/{p=($2>0)?($3*100/$2):0; printf "%d %d %d %.1f\n",$2,$3,$7,p}')
  echo "Memory: ${used}MB / ${total}MB (Avail: ${avail}MB, Used: ${pct}%)"
  awk -v u="$pct" -v t="$thresh" 'BEGIN{exit !(u>t)}' && log_msg "WARN: Memory ${pct}% > ${thresh}%" || log_msg "INFO: Memory ${pct}% checked"
}

# Disk Monitoring
disk_mon() {
  need_cmd df || return
  local thresh; thresh=$(read_num "Disk warning threshold (1-100): " 1 100) || return
  df -hP | awk 'NR==1 || $5+0 >= 0 {print}'
  while read -r fs size used avail pct mnt; do
    local u=${pct%%%}; [[ "$u" =~ ^[0-9]+$ ]] && (( u >= thresh )) && log_msg "WARN: Disk $mnt at ${pct} (thresh ${thresh}%)"
  done < <(df -P | awk 'NR>1')
  log_msg "INFO: Disk check completed"
}

# Searching Process
proc_search() {
  read -r -p "Process name or PID: " q
  [[ -z "$q" ]] && { echo "Empty query."; return; }
  if [[ "$q" =~ ^[0-9]+$ ]]; then ps -fp "$q" 2>/dev/null || echo "PID $q not found."
  else pgrep -af "$q" || echo "No process matching '$q'."; fi
  log_msg "INFO: Searched process '$q'"
}

# Killing Process
proc_kill() {
  local pid; pid=$(read_num "PID to terminate: " 2 4194304) || return
  ps -p "$pid" -o pid=,uid=,comm= || { echo "PID $pid not found."; return; }
  read -r puid pcomm < <(ps -p "$pid" -o uid=,comm= | awk '{print $1,$2}')
  (( puid < 1000 || pid <= 100 )) && { echo "Blocked: protected system process."; log_msg "WARN: Blocked kill on PID $pid ($pcomm)"; return; }
  read -r -p "Terminate PID $pid ($pcomm)? [y/N]: " ans
  [[ "$ans" =~ ^[Yy]$ ]] && { kill "$pid" 2>/dev/null && log_msg "INFO: Terminated PID $pid ($pcomm)" || echo "Failed (Permission Denied or Exited)."; }
}

# Generating Report
gen_report() {
  read -r -p "Report directory: " dir
  mkdir -p "$dir" 2>/dev/null || { echo "Access Not Granted '$dir'."; return; }
  local file="${dir}/health_$(date '+%Y%m%d_%H%M%S').txt"
  {
    echo "=== System Health Report ($(date)) ==="
    uname -a; uptime; echo "--- Memory ---"; free -h; echo "--- Disk ---"; df -hP; echo "--- Top CPU ---"; ps -eo pid,comm,%cpu,%memory --sort=-%cpu | head -n 6
  } > "$file" 2>/dev/null && { echo "Saved: $file"; log_msg "INFO: Report saved to $file"; } || echo "Write failed."
}

# Clearing Logs
clear_log() {
  read -r -p "Clear activity log? [y/N]: " ans
  [[ "$ans" =~ ^[Yy]$ ]] && { : > "$LOG_FILE" && echo "Log cleared."; }
}

while true; do
  cat <<'EOF'

1) System Info       2) Monitor CPU      3) Monitor Memory   4) Monitor Disk     5) Search Process
6) Terminate Process    7) Health Report    8) View Log         9) Clear Log        0) Exit
EOF
  read -r -p "Choice: " ch
  case "$ch" in
    1) sys_info ;; 2) cpu_mon ;; 3) mem_mon ;; 4) disk_mon ;; 5) proc_search ;;
    6) proc_kill ;; 7) gen_report ;; 8) cat "$LOG_FILE" ;; 9) clear_log ;;
    0) log_msg "INFO: Exit"; break ;; *) echo "Invalid choice." ;;
  esac
done