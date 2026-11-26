#include <ctype.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LINE_BUF 2048
#define MAX_MATCH_LINES 512
#define MAX_FILES 32

typedef struct {
    int idx;
    char file[64];
    char key[128];
    int total_lines;
    int occurrences;
    int matching_lines;
    int match_nums[MAX_MATCH_LINES];
    int truncated;
    int ok;
    char err[128];
} Task;

static void flush_in(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {}
}

static void read_line_str(const char *prompt, char *buf, size_t sz) {
    printf("%s", prompt);
    if (fgets(buf, (int)sz, stdin)) {
        buf[strcspn(buf, "\n")] = '\0';
    } else {
        buf[0] = '\0';
        clearerr(stdin);
    }
}

static int read_file_count(void) {
    int n = 0;
    while (1) {
        printf("Number of input files (1-%d): ", MAX_FILES);
        if (scanf("%d", &n) == 1 && n >= 1 && n <= MAX_FILES) {
            flush_in();
            return n;
        }
        printf("Enter a valid number from 1 to %d.\n", MAX_FILES);
        flush_in();
    }
}

static char *str_casestr(const char *hay, const char *needle) {
    if (!*needle) return (char *)hay;
    size_t nlen = strlen(needle);
    for (; *hay; hay++) {
        size_t i = 0;
        while (i < nlen && hay[i] &&
               tolower((unsigned char)hay[i]) == tolower((unsigned char)needle[i])) {
            i++;
        }
        if (i == nlen) return (char *)hay;
    }
    return NULL;
}

static int count_line_matches(const char *line, const char *key) {
    int cnt = 0;
    size_t klen = strlen(key);
    const char *p = line;
    if (!klen) return 0;
    while ((p = str_casestr(p, key)) != NULL) {
        cnt++;
        p += klen;
    }
    return cnt;
}

static int create_input_file(const char *filename, const char *key, int idx) {
    FILE *fp = fopen(filename, "w");
    if (!fp) return 0;
    fprintf(fp, "Document %d overview for %s systems.\n", idx, key);
    fprintf(fp, "Line 2 discusses %s architecture and deployment.\n", key);
    fprintf(fp, "Line 3 contains general notes without the target term.\n");
    fprintf(fp, "Line 4 highlights %s performance and reliability.\n", key);
    fclose(fp);
    return 1;
}

static void init_task(Task *t, int idx, const char *file, const char *key) {
    memset(t, 0, sizeof(*t));
    t->idx = idx;
    snprintf(t->file, sizeof(t->file), "%s", file);
    snprintf(t->key, sizeof(t->key), "%s", key);
    snprintf(t->err, sizeof(t->err), "None");
}

static void build_report_name(const char *in_file, char *out, size_t sz) {
    char base[128];
    snprintf(base, sizeof(base), "%s", in_file);
    char *dot = strrchr(base, '.');
    if (dot) *dot = '\0';
    snprintf(out, sz, "%s_report.txt", base);
}

static int write_report(Task *t) {
    char rpath[160];
    build_report_name(t->file, rpath, sizeof(rpath));
    FILE *out = fopen(rpath, "w");
    if (!out) {
        printf(t->err, sizeof(t->err), "Failed to create report %s", rpath);
        return 0;
    }
    fprintf(out, "Input File: %s\n", t->file);
    fprintf(out, "Search Keyword: %s\n", t->key);
    fprintf(out, "Status: %s\n", t->ok ? "Success" : "Failed");
    fprintf(out, "Total Lines: %d\n", t->total_lines);
    fprintf(out, "Total Occurrences: %d\n", t->occurrences);
    fprintf(out, "Matching Lines Count: %d\n", t->matching_lines);
    fprintf(out, "Matching Line Numbers: ");
    if (t->matching_lines == 0) {
        fprintf(out, "None");
    } else {
        int limit = t->matching_lines < MAX_MATCH_LINES ? t->matching_lines : MAX_MATCH_LINES;
        for (int i = 0; i < limit; i++) {
            fprintf(out, "%s%d", i ? ", " : "", t->match_nums[i]);
        }
        if (t->truncated) fprintf(out, " (truncated)");
    }
    fprintf(out, "\nError Details: %s\n", t->err);
    fclose(out);
    return 1;
}

