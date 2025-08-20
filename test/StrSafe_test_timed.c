#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "StrSafe.h"

#define NUM_TESTS 1000
#define MAX_LEN 6400

#include <time.h>

long long get_nanoseconds(struct timespec start, struct timespec end) {
	return (end.tv_sec - start.tv_sec) * 1000000000LL + (end.tv_nsec - start.tv_nsec);
}

// Utility: Generate a random lowercase string
char* random_string(size_t len) {
    char* str = malloc(len + 1);
    for (size_t i = 0; i < len; ++i)
        str[i] = 'a' + rand() % 26;
    str[len] = '\0';
    return str;
}

// Utility: Ensure needle is in haystack in 50% of cases
char* generate_haystack(const char* needle, bool ensure_contains) {
    size_t base_len = rand() % (MAX_LEN - strlen(needle));
    char* base = random_string(base_len);
    if (ensure_contains) {
        char* result = malloc(base_len + strlen(needle) + 1);
        size_t insert_pos = rand() % (base_len + 1);
        strncpy(result, base, insert_pos);
        strcpy(result + insert_pos, needle);
        strcpy(result + insert_pos + strlen(needle), base + insert_pos);
        free(base);
        return result;
    }
    return base;
}

void log_header_time(FILE* f, const char* func_name, long long duration_ns) {
	fprintf(f, "\n=== %s ===\n", func_name);
	fprintf(f, "Duration (ns): %lld\n", duration_ns);
}
// Logging
void log_header(FILE* f, const char* func_name) {
    fprintf(f, "\n=== %s ===\n", func_name);
}

// Test: strsafe_set
void test_strsafe_set(FILE* f) {
    log_header(f, "strsafe_set");
    for (int i = 0; i < NUM_TESTS; ++i) {
        char* src = random_string(rand() % MAX_LEN);
        StrSafe s;
        strsafe_init(&s);
        strsafe_set(&s, src);
        fprintf(f, "%s,%s\n", src, s.data);
        strsafe_free(&s);
        free(src);
    }
}

// Test: strsafe_compare
void test_strsafe_compare(FILE* f) {
    log_header(f, "strsafe_compare");
    for (int i = 0; i < NUM_TESTS; ++i) {
        char* a_str = random_string(5);
        char* b_str = (i % 2 == 0) ? strdup(a_str) : random_string(5);
        StrSafe a;
        strsafe_init(&a);
        strsafe_set(&a, a_str);
        bool result = strsafe_compare(&a, &(StrSafe){.data = b_str, .len = strlen(b_str)});
        fprintf(f, "%s,%s,%s\n", a_str, b_str, result ? "true" : "false");
        strsafe_free(&a);
        free(a_str);
        free(b_str);
    }
}

// Test: strsafe_copy
void test_strsafe_copy(FILE* f) {
    log_header(f, "strsafe_copy");
    for (int i = 0; i < NUM_TESTS; ++i) {
        char* src_str = random_string(rand() % MAX_LEN);
        StrSafe src, dst;
        strsafe_init(&src);
        strsafe_init(&dst);
        strsafe_set(&src, src_str);
        strsafe_copy(&dst, &src);
        fprintf(f, "%s,%s\n", src.data, dst.data);
        strsafe_free(&src);
        strsafe_free(&dst);
        free(src_str);
    }
}

// Test: strsafe_append
void test_strsafe_append(FILE* f) {
    log_header(f, "strsafe_append");
    for (int i = 0; i < NUM_TESTS; ++i) {
        char* base = random_string(5);
        char* suffix = random_string(5);
        StrSafe s, suf;
        strsafe_init(&s);
        strsafe_init(&suf);
        strsafe_set(&s, base);
        strsafe_set(&suf, suffix);
        strsafe_append(&s, &suf);
        fprintf(f, "%s,%s,%s\n", base, suffix, s.data);
        strsafe_free(&s);
        strsafe_free(&suf);
        free(base);
        free(suffix);
    }
}

