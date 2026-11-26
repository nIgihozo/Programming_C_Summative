#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define DATA_FILE "sale.txt"

// Structure for Txn
typedef struct {
    int id;
    char product[50];
    int quantity;
    double price;
    double total;
    char date[20];
} Txn;

typedef double (*MetricFn)(const Txn *arr, int n);
typedef int (*MatchFn)(const Txn *t, const void *ctx);
typedef int (*CmpFn)(const Txn *a, const Txn *b, int desc);
static void flush_in(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {}
}

static void read_str(const char *p, char *buf, size_t n) {
    printf("%s", p);
    if (fgets(buf, (int)n, stdin)) buf[strcspn(buf, "\n")] = '\0';
    else { buf[0] = '\0'; clearerr(stdin); }
}

static int read_int(const char *p, int lo, int hi) {
    int v;
    while (1) {
        printf("%s", p);
        if (scanf("%d", &v) == 1 && v >= lo && v <= hi) { flush_in(); return v; }
        printf("Invalid integer (%d-%d).\n", lo, hi); flush_in();
    }
}

static double read_dbl(const char *p, double lo, double hi) {
    double v;
    while (1) {
        printf("%s", p);
        if (scanf("%lf", &v) == 1 && v >= lo && v <= hi) { flush_in(); return v; }
        printf("Invalid number (%.2f-%.2f).\n", lo, hi); flush_in();
    }
}

static int find_id(const Txn *a, int n, int id) {
    for (int i = 0; i < n; i++) if (a[i].id == id) return i;
    return -1;
}

// Displaying transaction recorded
static void print_txn(const Txn *t) {
    printf("ID: %d | %-18s | Quantaty: %d | Price: %.2f | Total: %.2f | %s\n",
           t->id, t->product, t->quantity, t->price, t->total, t->date);
}

// Function pointer operations 
static double calc_rev(const Txn *a, int n) {
    double s = 0;
    for (int i = 0; i < n; i++) s += a[i].total;
    return s;
}
static double calc_avg(const Txn *a, int n) { return n ? calc_rev(a, n) / n : 0.0; }
static double calc_max(const Txn *a, int n) {
    double m = n ? a[0].total : 0.0;
    for (int i = 1; i < n; i++) if (a[i].total > m) m = a[i].total;
    return m;
}
static double calc_min(const Txn *a, int n) {
    double m = n ? a[0].total : 0.0;
    for (int i = 1; i < n; i++) if (a[i].total < m) m = a[i].total;
    return m;
}
static double calc_units(const Txn *a, int n) {
    double u = 0;
    for (int i = 0; i < n; i++) u += a[i].quantity;
    return u;
}

// Callback
static int match_min_amt(const Txn *t, const void *ctx) { return t->total >= *(const double *)ctx; }
static int match_prod(const Txn *t, const void *ctx) { return strstr(t->product, (const char *)ctx) != NULL; }
static int count_where(const Txn *a, int n, MatchFn pred, const void *ctx) {
    int c = 0;
    for (int i = 0; i < n; i++) if (pred(&a[i], ctx)) c++;
    return c;
}

// Filtering into transaction record
static void filter_cb(const Txn *a, int n, MatchFn pred, const void *ctx) {
    int hits = 0;
    for (int i = 0; i < n; i++) if (pred(&a[i], ctx)) { print_txn(&a[i]); hits++; }
    if (!hits) printf("No matching transactions exist.\n");
}

// Callback comparators for manual bubble sort 
static int cmp_id(const Txn *a, const Txn *b, int desc) { return desc ? b->id - a->id : a->id - b->id; }
static int cmp_prod(const Txn *a, const Txn *b, int desc) {
    int r = strcmp(a->product, b->product);
    return desc ? -r : r;
}
static int cmp_total(const Txn *a, const Txn *b, int desc) {
    if (a->total == b->total) return 0;
    return desc ? (b->total > a->total ? 1 : -1) : (a->total > b->total ? 1 : -1);
}

