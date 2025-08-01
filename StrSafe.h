/* safe_str.h - Header-only minimal safe ASCII string library for C11 */
#ifndef SAFE_STR_H
#define SAFE_STR_H

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define STR_MAX_SIZE SIZE_MAX
#define STR_DEFAULT_CAPACITY 64

typedef struct {
    char* data;
    size_t length;
    size_t capacity;
} StrSafe;

static inline int str_init(StrSafe* str, size_t capacity) {
    if (!str || capacity == 0) return -1;
    str->data = (char*)malloc(capacity);
    if (!str->data) return -1;
    str->data[0] = '\0';
    str->length = 0;
    str->capacity = capacity;
    return 0;
}

static inline int str_init_default(StrSafe* str) {
    return str_init(str, STR_DEFAULT_CAPACITY);
}

static inline int str_ensure_capacity(StrSafe* str, size_t required_capacity) {
    if (!str) return -1;
    if (required_capacity + 1 <= str->capacity) return 0;
    size_t new_capacity = required_capacity + 1;
    char* new_data = (char*)realloc(str->data, new_capacity);
    if (!new_data) return -1;
    str->data = new_data;
    str->capacity = new_capacity;
    return 0;
}

static inline int str_fit(StrSafe* str) {
    if (!str || !str->data) return -1;
    size_t fit_size = str->length + 1;
    char* shrunk = (char*)realloc(str->data, fit_size);
    if (!shrunk) return -1;
    str->data = shrunk;
    str->capacity = fit_size;
    return 0;
}

static inline void str_free(StrSafe* str) {
    if (!str) return;
    if (str->data) free(str->data);
    str->data = NULL;
    str->length = 0;
    str->capacity = 0;
}

static inline int str_set(StrSafe* destination_string, const char* string_to_set) {
    if (!destination_string || !string_to_set) return -1;
    size_t len = strlen(string_to_set);
    if (str_ensure_capacity(destination_string, len) != 0) return -1;
    memcpy(destination_string->data, string_to_set, len + 1);
    destination_string->length = len;
    return 0;
}

static inline int str_copy(StrSafe* destination_string, const StrSafe* source_string) {
    if (!destination_string || !source_string || !source_string->data) return -1;
    if (str_ensure_capacity(destination_string, source_string->length) != 0) return -1;
    memcpy(destination_string->data, source_string->data, source_string->length + 1);
    destination_string->length = source_string->length;
    return 0;
}

static inline int str_copy_cstr(StrSafe* destination_string, const char* source_string) {
    return str_set(destination_string, source_string);
}

static inline int str_append(StrSafe* appended_string, const char* data_to_append) {
    if (!appended_string || !data_to_append) return -1;
    size_t clen = strlen(data_to_append);
    if (str_ensure_capacity(appended_string, appended_string->length + clen) != 0) return -1;
    memcpy(appended_string->data + appended_string->length, data_to_append, clen + 1);
    appended_string->length += clen;
    return 0;
}

static inline int str_append_str(StrSafe* appended_string, const StrSafe* data_to_append) {
    if (!appended_string || !data_to_append || !data_to_append->data) return -1;
    if (str_ensure_capacity(appended_string, appended_string->length + data_to_append->length) != 0) return -1;
    memcpy(appended_string->data + appended_string->length, data_to_append->data, data_to_append->length + 1);
    appended_string->length += data_to_append->length;
    return 0;
}

static inline int str_cmp(const StrSafe* first_string, const StrSafe* second_string) {
    if (!first_string || !second_string) return INT32_MIN;
    return strcmp(first_string->data, second_string->data);
}

static inline size_t str_find(const StrSafe* string_to_be_searched, const char* searched_data) {
    if (!string_to_be_searched || !searched_data || !string_to_be_searched->data) return STR_MAX_SIZE;
    char* pos = strstr(string_to_be_searched->data, searched_data);
    if (!pos) return STR_MAX_SIZE;
    return (size_t)(pos - string_to_be_searched->data);
}

static inline size_t str_find_str(const StrSafe* string_to_be_searched, const StrSafe* searched_data) {
    if (!string_to_be_searched || !searched_data || !searched_data->data) return STR_MAX_SIZE;
    return str_find(string_to_be_searched, searched_data->data);
}

static inline size_t str_count(const StrSafe* source_string, const char* what_is_counted) {
    if (!source_string || !what_is_counted || !*what_is_counted || !source_string->data) return 0;
    size_t count = 0;
    const char* p = source_string->data;
    size_t nlen = strlen(what_is_counted);
    while (p = strstr(p, what_is_counted)) {
        ++count;
        p += nlen;
    }
    return count;
}

static inline size_t str_count_str(const StrSafe* source_string, const StrSafe* what_is_counted) {
    if (!source_string || !what_is_counted || !what_is_counted->data) return 0;
    size_t count = 0;
    const char* p = source_string->data;
    while (p = strstr(source_string->data, what_is_counted->data)) {
        ++count;
        p += what_is_counted->length;
    }
    return count;
}