// Test: strsafe_substr
void test_strsafe_substr(FILE* f) {
    log_header(f, "strsafe_substr");
    for (int i = 0; i < NUM_TESTS; ++i) {
        char* base = random_string(10);
        StrSafe s, sub;
        strsafe_init(&s);
        strsafe_init(&sub);
        strsafe_set(&s, base);
        size_t pos = rand() % 5;
        size_t len = rand() % 5;
        strsafe_substr(&sub, pos, len);
        fprintf(f, "%s,%zu,%zu,%s\n", base, pos, len, sub.data);
        strsafe_free(&s);
        strsafe_free(&sub);
        free(base);
    }
}

// Test: strsafe_replace_all
void test_strsafe_replace_all(FILE* f) {
    log_header(f, "strsafe_replace_all");
    for (int i = 0; i < NUM_TESTS; ++i) {
        char* old_str = random_string(2);
        char* new_str = random_string(2);
        bool should_contain = i < NUM_TESTS / 2;
        char* base = generate_haystack(old_str, should_contain);
        StrSafe s, old, new;
        strsafe_init(&s);
        strsafe_init(&old);
        strsafe_init(&new);
        strsafe_set(&s, base);
        strsafe_set(&old, old_str);
        strsafe_set(&new, new_str);
        strsafe_replace_all(&s, &old, &new);
        fprintf(f, "%s,%s,%s,%s\n", base, old_str, new_str, s.data);
        strsafe_free(&s);
        strsafe_free(&old);
        strsafe_free(&new);
        free(base);
        free(old_str);
        free(new_str);
    }
}

// Test: strsafe_insert
void test_strsafe_insert(FILE* f) {
    log_header(f, "strsafe_insert");
    for (int i = 0; i < NUM_TESTS; ++i) {
        char* base = random_string(10);
        char* insert = random_string(5);
        size_t pos = rand() % (strlen(base) + 1);

        StrSafe s, ins;
        strsafe_init(&s);
        strsafe_init(&ins);
        strsafe_set(&s, base);
        strsafe_set(&ins, insert);

        strsafe_insert(&s, pos, &ins);
        fprintf(f, "%s,%s,%zu,%s\n", base, insert, pos, s.data);

        strsafe_free(&s);
        strsafe_free(&ins);
        free(base);
        free(insert);
    }
}

// Test: strsafe_remove_all
void test_strsafe_remove_all(FILE* f) {
    log_header(f, "strsafe_remove_all");
    for (int i = 0; i < NUM_TESTS; ++i) {
        char* remove = random_string(2);
        bool should_contain = i < NUM_TESTS / 2;
        char* base = generate_haystack(remove, should_contain);

        StrSafe s, rem;
        strsafe_init(&s);
        strsafe_init(&rem);
        strsafe_set(&s, base);
        strsafe_set(&rem, remove);

        strsafe_remove_all(&s, &rem);
        fprintf(f, "%s,%s,%s\n", base, remove, s.data);

        strsafe_free(&s);
        strsafe_free(&rem);
        free(base);
        free(remove);
    }
}

// Test: strsafe_count
void test_strsafe_count(FILE* f) {
    log_header(f, "strsafe_count");
    for (int i = 0; i < NUM_TESTS; ++i) {
        char* needle = random_string(2);
        bool should_contain = i < NUM_TESTS / 2;
        char* haystack = generate_haystack(needle, should_contain);

        StrSafe h, n;
        strsafe_init(&h);
        strsafe_init(&n);
        strsafe_set(&h, haystack);
        strsafe_set(&n, needle);

        size_t count = strsafe_count(&h, &n);
        fprintf(f, "%s,%s,%zu\n", haystack, needle, count);

        strsafe_free(&h);
        strsafe_free(&n);
        free(haystack);
        free(needle);
    }
}

// Test: strsafe_find
void test_strsafe_find(FILE* f) {
    log_header(f, "strsafe_find");
    for (int i = 0; i < NUM_TESTS; ++i) {
        char* needle = random_string(3);
        bool should_contain = i < NUM_TESTS / 2;
        char* haystack = generate_haystack(needle, should_contain);

        StrSafe h, n;
        strsafe_init(&h);
        strsafe_init(&n);
        strsafe_set(&h, haystack);
        strsafe_set(&n, needle);

        ssize_t pos = strsafe_find(&h, &n);
        fprintf(f, "%s,%s,%zd\n", haystack, needle, pos);

        strsafe_free(&h);
        strsafe_free(&n);
        free(haystack);
        free(needle);
    }
}

