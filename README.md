# Programming_C_Summative

A comprehensive five-project systems programming  covering embedded hardware simulation, Linux shell administration, dynamic memory management in C, function-pointer-driven transaction analytics, and POSIX multi-threaded document processing.

---

## Repository Structure

```text
Programming_C_Summative/
├── README.md
├── Project1_Arduino_Occupancy/
│   └── occupancy_monitor.ino
├── Project2_Linux_Monitor/
│   └── system_monitor.sh
├── Project3_Student_Records/
│   └── student_records.c
├── Project4_Sales_Transactions/
│   └── sales_system.c
└── Project5_Multi_Threading/
    └── threading.c
```
---
## Projects Overview
### Project	Platform / Tech	Core Systems Concepts
```text
+---+----------------------------------------------------+--------------------------+---------------------------------------------------------------------------------------------------+
| # | Project                                            | Platform / Tech          | Core Systems Concepts                                                                             |
+---+----------------------------------------------------+--------------------------+---------------------------------------------------------------------------------------------------+
| 1 | Smart Occupancy Monitoring & Access Control System | Arduino Uno (C/Tinkercad)| Non-blocking millis() timing, struct state management, edge-triggered buttons, 16x2 LCD & LEDs  |
| 2 | Interactive Linux Health & Process Monitor         | Linux Bash (ps, /proc)   | CPU/memory/disk threshold alerts, interactive process inspection, PID validation, safe kill      |
| 3 | Student Records Management System                  | C (stdio.h, stdlib.h)    | Dynamic memory (malloc, realloc, free), file persistence, manual linear search & bubble sort      |
| 4 | Sales Transaction Processing & Analysis System     | C (Function Pointers)    | Runtime metric dispatch (MetricFn), callback filtering (MatchFn) & sorting (CmpFn), sale.txt     |
| 5 | Multi-Threaded Document Search & Analysis System   | C (pthread.h)            | Concurrent file processing via POSIX threads, lock-free per-thread Task structs, case-insensitive |
+---+----------------------------------------------------+--------------------------+---------------------------------------------------------------------------------------------------+
```

--- 
### Project 1: Smart Occupancy Monitoring & Access Control System
- Environment: Tinkercad Circuits or Arduino IDE (Arduino Uno).

- Setup: Wire entry/exit push buttons, status LEDs, and a 16x2 LCD according to the pin definitions at the top of occupancy_monitor.ino.

- Execution: Upload the sketch and open the Serial Monitor at 9600 baud to observe real-time occupancy counts, capacity alerts, and non-blocking status updates.

### Project 2: Interactive Linux Health & Process Monitor
- Requirements: Bash shell on Linux (or WSL/Git Bash with standard GNU utilities).

Run:
```
Bash
cd Project2_Linux_Monitor
chmod +x system_monitor.sh
./system_monitor.sh
```
---
- Features: Displays real-time CPU, RAM, and disk usage with threshold warnings, lists top resource-consuming processes, and safely terminates selected PIDs after confirmation.

### Project 3: Student Records Management System
Compile & Run:

```
Bash
cd Project3_Student_Records
gcc -Wall -Wextra -std=c11 student_records.c -o student_records
./student_records
```

- Features: Add, view, update, and delete student records in dynamically resized heap memory; search manually; sort manually; and persist records to disk.

### Project 4: Sales Transaction Processing & Analysis System
Compile & Run:
```
Bash
cd Project4_Sales_Transactions
gcc -Wall -Wextra -std=c11 sales_system.c -o sales_system
./sales_system
```
--- 
### Technical Summary:

- Structure: Txn struct, modular static helper functions, and a menu-driven main loop.

- Memory: realloc and free manage the dynamic Txn array.

- Function Pointers & Callbacks: MetricFn runs metrics; MatchFn filters and counts; CmpFn sorts.

- Files & Checks: Binary save/load in sale.txt with input, ID, and empty-list checks.

### Project 5: Multi-Threaded Document Search & Analysis System
Compile & Run:
```
Bash
cd Project5_Multi_Threading
gcc -Wall -Wextra -pthread threading.c -o threading
./threading
```
---

### Execution Flow:

- Enter a search keyword (e.g., IoT) and the number of input files to generate (e.g., 3).

- The program creates document1.txt through documentN.txt and spawns one POSIX thread (pthread_create) per file.

- Each thread operates independently on its own Task struct—counting total lines, performing case-insensitive keyword searches, recording matching line numbers, and writing documentX_report.txt—without requiring mutex locks.

- pthread_join() ensures all worker threads finish before printing the final processing summary.

### Error Handling & Defensive Practices
- **Input Validation:** Bounded integer/floating-point parsers flush malformed stdin buffers cleanly without infinite loops.

- **Memory Safety**: Every malloc/calloc/realloc allocation is checked for NULL and paired with free() prior to exit.

- **File & Thread Resilience:** Missing files, write failures, duplicate IDs, empty datasets, and thread creation errors are caught and reported without crashing.