// Sorting records
static void sort_cb(Txn *a, int n, CmpFn cmp, int desc) {
    for (int i = 0; i < n - 1; i++)
        for (int j = 0; j < n - i - 1; j++)
            if (cmp(&a[j], &a[j + 1], desc) > 0) { Txn tmp = a[j]; a[j] = a[j + 1]; a[j + 1] = tmp; }
}

// Adding transaction
static Txn *add_txn(Txn *a, int *n) {
    int id = read_int("ID: ", 1, 99999999);
    if (find_id(a, *n, id) != -1) { printf("This ID is already exist. \nCreate new one!.\n"); return a; }
    Txn *p = realloc(a, (size_t)(*n + 1) * sizeof(Txn));
    if (!p) { printf("Memory error.\n"); return a; }
    a = p;
    Txn *t = &a[*n];
    read_str("Product: ", t->product, sizeof(t->product));
    t->quantity = read_int("Quantity: ", 1, 1000000);
    t->price = read_dbl("Unit price: ", 0.01, 10000000.0);
    read_str("Date (YYYY-MM-DD): ", t->date, sizeof(t->date));
    t->id = id;
    t->total = t->quantity * t->price;
    (*n)++;
    printf("Product Added Successful!\n");
    return a;
}

// Updating transaction
static void upd_txn(Txn *a, int n) {
    int idx = find_id(a, n, read_int("ID to update: ", 1, 99999999));
    if (idx == -1) { printf("Not found.\n"); return; }
    Txn *t = &a[idx];
    read_str("New product: ", t->product, sizeof(t->product));
    t->quantity = read_int("New quantity: ", 1, 1000000);
    t->price = read_dbl("New unit price: ", 0.01, 10000000.0);
    read_str("New date: ", t->date, sizeof(t->date));
    t->total = t->quantity * t->price;
    printf("Transaction Updated Successful!\n");
}

// Deleting transaction from records
static Txn *del_txn(Txn *a, int *n) {
    int idx = find_id(a, *n, read_int("ID to delete: ", 1, 99999999));
    if (idx == -1) { printf("ID doesn't found.\n"); return a; }
    for (int i = idx; i < *n - 1; i++) a[i] = a[i + 1];
    (*n)--;
    if (*n == 0) { free(a); printf("Transaction deleted successful!\n"); return NULL; }
    Txn *p = realloc(a, (size_t)(*n) * sizeof(Txn));
    
    return p ? p : a;
}

// Searching options
static void search_txns(const Txn *a, int n) {
    int m = read_int("\nSearch Option \n1. ID \n2. Product \n3. Amount>= \nEnter Your Choice: ", 1, 3);
    if (m == 1) {
        int idx = find_id(a, n, read_int("ID: ", 1, 99999999));
        if (idx != -1) print_txn(&a[idx]); else printf("Not found.\n");
    } else if (m == 2) {
        char q[50]; read_str("Product query: ", q, sizeof(q));
        filter_cb(a, n, match_prod, q);
    } else {
        double amt = read_dbl("Min amount: ", 0.0, 1e12);
        filter_cb(a, n, match_min_amt, &amt);
    }
}

// Generating transactions report
static void report(const Txn *a, int n) {
    char prod[50];
    read_str("Product for revenue summary: ", prod, sizeof(prod));
    double prodRev = 0;
    for (int i = 0; i < n; i++) if (strcmp(a[i].product, prod) == 0) prodRev += a[i].total;
    printf("\n--- Sales Report ---\nTransactions: %d | Units: %.0f | Revenue: %.2f\n",
           n, calc_units(a, n), calc_rev(a, n));
    printf("Avg: %.2f | High: %.2f | Low: %.2f | %s Revenue: %.2f\n",
           calc_avg(a, n), calc_max(a, n), calc_min(a, n), prod, prodRev);
}