// Test: strsafe_find_from_pos
void test_strsafe_find_from_pos(FILE* f) {
    log_header(f, "strsafe_find_from_pos");
    for (int i = 0; i < NUM_TESTS; ++i) {
        char* needle = random_string(2);
        bool should_contain = i < NUM_TESTS / 2;
        char* haystack = generate_haystack(needle, should_contain);

        StrSafe h, n;
        strsafe_init(&h);
        strsafe_init(&n);
        strsafe_set(&h, haystack);
        strsafe_set(&n, needle);

        size_t pos_start = rand() % (strlen(haystack) + 1);
        ssize_t pos = strsafe_find_from_pos(&h, &n, pos_start);
        fprintf(f, "%s,%s,%zu,%zd\n", haystack, needle, pos_start, pos);

        strsafe_free(&h);
        strsafe_free(&n);
        free(haystack);
        free(needle);
    }
}

// Test: strsafe_array_free (used after split)
void test_strsafe_array_free(FILE* f) {
    log_header(f, "strsafe_array_free (via strsafe_split)");
    for (int i = 0; i < NUM_TESTS; ++i) {
        char* delim = random_string(1);
        char* base = generate_haystack(delim, true);

        StrSafe s, d;
        strsafe_init(&s);
        strsafe_init(&d);
        strsafe_set(&s, base);
        strsafe_set(&d, delim);

        StrSafe_array parts = strsafe_split(&s, &d);
        fprintf(f, "%s,%s,%d parts\n", base, delim, parts.array_size);
        for (int j = 0; j < parts.array_size; ++j) {
            fprintf(f, ",%s", parts.arr[j].data);
            strsafe_free(&parts.arr[j]);
        }
        fprintf(f, "\n");

        strsafe_array_free(&parts);
        strsafe_free(&s);
        strsafe_free(&d);
        free(base);
        free(delim);
    }
}

// Test: strsafe_trim
void test_strsafe_trim(FILE* f) {
    log_header(f, "strsafe_trim");
    for (int i = 0; i < NUM_TESTS; ++i) {
        char* base = malloc(MAX_LEN + 1);
        size_t pad = rand() % 5;
        memset(base, ' ', pad);
        strcpy(base + pad, random_string(10));
        strcat(base, "     ");

        StrSafe s;
        strsafe_init(&s);
        strsafe_set(&s, base);
        strsafe_trim(&s);
        fprintf(f, "\"%s\",\"%s\"\n", base, s.data);

        strsafe_free(&s);
        free(base);
    }
}

// Test: strsafe_ensure_capacity
void test_strsafe_ensure_capacity(FILE* f) {
    log_header(f, "strsafe_ensure_capacity");
    for (int i = 0; i < NUM_TESTS; ++i) {
        size_t cap = rand() % 128 + 64;
        StrSafe s;
        strsafe_init(&s);
        bool ok = strsafe_ensure_capacity(&s, cap);
        fprintf(f, "%zu,%s\n", cap, ok ? "true" : "false");
        strsafe_free(&s);
    }
}