static inline int str_substr(StrSafe* destination_string, const StrSafe* source_string, size_t pos, size_t len) {
    if (!destination_string || !source_string || pos >= source_string->length) return -1;
    if (pos + len > source_string->length) len = source_string->length - pos;
    if (str_ensure_capacity(destination_string, len) != 0) return -1;
    memcpy(destination_string->data, source_string->data + pos, len);
    destination_string->data[len] = '\0';
    destination_string->length = len;
    return 0;
}

static inline bool is_space(char ch) {
    return ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r';
}

static inline void str_trim(StrSafe* str) {
    if (!str || !str->data || str->length == 0) return;

    size_t start = 0;
    while (start < str->length && is_space(str->data[start])) ++start;

    size_t end = str->length;
    while (end > start && is_space(str->data[end - 1])) --end;

    size_t new_len = end - start;
    if (start > 0 && new_len > 0)
        memmove(str->data, str->data + start, new_len);

    str->data[new_len] = '\0';
    str->length = new_len;
    str_fit(str);
}

static inline int str_replace(StrSafe* string, const char* target, const char* replacement) {
    if (!string || !target || !replacement || !string->data) return -1;

    size_t tlen = strlen(target);
    size_t rlen = strlen(replacement);

    if (tlen == 0 || tlen > string->length) return 0;

    char* p = string->data;
    size_t occurrences = 0;
    while ((p = strstr(p, target))) {
        ++occurrences;
        p += tlen;
    }

    if (occurrences == 0) return 0;

    size_t new_len = string->length + occurrences * (rlen - tlen);
    if (str_ensure_capacity(string, new_len) != 0) return -1;

    char* dst = string->data;
    char* src = string->data;
    char* temp = (char*)malloc(new_len + 1);
    if (!temp) return -1;
    char* out = temp;

    while ((p = strstr(src, target))) {
        size_t seg_len = p - src;
        memcpy(out, src, seg_len);
        out += seg_len;
        memcpy(out, replacement, rlen);
        out += rlen;
        src = p + tlen;
    }
    strcpy(out, src);

    memcpy(dst, temp, new_len + 1);
    string->length = new_len;
    free(temp);
    return 0;
}

static inline int str_replace_str(StrSafe* str, const StrSafe* target, const StrSafe* replacement) {
    if (!str || !target || !replacement || !target->data || !replacement->data) return -1;

    size_t tlen = target->length;
    size_t rlen = replacement->length;

    if (tlen == 0 || tlen > str->length) return 0;

    char* p = str->data;
    size_t occurrences = 0;
    while ((p = strstr(p, target->data))) {
        ++occurrences;
        p += tlen;
    }

    if (occurrences == 0) return 0;

    size_t new_len = str->length + occurrences * (rlen - tlen);
    if (str_ensure_capacity(str, new_len) != 0) return -1;

    char* src = str->data;
    char* temp = (char*)malloc(new_len + 1);
    if (!temp) return -1;
    char* out = temp;

    while ((p = strstr(src, target->data))) {
        size_t seg_len = p - src;
        memcpy(out, src, seg_len);
        out += seg_len;
        memcpy(out, replacement->data, rlen);
        out += rlen;
        src = p + tlen;
    }
    strcpy(out, src);

    memcpy(str->data, temp, new_len + 1);
    str->length = new_len;
    free(temp);
    return 0;
}

static inline int str_remove(StrSafe* str, const char* substr) {
    if (!str || !substr || !*substr || !str->data) return -1;

    size_t slen = strlen(substr);
    if (slen == 0 || slen > str->length) return 0;

    char* src = str->data;
    char* temp = (char*)malloc(str->length + 1);
    if (!temp) return -1;
    char* out = temp;
    char* p;

    while ((p = strstr(src, substr))) {
        size_t seg_len = p - src;
        memcpy(out, src, seg_len);
        out += seg_len;
        src = p + slen;
    }
    strcpy(out, src);

    size_t new_len = strlen(temp);
    memcpy(str->data, temp, new_len + 1);
    str->length = new_len;
    free(temp);
    return 0;
}

static inline int str_remove_str(StrSafe* str, const StrSafe* substr) {
    if (!str || !substr || !substr->data || substr->length == 0 || substr->length > str->length) return -1;

    char* src = str->data;
    char* temp = (char*)malloc(str->length + 1);
    if (!temp) return -1;
    char* out = temp;
    char* p;

    while ((p = strstr(src, substr->data))) {
        size_t seg_len = p - src;
        memcpy(out, src, seg_len);
        out += seg_len;
        src = p + substr->length;
    }
    strcpy(out, src);

    size_t new_len = strlen(temp);
    memcpy(str->data, temp, new_len + 1);
    str->length = new_len;
    free(temp);
    return 0;
}

#endif // SAFE_STR_H