static void *process_file(void *arg) {
    Task *t = (Task *)arg;
    printf("[Thread %d] Processing %s ...\n", t->idx, t->file);

    FILE *fp = fopen(t->file, "r");
    if (!fp) {
        snprintf(t->err, sizeof(t->err), "Cannot open input file");
        printf("[Thread %d] Error processing %s: %s\n", t->idx, t->file, t->err);
        return NULL;
    }

    char buf[LINE_BUF];
    int line_no = 0;
    while (fgets(buf, sizeof(buf), fp)) {
        line_no++;
        int hits = count_line_matches(buf, t->key);
        if (hits > 0) {
            t->occurrences += hits;
            if (t->matching_lines < MAX_MATCH_LINES) {
                t->match_nums[t->matching_lines] = line_no;
            } else {
                t->truncated = 1;
            }
            t->matching_lines++;
        }
        while (!strchr(buf, '\n') && fgets(buf, sizeof(buf), fp)) {
            t->occurrences += count_line_matches(buf, t->key);
        }
    }
    t->total_lines = line_no;
    fclose(fp);

     t->ok = 1;
    if (!write_report(t)) {
        printf("[Thread %d] Error processing %s: %s\n", t->idx, t->file, t->err);
        return NULL;
    }
    
    printf("[Thread %d] Successfully processed %s\n", t->idx, t->file);
    return NULL;
}

static void print_summary(const Task *tasks, int n) {
    printf("\n===== PROCESSING SUMMARY =====\n");
    int all_ok = 1;
    for (int i = 0; i < n; i++) {
        if (tasks[i].ok) {
            printf("%s: Completed\n", tasks[i].file);
        } else {
            all_ok = 0;
            printf("%s: Failed (%s)\n", tasks[i].file, tasks[i].err);
        }
    }
    printf("%s\n", all_ok
        ? "All processing tasks completed.\nReports generated successfully."
        : "Processing finished with errors.");
}

int main(void) {
    char key[128] = {0};
    while (strlen(key) == 0) {
        read_line_str("Enter search keyword: ", key, sizeof(key));
        if (strlen(key) == 0) printf("Keyword cannot be empty.\n");
    }

    int n = read_file_count();

    printf("\n===== MULTI-THREADED DOCUMENT ANALYZER =====\n");
    printf("Search Keyword: %s\n", key);
    printf("Number of Input Files: %d\n", n);

    pthread_t *tids = calloc((size_t)n, sizeof(pthread_t));
    Task *tasks = calloc((size_t)n, sizeof(Task));
    int *started = calloc((size_t)n, sizeof(int));
    if (!tids || !tasks || !started) {
        printf("Memory allocation failed.\n");
        free(tids); free(tasks); free(started);
        return 1;
    }

    for (int i = 0; i < n; i++) {
        char fname[64];
        snprintf(fname, sizeof(fname), "document%d.txt", i + 1);
        init_task(&tasks[i], i + 1, fname, key);

        if (!create_input_file(fname, key, i + 1)) {
            snprintf(tasks[i].err, sizeof(tasks[i].err), "Failed to create input file");
            printf("[Thread %d] Error creating %s\n", i + 1, fname);
            continue;
        }

        int rc = pthread_create(&tids[i], NULL, process_file, &tasks[i]);
        if (rc != 0) {
            snprintf(tasks[i].err, sizeof(tasks[i].err), "Thread creation failed (code %d)", rc);
            printf("[Thread %d] Error creating thread for %s\n", i + 1, fname);
        } else {
            started[i] = 1;
        }
    }

    for (int i = 0; i < n; i++) {
        if (started[i]) pthread_join(tids[i], NULL);
    }

    print_summary(tasks, n);
    free(tids);
    free(tasks);
    free(started);
    return 0;
}