// Test: strsafe_appendv
void test_strsafe_appendv(FILE* f) {
    log_header(f, "strsafe_appendv");
    for (int i = 0; i < NUM_TESTS; ++i) {
        StrSafe s, a, b, c;
        strsafe_init(&s);
        strsafe_init(&a);
        strsafe_init(&b);
        strsafe_init(&c);

        char* base = random_string(5);
        char* s1 = random_string(3);
        char* s2 = random_string(3);
        char* s3 = random_string(3);

        strsafe_set(&s, base);
        strsafe_set(&a, s1);
        strsafe_set(&b, s2);
        strsafe_set(&c, s3);

        strsafe_appendv(&s, &a, &b, &c, NULL);
        fprintf(f, "%s,%s,%s,%s,%s\n", base, s1, s2, s3, s.data);

        strsafe_free(&s);
        strsafe_free(&a);
        strsafe_free(&b);
        strsafe_free(&c);
        free(base);
        free(s1);
        free(s2);
        free(s3);
    }
}
// Test: strsafe_split
void test_strsafe_split(FILE* f) {
    log_header(f, "strsafe_split");
    for (int i = 0; i < NUM_TESTS; ++i) {
        char* delim = random_string(1);
        char* base = generate_haystack(delim, true);

        StrSafe s, d;
        strsafe_init(&s);
        strsafe_init(&d);
        strsafe_set(&s, base);
        strsafe_set(&d, delim);

        StrSafe_array parts = strsafe_split(&s, &d);
        fprintf(f, "%s,%s,%d parts", base, delim, parts.array_size);
        for (int j = 0; j < parts.array_size; ++j) {
            fprintf(f, ",%s", parts.arr[j].data);
            strsafe_free(&parts.arr[j]);
        }
        fprintf(f, "\n");

        strsafe_array_free(&parts);
        strsafe_free(&s);
        strsafe_free(&d);
        free(base);
        free(delim);
    }
}

// Test: cstr_replace
void test_cstr_replace(FILE* f) {
    log_header(f, "cstr_replace");
    for (int i = 0; i < NUM_TESTS; ++i) {
        char* old_str = random_string(2);
        char* new_str = random_string(2);
        bool should_contain = i < NUM_TESTS / 2;
        char* base = generate_haystack(old_str, should_contain);

        StrSafe s;
        strsafe_init(&s);
        strsafe_set(&s, base);

        cstr_replace(&s, old_str, new_str);
        fprintf(f, "%s,%s,%s,%s\n", base, old_str, new_str, s.data);

        strsafe_free(&s);
        free(base);
        free(old_str);
        free(new_str);
    }
}

// Test: cstr_replace_all
void test_cstr_replace_all(FILE* f) {
    log_header(f, "cstr_replace_all");
    for (int i = 0; i < NUM_TESTS; ++i) {
        char* old_str = random_string(2);
        char* new_str = random_string(2);
        bool should_contain = i < NUM_TESTS / 2;
        char* base = generate_haystack(old_str, should_contain);

        StrSafe s;
        strsafe_init(&s);
        strsafe_set(&s, base);

        cstr_replace_all(&s, old_str, new_str);
        fprintf(f, "%s,%s,%s,%s\n", base, old_str, new_str, s.data);

        strsafe_free(&s);
        free(base);
        free(old_str);
        free(new_str);
    }
}

// Test: cstr_find
void test_cstr_find(FILE* f) {
    log_header(f, "cstr_find");
    for (int i = 0; i < NUM_TESTS; ++i) {
        char* needle = random_string(3);
        bool should_contain = i < NUM_TESTS / 2;
        char* haystack = generate_haystack(needle, should_contain);

        StrSafe s;
        strsafe_init(&s);
        strsafe_set(&s, haystack);

        ssize_t pos = cstr_find(&s, needle);
        fprintf(f, "%s,%s,%zd\n", haystack, needle, pos);

        strsafe_free(&s);
        free(haystack);
        free(needle);
    }
}

// Test: cstr_find_from_pos
void test_cstr_find_from_pos(FILE* f) {
    log_header(f, "cstr_find_from_pos");
    for (int i = 0; i < NUM_TESTS; ++i) {
        char* needle = random_string(2);
        bool should_contain = i < NUM_TESTS / 2;
        char* haystack = generate_haystack(needle, should_contain);

        StrSafe s;
        strsafe_init(&s);
        strsafe_set(&s, haystack);

        size_t pos_start = rand() % (strlen(haystack) + 1);
        ssize_t pos = cstr_find_from_pos(&s, needle, pos_start);
        fprintf(f, "%s,%s,%zu,%zd\n", haystack, needle, pos_start, pos);

        strsafe_free(&s);
        free(haystack);
        free(needle);
    }
}

