#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Max Courses
#define MAX_COURSES 10

// Saving File
#define DATA_FILE "students.txt"


// Structure for Course
typedef struct {
    char code[20];
    char name[50];
    double grades;
    double maxGrades;
} Course;

//Structure for Students Profile Record
typedef struct {
    int id;
    char firstName[50];
    char lastName[50];
    char programme[50];
    int year;
    int courseCount;
    Course courses[MAX_COURSES];
} Student;

static void flush_input(void) {
    int c;
    while((c = getchar()) !='\n' && c !=EOF) {}
}

static void read_line(const char *prompt, char *buf, size_t size) {
    printf("%s", prompt);
    if(fgets(buf, (int)size,stdin)) {
        buf[strcspn(buf, "\n")] = '\0';
    } else {
        buf[0] = '\0';
        clearerr(stdin);
    }
}

static int read_int(const char *prompt, int min, int max) {
    int v;
    while (1) {
        printf("%s", prompt);
        if(scanf("%d", &v) == 1 && v >= min && v <= max) {
            flush_input();
            return v;
        }
        printf("Enter a valid interger (%d-%d).\n", min, max);
        flush_input();
    }
}

static double read_double(const char *prompt, double min, double max) {
    double v;
    while (1) {
        printf("%s", prompt);
        if(scanf("%lf", &v) == 1 && v >= min && v <= max) {
        flush_input();
        return v;
    }
    printf("Enter a valid number (%.1f-%.1f)\n", min, max);
    flush_input();

    }
}

// Finding student by using ID from student records
static int find_by_id(const Student *list, int n, int id) {
    for (int i = 0; i < n; i++) {
        if ((list + i)->id == id) return i;
    }
    return -1;
}

// Calculating student's grades average
static double calc_avg(const Student *s) {
    if (!s || s->courseCount <= 0 ) return 0.0;
    double earned = 0.0, possible = 0.0;
    for (int i = 0; i < s->courseCount; i++) {
        earned += s->courses[i].grades;
        possible += s->courses[i].maxGrades;
    }
    return (possible > 0.0) ? (earned / possible) * 100.0 : 0.0;
}

// Check if student passed
static int has_passed(const Student *s) {
    if (!s || s->courseCount <= 0) return 0;
    for (int i = 0; i < s->courseCount; i++) {
        if (s->courses[i].maxGrades <= 0.0 || s->courses[i].grades < 0.5 * s->courses[i].maxGrades) return 0;
    }
    return 1;
}

// Displaying all students from current record
static void print_student(const Student *s) {
    printf("\nID: %d | %s %s | %s (Year %d) | Courses: %d | Avg: %.2f%% | %s\n",
           s->id, s->firstName, s->lastName, s->programme, s->year,
           s->courseCount, calc_avg(s),
           s->courseCount ? (has_passed(s) ? "PASS" : "FAIL") : "N/A");
    for (int i = 0; i < s->courseCount; i++) {
        const Course *c = &s->courses[i];
        printf("  - [%s] %s: %.1f / %.1f\n", c->code, c->name, c->grades, c->maxGrades);
    }
}

// Adding student
static Student *add_student(Student **list, int *n) {
    int id = read_int("Student ID: ", 1, 99999999);
    if (find_by_id(*list, *n, id) != -1) {
        printf("Duplicate ID %d rejected.\n", id);
        return *list;
    }

    Student *tmp = realloc(*list, (size_t)(*n + 1) * sizeof(Student));
    if (!tmp) {
        printf("Memory allocation failed.\n");
        return *list;
    }
    *list = tmp;

    Student *s = &(*list)[*n];
    memset(s, 0, sizeof(Student));

    s->id = id;
    read_line("First name: ", s->firstName, sizeof(s->firstName));
    read_line("Last name: ", s->lastName, sizeof(s->lastName));
    read_line("Programme: ", s->programme, sizeof(s->programme));
    s->year = read_int("Year of study (1-4): ", 1, 4);

    (*n)++;
    printf("Student registered.\n");
    return *list;
}

// Updating student records
static void update_student(Student *list, int n) {
    int idx = find_by_id(list, n, read_int("Student ID to update: ", 1, 99999999));
    if (idx == -1) { printf("Student not found.\n"); return; }
    Student *s = list + idx;
    read_line("New first name: ", s->firstName, sizeof(s->firstName));
    read_line("New last name: ", s->lastName, sizeof(s->lastName));
    read_line("New programme: ", s->programme, sizeof(s->programme));
    s->year = read_int("New year (1-3): ", 1, 3);
    printf("Record updated.\n");
}