// Saving transactions
static void save_txns(const Txn *a, int n) {
    FILE *fp = fopen(DATA_FILE, "wb");
    if (!fp) { printf("File write error.\n"); return; }
    if (fwrite(&n, sizeof(int), 1, fp) != 1 || (n > 0 && fwrite(a, sizeof(Txn), (size_t)n, fp) != (size_t)n))
        printf("Write incomplete.\n");
    else printf("Saved %d transaction(s).\n", n);
    fclose(fp);
}

// Loading transaction from saved one
static Txn *load_txns(Txn *a, int *n) {
    FILE *fp = fopen(DATA_FILE, "rb");
    if (!fp) { printf("No saved file found.\n"); return a; }
    int cnt = 0;
    if (fread(&cnt, sizeof(int), 1, fp) != 1 || cnt < 0) { fclose(fp); printf("Read error.\n"); return a; }
    Txn *buf = cnt ? malloc((size_t)cnt * sizeof(Txn)) : NULL;
    if (cnt > 0 && (!buf || fread(buf, sizeof(Txn), (size_t)cnt, fp) != (size_t)cnt)) {
        free(buf); fclose(fp); printf("Load error.\n"); return a;
    }
    fclose(fp);
    free(a);
    *n = cnt;
    printf("Loaded %d transaction(s).\n", *n);
    return buf;
}

int main(void) {
    Txn *txns = malloc(0);
    int count = 0;
    txns = load_txns(txns, &count);

    MetricFn metrics[] = { calc_rev, calc_avg, calc_max, calc_min, calc_units };
    const char *mnames[] = { "Total Revenue", "Avg Value", "Highest", "Lowest", "Total Units" };

    while (1) {
        printf("===== SALES TRANSACTION MANAGEMENT SYSTEM =====");
        printf("\n1.Add New Transaction \n2.Display All Transactions \n3.Update Transaction \n4.Delete Transaction \n5.Process Sales Data \n6.Filter Transactions (Callbacks) \n7.Search Transactions \n8.Sort Transactions \n9.Generate Sales Report \n10.Save Transactions \n11.Load Transactions \n0.Exit\n");
        int ch = read_int("Choice: ", 0, 11);
        if (ch == 0) { save_txns(txns, count); free(txns); return 0; }
        if (count == 0 && ch != 1 && ch != 11) { printf("No transactions recorded.\n"); continue; }

        // Switch Cases for inputs
        switch (ch) {
            case 1: txns = add_txn(txns, &count); break;
            case 2: for (int i = 0; i < count; i++) print_txn(&txns[i]); break;
            case 3: upd_txn(txns, count); break;
            case 4: txns = del_txn(txns, &count); break;
            case 5: {
                int op = read_int("\n1. Rev \n2)Avg \n3. High \n4)Low \n5. Units: ", 1, 5) - 1;
                printf("%s: %.2f\n", mnames[op], metrics[op](txns, count));
                break;
            }
            case 6: {
                int f = read_int("1)Amount>= 2)Product: ", 1, 2);
                if (f == 1) {
                    double v = read_dbl("Min total: ", 0.0, 1e12);
                    filter_cb(txns, count, match_min_amt, &v);
                    printf("Matching count: %d\n", count_where(txns, count, match_min_amt, &v));
                } else {
                    char p[50]; read_str("Product: ", p, sizeof(p));
                    filter_cb(txns, count, match_prod, p);
                    printf("Matching count: %d\n", count_where(txns, count, match_prod, p));
                }
                break;
            }
            case 7: search_txns(txns, count); break;
            case 8: {
                int s = read_int("\n1. ID \n2. Product \n3. Total \nEnter your choice: ", 1, 3);
                int d = read_int("0)Asc 1)Desc: ", 0, 1);
                CmpFn fns[] = { cmp_id, cmp_prod, cmp_total };
                sort_cb(txns, count, fns[s - 1], d);
                printf("Sorted.\n");
                break;
            }
            case 9: report(txns, count); break;
            case 10: save_txns(txns, count); break;
            case 11: txns = load_txns(txns, &count); break;
        }
    }
}