// Test: cstr_compare
void test_cstr_compare(FILE* f) {
    log_header(f, "cstr_compare");
    for (int i = 0; i < NUM_TESTS; ++i) {
        char* a_str = random_string(5);
        char* b_str = (i % 2 == 0) ? strdup(a_str) : random_string(5);

        StrSafe a;
        strsafe_init(&a);
        strsafe_set(&a, a_str);

        bool result = cstr_compare(&a, b_str);
        fprintf(f, "%s,%s,%s\n", a_str, b_str, result ? "true" : "false");

        strsafe_free(&a);
        free(a_str);
        free(b_str);
    }
}

// Test: cstr_count
void test_cstr_count(FILE* f) {
    log_header(f, "cstr_count");
    for (int i = 0; i < NUM_TESTS; ++i) {
        char* needle = random_string(2);
        bool should_contain = i < NUM_TESTS / 2;
        char* haystack = generate_haystack(needle, should_contain);

        StrSafe s;
        strsafe_init(&s);
        strsafe_set(&s, haystack);

        size_t count = cstr_count(&s, needle);
        fprintf(f, "%s,%s,%zu\n", haystack, needle, count);

        strsafe_free(&s);
        free(haystack);
        free(needle);
    }
}

// Test: cstr_remove
void test_cstr_remove(FILE* f) {
    log_header(f, "cstr_remove");
    for (int i = 0; i < NUM_TESTS; ++i) {
        char* remove = random_string(2);
        bool should_contain = i < NUM_TESTS / 2;
        char* base = generate_haystack(remove, should_contain);

        StrSafe s;
        strsafe_init(&s);
        strsafe_set(&s, base);

        cstr_remove(&s, remove);
        fprintf(f, "%s,%s,%s\n", base, remove, s.data);

        strsafe_free(&s);
        free(base);
        free(remove);
    }
}

// Test: cstr_remove_all
void test_cstr_remove_all(FILE* f) {
    log_header(f, "cstr_remove_all");
    for (int i = 0; i < NUM_TESTS; ++i) {
        char* remove = random_string(2);
        bool should_contain = i < NUM_TESTS / 2;
        char* base = generate_haystack(remove, should_contain);

        StrSafe s;
        strsafe_init(&s);
        strsafe_set(&s, base);

        cstr_remove_all(&s, remove);
        fprintf(f, "%s,%s,%s\n", base, remove, s.data);

        strsafe_free(&s);
        free(base);
        free(remove);
    }
}

// Test: cstr_append
void test_cstr_append(FILE* f) {
    log_header(f, "cstr_append");
    for (int i = 0; i < NUM_TESTS; ++i) {
        char* base = random_string(5);
        char* suffix = random_string(5);

        StrSafe s;
        strsafe_init(&s);
        strsafe_set(&s, base);

        cstr_append(&s, suffix);
        fprintf(f, "%s,%s,%s\n", base, suffix, s.data);

        strsafe_free(&s);
        free(base);
        free(suffix);
    }
}

// Test: cstr_appendv
void test_cstr_appendv(FILE* f) {
    log_header(f, "cstr_appendv");
    for (int i = 0; i < NUM_TESTS; ++i) {
        char* base = random_string(5);
        char* s1 = random_string(3);
        char* s2 = random_string(3);
        char* s3 = random_string(3);

        StrSafe s;
        strsafe_init(&s);
        strsafe_set(&s, base);

        cstr_appendv(&s, s1, s2, s3, NULL);
        fprintf(f, "%s,%s,%s,%s,%s\n", base, s1, s2, s3, s.data);

        strsafe_free(&s);
        free(base);
        free(s1);
        free(s2);
        free(s3);
    }
}