// Deleting student from student record
static Student *delete_student(Student **list, int *n) {
    int idx = find_by_id(*list, *n, read_int("Student ID to delete: ", 1, 99999999));
    if (idx == -1) { printf("Student not found.\n"); return *list; }
    for (int i = idx; i < *n - 1; i++) (*list)[i] = (*list)[i + 1];
    (*n)--;
    if (*n == 0) {
        free(*list);
        *list = NULL;
    } else {
        Student *tmp = realloc(*list, (size_t)(*n) * sizeof(Student));
        if (tmp) *list = tmp;
    }
    printf("Student deleted.\n");
    return *list;
}

// Managing and updating the student's courses
static void manage_course(Student *list, int n, int is_update) {
    int idx = find_by_id(list, n, read_int("Student ID: ", 1, 99999999));
    if (idx == -1) { printf("Student not found.\n"); return; }
    Student *s = list + idx;

    if (!is_update) {
        if (s->courseCount >= MAX_COURSES) { printf("Course limit reached.\n"); return; }
        Course *c = &s->courses[s->courseCount];
        read_line("Course code: ", c->code, sizeof(c->code));
        read_line("Course name: ", c->name, sizeof(c->name));
        c->maxGrades = read_double("Maximum marks (>0): ", 0.1, 10000.0);
        c->grades = read_double("Marks obtained: ", 0.0, c->maxGrades);
        s->courseCount++;
        printf("Course added.\n");
    } else {
        if (s->courseCount == 0) { printf("No courses registered.\n"); return; }
        char code[16];
        read_line("Course code to update: ", code, sizeof(code));
        for (int i = 0; i < s->courseCount; i++) {
            if (strcmp(s->courses[i].code, code) == 0) {
                s->courses[i].grades = read_double("New marks: ", 0.0, s->courses[i].maxGrades);
                printf("Marks updated.\n");
                return;
            }
        }
        printf("Course not found.\n");
    }
}

// Searching student
static void search_students(Student *list, int n) {
    int mode = read_int("Search by: 1) ID  2) Name  3) Programme: ", 1, 3);
    int found = 0;
    if (mode == 1) {
        int id = read_int("Enter ID: ", 1, 99999999);
        int idx = find_by_id(list, n, id);
        if (idx != -1) { print_student(list + idx); found = 1; }
    } else {
        char q[40];
        read_line("Enter search text: ", q, sizeof(q));
        for (int i = 0; i < n; i++) {
            Student *s = list + i;
            if ((mode == 2 && (strstr(s->firstName, q) || strstr(s->lastName, q))) ||
                (mode == 3 && strstr(s->programme, q))) {
                print_student(s);
                found = 1;
            }
        }
    }
    if (!found) printf("No matching records.\n");
}

// Sorting student records
static void sort_students(Student *list, int n) {
    if (n < 2) { printf("Not enough records to sort.\n"); return; }
    int by = read_int("Sort by: \n1.ID  \n2.Name  \n3.Average  \n4.Programme \nEnter your choice:", 1, 4);
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            Student *a = list + j, *b = list + j + 1;
            int swap = 0;
            if (by == 1) swap = a->id > b->id;
            else if (by == 2) {
                int cmp = strcmp(a->lastName, b->lastName);
                swap = (cmp != 0) ? cmp > 0 : strcmp(a->firstName, b->firstName) > 0;
            } else if (by == 3) swap = calc_avg(a) < calc_avg(b);
            else if (by == 4) swap = strcmp(a->programme, b->programme) > 0;
            if (swap) { Student t = list[j]; list[j] = list[j + 1]; list[j + 1] = t; }
        }
    }
    printf("Records sorted.\n");
}

// Student report analysis
static void report_analytics(Student *list, int n) {
    if (n == 0) { printf("No students registered.\n"); return; }
    int totalCourses = 0, passCount = 0, failCount = 0;
    double sumAvg = 0.0, hi = -1.0, lo = 101.0;
    const Student *best = NULL, *worst = NULL;

    for (int i = 0; i < n; i++) {
        const Student *s = list + i;
        totalCourses += s->courseCount;
        if (s->courseCount > 0) {
            double avg = calc_avg(s);
            sumAvg += avg;
            if (avg > hi) { hi = avg; best = s; }
            if (avg < lo) { lo = avg; worst = s; }
            if (has_passed(s)) passCount++; else failCount++;
        }
    }

    // Printing studen academic performance report
    printf("\n=== Academic Performance Report ===\n");
    printf("Students: %d | Total Courses: %d | Overall Avg: %.2f%%\n",
           n, totalCourses, (n > 0) ? sumAvg / n : 0.0);
    printf("Passed: %d | Failed: %d\n", passCount, failCount);
    if (best) printf("Highest Avg: %s %s (%.2f%%)\n", best->firstName, best->lastName, hi);
    if (worst) printf("Lowest Avg: %s %s (%.2f%%)\n", worst->firstName, worst->lastName, lo);

    // Printing course's average
    printf("\n-- Average per Course --\n");
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < list[i].courseCount; j++) {
            const char *code = list[i].courses[j].code;
            int seen = 0;
            for (int x = 0; x <= i; x++) {
                int maxY = (x == i) ? j : list[x].courseCount;
                for (int y = 0; y < maxY; y++)
                    if (strcmp(list[x].courses[y].code, code) == 0) seen = 1;
            }
            if (seen && (i > 0 || j > 0)) continue;
            double pctSum = 0.0; int cnt = 0;
            for (int x = 0; x < n; x++)
                for (int y = 0; y < list[x].courseCount; y++)
                    if (strcmp(list[x].courses[y].code, code) == 0 && list[x].courseCount > 0) {
                        pctSum += (list[x].courses[y].grades / list[x].courses[y].maxGrades) * 100.0;
                        cnt++;
                    }
            if (cnt > 0) printf("  %s: %.2f%%\n", code, pctSum / cnt);
        }
    }

    printf("\n-- Students per Programme --\n");
    for (int i = 0; i < n; i++) {
        int dup = 0;
        for (int k = 0; k < i; k++) if (strcmp(list[k].programme, list[i].programme) == 0) dup = 1;
        if (dup) continue;
        int cnt = 0;
        for (int k = 0; k < n; k++) if (strcmp(list[k].programme, list[i].programme) == 0) cnt++;
        printf("  %s: %d\n", list[i].programme, cnt);
    }
}

// Saving student records
static void save_data(Student *list, int n) {
    FILE *fp = fopen(DATA_FILE, "wb");
    if (!fp) { printf("Error opening %s for writing.\n", DATA_FILE); return; }
    fwrite(&n, sizeof(int), 1, fp);
    for (int i = 0; i < n; i++) fwrite(&list[i], sizeof(Student), 1, fp);
    fclose(fp);
    printf("Saved %d student(s) to %s.\n", n, DATA_FILE);
}

// Loading data from file
static Student *load_data(Student **list, int *n) {
    FILE *fp = fopen(DATA_FILE, "rb");
    if (!fp) return *list;
    int count = 0;
    if (fread(&count, sizeof(int), 1, fp) != 1 || count <= 0) {
        fclose(fp);
        return *list;
    }
    Student *arr = malloc((size_t)count * sizeof(Student));
    if (!arr) {
        fclose(fp);
        return *list;
    }
    if (fread(arr, sizeof(Student), (size_t)count, fp) != (size_t)count) {
        free(arr);
        fclose(fp);
        return *list;
    }
    fclose(fp);
    free(*list);
    *list = arr;
    *n = count;
    printf("Loaded %d student(s) from %s.\n", *n, DATA_FILE);
    return *list;
}

static void free_all(Student *list) {
    free(list);
}


int main(void) {
    Student *students = NULL;
    int count = 0;
    students = load_data(&students, &count);

    while (1) {
        printf("\n==== Student and Academic Record ====");
        printf("\n1.Register \n2.Display All Students \n3.View \n4.Update \n5.Delete \n6.Add Course \n7.Update Marks \n8.Search \n9.Sort \n10.Report \n11.Save \n0.Exit\n");

        // Switch cases for user's input
        switch (read_int("Choice: ", 0, 11)) {
            case 1: students = add_student(&students, &count); break;
            case 2: for (int i = 0; i < count; i++) print_student(&students[i]); break;
            case 3: { int idx = find_by_id(students, count, read_int("ID: ", 1, 99999999));
                      if (idx != -1) print_student(&students[idx]); else printf("Not found.\n"); break; }
            case 4: update_student(students, count); break;
            case 5: students = delete_student(&students, &count); break;
            case 6: manage_course(students, count, 0); break;
            case 7: manage_course(students, count, 1); break;
            case 8: search_students(students, count); break;
            case 9: sort_students(students, count); break;
            case 10: report_analytics(students, count); break;
            case 11: save_data(students, count); break;
            case 0: save_data(students, count); free_all(students); return 0;
        }
    }
}