// Test: cstr_split
void test_cstr_split(FILE* f) {
    log_header(f, "cstr_split");
    for (int i = 0; i < NUM_TESTS; ++i) {
        char* delim = random_string(1);
        char* base = generate_haystack(delim, true);

        StrSafe s;
        strsafe_init(&s);
        strsafe_set(&s, base);

        StrSafe_array parts = cstr_split(&s, delim);
        fprintf(f, "%s,%s,%d parts", base, delim, parts.array_size);
        for (int j = 0; j < parts.array_size; ++j) {
            fprintf(f, ",%s", parts.arr[j].data);
            strsafe_free(&parts.arr[j]);
        }
        fprintf(f, "\n");

        free(parts.arr);
        strsafe_free(&s);
        free(base);
        free(delim);
    }
}
// Main
int main() {
    srand((unsigned int)time(NULL));
    FILE* f = fopen("test_results.txt", "w");
    if (!f) {
        perror("Failed to open test_results.txt");
        return 1;
    }
	struct timespec start, end;
	long long duration;

    // StrSafe-based tests
	clock_gettime(CLOCK_MONOTONIC, &start);
    test_strsafe_set(f);
	clock_gettime(CLOCK_MONOTONIC, &end);
	duration = get_nanoseconds(start, end);
	log_header_time(f, "test_strsafe_set", duration);
	clock_gettime(CLOCK_MONOTONIC, &start);
    test_strsafe_compare(f);
	clock_gettime(CLOCK_MONOTONIC, &end);
	duration = get_nanoseconds(start, end);
	log_header_time(f, "test_strsafe_compare", duration);
	clock_gettime(CLOCK_MONOTONIC, &start);
    test_strsafe_copy(f);
	clock_gettime(CLOCK_MONOTONIC, &end);
	duration = get_nanoseconds(start, end);
	log_header_time(f, "test_strsafe_copy", duration);
	clock_gettime(CLOCK_MONOTONIC, &start);
    test_strsafe_append(f);
	clock_gettime(CLOCK_MONOTONIC, &end);
	duration = get_nanoseconds(start, end);
	log_header_time(f, "test_strsafe_append", duration);
	clock_gettime(CLOCK_MONOTONIC, &start);
    test_strsafe_appendv(f);
	clock_gettime(CLOCK_MONOTONIC, &end);
	duration = get_nanoseconds(start, end);
	log_header_time(f, "test_strsafe_appendv", duration);
	clock_gettime(CLOCK_MONOTONIC, &start);
    test_strsafe_insert(f);
	clock_gettime(CLOCK_MONOTONIC, &end);
	duration = get_nanoseconds(start, end);
	log_header_time(f, "test_strsafe_insert", duration);
	clock_gettime(CLOCK_MONOTONIC, &start);
    test_strsafe_substr(f);
	clock_gettime(CLOCK_MONOTONIC, &end);
	duration = get_nanoseconds(start, end);
	log_header_time(f, "test_strsafe_substr", duration);
	clock_gettime(CLOCK_MONOTONIC, &start);
    test_strsafe_replace_all(f);
	clock_gettime(CLOCK_MONOTONIC, &end);
	duration = get_nanoseconds(start, end);
	log_header_time(f, "test_strsafe_replace_all", duration);
	clock_gettime(CLOCK_MONOTONIC, &start);
    test_strsafe_remove_all(f);
	clock_gettime(CLOCK_MONOTONIC, &end);
	duration = get_nanoseconds(start, end);
	log_header_time(f, "test_strsafe_remove_all", duration);
	clock_gettime(CLOCK_MONOTONIC, &start);
    test_strsafe_count(f);
	clock_gettime(CLOCK_MONOTONIC, &end);
	duration = get_nanoseconds(start, end);
	log_header_time(f, "test_strsafe_count", duration);
	clock_gettime(CLOCK_MONOTONIC, &start);
    test_strsafe_find(f);
	clock_gettime(CLOCK_MONOTONIC, &end);
	duration = get_nanoseconds(start, end);
	log_header_time(f, "test_strsafe_find", duration);
	clock_gettime(CLOCK_MONOTONIC, &start);
    test_strsafe_find_from_pos(f);
	clock_gettime(CLOCK_MONOTONIC, &end);
	duration = get_nanoseconds(start, end);
	log_header_time(f, "test_strsafe_find_from_pos", duration);
	clock_gettime(CLOCK_MONOTONIC, &start);
    test_strsafe_split(f);
	clock_gettime(CLOCK_MONOTONIC, &end);
	duration = get_nanoseconds(start, end);
	log_header_time(f, "test_strsafe_split", duration);
	clock_gettime(CLOCK_MONOTONIC, &start);
    test_strsafe_array_free(f);
	clock_gettime(CLOCK_MONOTONIC, &end);
	duration = get_nanoseconds(start, end);
	log_header_time(f, "test_strsafe_array_free", duration);
	clock_gettime(CLOCK_MONOTONIC, &start);
    test_strsafe_trim(f);
	clock_gettime(CLOCK_MONOTONIC, &end);
	duration = get_nanoseconds(start, end);
	log_header_time(f, "test_strsafe_trim", duration);
	clock_gettime(CLOCK_MONOTONIC, &start);
	test_strsafe_ensure_capacity(f);
	clock_gettime(CLOCK_MONOTONIC, &end);
	duration = get_nanoseconds(start, end);
	log_header_time(f, "test_strsafe_ensure_capacity", duration);

    // C-string based tests
    
	clock_gettime(CLOCK_MONOTONIC, &start);
	test_cstr_replace(f);
	clock_gettime(CLOCK_MONOTONIC, &end);
	duration = get_nanoseconds(start, end);
	log_header_time(f, "test_cstr_replace", duration);    
	clock_gettime(CLOCK_MONOTONIC, &start);
	test_cstr_replace_all(f);
	clock_gettime(CLOCK_MONOTONIC, &end);
	duration = get_nanoseconds(start, end);
	log_header_time(f, "test_cstr_replace_all", duration);    
	clock_gettime(CLOCK_MONOTONIC, &start);
	test_cstr_find(f);
	clock_gettime(CLOCK_MONOTONIC, &end);
	duration = get_nanoseconds(start, end);
	log_header_time(f, "test_cstr_find", duration);    
	clock_gettime(CLOCK_MONOTONIC, &start);
	test_cstr_find_from_pos(f);
	clock_gettime(CLOCK_MONOTONIC, &end);
	duration = get_nanoseconds(start, end);
	log_header_time(f, "test_cstr_find_from_pos", duration);    
	clock_gettime(CLOCK_MONOTONIC, &start);
	test_cstr_compare(f);
	clock_gettime(CLOCK_MONOTONIC, &end);
	duration = get_nanoseconds(start, end);
	log_header_time(f, "test_cstr_compare", duration);    
	clock_gettime(CLOCK_MONOTONIC, &start);
	test_cstr_count(f);
	clock_gettime(CLOCK_MONOTONIC, &end);
	duration = get_nanoseconds(start, end);
	log_header_time(f, "test_cstr_count", duration);    
	clock_gettime(CLOCK_MONOTONIC, &start);
	test_cstr_remove(f);
	clock_gettime(CLOCK_MONOTONIC, &end);
	duration = get_nanoseconds(start, end);
	log_header_time(f, "test_cstr_remove", duration);    
	clock_gettime(CLOCK_MONOTONIC, &start);
	test_cstr_remove_all(f);
	clock_gettime(CLOCK_MONOTONIC, &end);
	duration = get_nanoseconds(start, end);
	log_header_time(f, "test_cstr_remove_all", duration);    
	clock_gettime(CLOCK_MONOTONIC, &start);
	test_cstr_append(f);
	clock_gettime(CLOCK_MONOTONIC, &end);
	duration = get_nanoseconds(start, end);
	log_header_time(f, "test_cstr_append", duration);    
	clock_gettime(CLOCK_MONOTONIC, &start);
	test_cstr_appendv(f);
	clock_gettime(CLOCK_MONOTONIC, &end);
	duration = get_nanoseconds(start, end);
	log_header_time(f, "test_cstr_appendv", duration);    
	clock_gettime(CLOCK_MONOTONIC, &start);
	test_cstr_split(f);
	clock_gettime(CLOCK_MONOTONIC, &end);
	duration = get_nanoseconds(start, end);
	log_header_time(f, "test_cstr_split", duration);
    fclose(f);
    return 0;
}
