
/**
 * @file StrSafe.h
 * @brief Safe and efficient string manipulation utilities for dynamic strings in C23.
 *
 * This library provides a set of inline functions for managing dynamic strings (`StrSafe`)
 * and arrays of strings (`StrSafe_array`). It includes operations for initialization,
 * memory management, searching, replacing, appending, splitting, and more.
 *
 * All functions ensure proper memory handling and avoid unnecessary reallocations.
 * In all functions the first argument is StrSafe
 * `strsafe_` functions have the rest of the arguments, StrSafe (if the argument is string)
 * `cstr_` functions have the rest of the arguments char* (if the argument is string)
 *
 * @example
 * @code
 * #include "StrSafe.h"
 *
 * int main(void) {
 *     StrSafe s;
 *     strsafe_init(&s);
 *     strsafe_set(&s, "Hello, World!");
 *
 *     StrSafe suffix;
 *     strsafe_init(&suffix);
 *     strsafe_set(&suffix, " Goodbye!");
 *
 *     strsafe_append(&s, &suffix);
 *     // s now contains "Hello, World! Goodbye!"
 *
 *     StrSafe_array parts = strsafe_split(&s, &suffix);
 *     // parts.arr[0] contains "Hello, World!"
 *
 *     strsafe_array_free(&parts);
 *     strsafe_free(&s);
 *     strsafe_free(&suffix);
 *     return 0;
 * }
 * @endcode
 */

#ifndef SAFE_STR_H
#define SAFE_STR_H

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>

/**
 * @struct StrSafe
 * @brief Represents a dynamically allocated string with length and capacity tracking.
 */
typedef struct {
	char* data;   /**< Pointer to the string data (null-terminated). */
	size_t len;   /**< Length of the string (excluding null terminator). */
	size_t cap;   /**< Capacity of the allocated buffer (including null terminator). */
} StrSafe;

/**
 * @struct StrSafe_array
 * @brief Represents an array of `StrSafe` strings.
 */
typedef struct {
	StrSafe* arr;     /**< Pointer to array of `StrSafe` strings. */
	int array_size;   /**< Number of elements in the array. */
} StrSafe_array;

/**
 * @brief Initializes a `StrSafe` string to empty.
 * @param strsafe Pointer to the `StrSafe` to initialize.
 */
static inline void strsafe_init(StrSafe* strsafe) {
	strsafe->data = NULL;
	strsafe->len = 0;
	strsafe->cap = 0;
}

/**
 * @brief Frees memory used by a `StrSafe` string.
 * @param strsafe Pointer to the `StrSafe` to free.
 */
static inline void strsafe_free(StrSafe* strsafe) {
	free(strsafe->data);
	strsafe->data = NULL;
	strsafe->len = 0;
	strsafe->cap = 0;
}

/**
 * @brief Ensures the string has at least `min_cap` capacity.
 * @param strsafe Pointer to the string.
 * @param min_cap Minimum required capacity.
 * @return `true` if successful, `false` if allocation failed.
 */
static inline bool strsafe_ensure_capacity(StrSafe* strsafe, size_t min_cap) {
	if (strsafe->cap >= min_cap) return true;
	char* new_data = realloc(strsafe->data, min_cap);
	if (!new_data) return false;
	strsafe->data = new_data;
	strsafe->cap = min_cap;
	return true;
}

/**
 * @brief Trims the capacity of the string to fit its current length.
 * @param strsafe Pointer to the string.
 */
static inline void strsafe_trim(StrSafe* strsafe) {
	if (strsafe->len == 0) {
		free(strsafe->data);
		strsafe->data = NULL;
		strsafe->cap = 0;
		return;
	}
	char* trimmed = realloc(strsafe->data, strsafe->len + 1);
	if (trimmed) {
		strsafe->data = trimmed;
		strsafe->cap = strsafe->len + 1;
	}
}

/**
 * @brief Frees all strings in a `StrSafe_array` and the array itself.
 * @param strsafe_array Pointer to the array.
 */
static inline void strsafe_array_free(StrSafe_array* strsafe_array) {
	for (int i = 0; i < strsafe_array->array_size; ++i) {
		strsafe_free(&strsafe_array->arr[i]);
	}
	free(strsafe_array->arr);
	strsafe_array->arr = NULL;
	strsafe_array->array_size = 0;
}

/**
 * @brief Replaces the first occurrence of a substring with another in a `StrSafe` string.
 *
 * This function allocates a new buffer sized exactly for the result and replaces
 * the first match of `old_str` with `new_str`. Memory is freed before exiting.
 *
 * @param dst The target string to modify.
 * @param old_str The substring to be replaced.
 * @param new_str The replacement string.
 * @return Pointer to `dst`, or `NULL` on allocation failure.
 */
static inline StrSafe* cstr_replace(StrSafe* dst, const char* old_str, const char* new_str) {
	ssize_t pos = cstr_find(dst, old_str);
	if (pos < 0) return dst;

	size_t old_len = strlen(old_str);
	size_t new_len = strlen(new_str);
	size_t final_len = dst->len - old_len + new_len;

	char* buffer = malloc(final_len + 1);
	if (!buffer) return NULL;

	memcpy(buffer, dst->data, pos);
	memcpy(buffer + pos, new_str, new_len);
	memcpy(buffer + pos + new_len, dst->data + pos + old_len, dst->len - pos - old_len);
	buffer[final_len] = '\0';

	free(dst->data);
	dst->data = buffer;
	dst->len = final_len;
	dst->cap = final_len + 1;
	return dst;
}

/**
 * @brief Replaces all occurrences of a substring with another in a `StrSafe` string.
 *
 * This function precalculates the required capacity and performs a single allocation
 * for the final result. All matches of `old_str` are replaced with `new_str`.
 *
 * @param dst The target string to modify.
 * @param old_str The substring to be replaced.
 * @param new_str The replacement string.
 * @return Pointer to `dst`, or `NULL` on allocation failure.
 */
static inline StrSafe* cstr_replace_all(StrSafe* dst, const char* old_str, const char* new_str) {
	size_t count = cstr_count(dst, old_str);
	if (count == 0) return dst;

	size_t old_len = strlen(old_str);
	size_t new_len = strlen(new_str);
	size_t final_len = dst->len + count * (new_len - old_len);

	char* buffer = malloc(final_len + 1);
	if (!buffer) return NULL;

	char* src = dst->data;
	char* out = buffer;
	while (*src) {
		if (strncmp(src, old_str, old_len) == 0) {
			memcpy(out, new_str, new_len);
			out += new_len;
			src += old_len;
		}
		else {
			*out++ = *src++;
		}
	}
	*out = '\0';

	free(dst->data);
	dst->data = buffer;
	dst->len = final_len;
	dst->cap = final_len + 1;
	return dst;
}

/**
 * @brief Finds the first occurrence of a substring in a `StrSafe` string.
 *
 * @param haystack The string to search.
 * @param needle The substring to find.
 * @return Position of the first match, or -1 if not found.
 */
static inline ssize_t cstr_find(StrSafe* haystack, const char* needle) {
	char* pos = strstr(haystack->data, needle);
	return pos ? (ssize_t)(pos - haystack->data) : -1;
}

/**
 * @brief Finds the first occurrence of a substring starting from a given position.
 *
 * @param haystack The string to search.
 * @param needle The substring to find.
 * @param pos The position to start searching from.
 * @return Position of the first match, or -1 if not found.
 */
static inline ssize_t cstr_find_from_pos(StrSafe* haystack, const char* needle, size_t pos) {
	if (pos >= haystack->len) return -1;
	char* start = haystack->data + pos;
	char* found = strstr(start, needle);
	return found ? (ssize_t)(found - haystack->data) : -1;
}

/**
 * @brief Compares a `StrSafe` string with a C-string for equality.
 *
 * @param a The `StrSafe` string.
 * @param b The C-string.
 * @return `true` if the strings are equal, `false` otherwise.
 */
static inline bool cstr_compare(const StrSafe* a, const char* b) {
	return strcmp(a->data, b) == 0;
}

/**
 * @brief Counts the number of times a substring appears in a `StrSafe` string.
 *
 * @param haystack The string to search.
 * @param needle The substring to count.
 * @return Number of occurrences found.
 */
static inline size_t cstr_count(const StrSafe* haystack, const char* needle) {
	size_t count = 0;
	const char* p = haystack->data;
	size_t needle_len = strlen(needle);
	while ((p = strstr(p, needle))) {
		++count;
		p += needle_len;
	}
	return count;
}

/**
 * @brief Removes the first occurrence of a substring from a `StrSafe` string.
 *
 * This function shifts the remaining content and trims the capacity before exiting.
 *
 * @param dst The target string to modify.
 * @param str_to_remove The substring to remove.
 * @return Pointer to `dst`.
 */
static inline StrSafe* cstr_remove(StrSafe* dst, const char* str_to_remove) {
	ssize_t pos = cstr_find(dst, str_to_remove);
	if (pos < 0) return dst;

	size_t rem_len = strlen(str_to_remove);
	memmove(dst->data + pos, dst->data + pos + rem_len, dst->len - pos - rem_len + 1);
	dst->len -= rem_len;
	strsafe_trim(dst);
	return dst;
}

/**
 * @brief Removes all occurrences of a substring from a `StrSafe` string.
 *
 * This function performs in-place removal and trims the capacity before exiting.
 *
 * @param dst The target string to modify.
 * @param str_to_remove The substring to remove.
 * @return Pointer to `dst`.
 */
static inline StrSafe* cstr_remove_all(StrSafe* dst, const char* str_to_remove) {
	size_t rem_len = strlen(str_to_remove);
	char* src = dst->data;
	char* dst_ptr = dst->data;
	while (*src) {
		if (strncmp(src, str_to_remove, rem_len) == 0) {
			src += rem_len;
		}
		else {
			*dst_ptr++ = *src++;
		}
	}
	*dst_ptr = '\0';
	dst->len = dst_ptr - dst->data;
	strsafe_trim(dst);
	return dst;
}

/**
 * @brief Appends a C-string to a `StrSafe` string.
 *
 * Ensures capacity and performs a single allocation if needed.
 *
 * @param dst The target string to append to.
 * @param suffix The C-string to append.
 * @return Pointer to `dst`, or `NULL` on allocation failure.
 */
static inline StrSafe* cstr_append(StrSafe* dst, const char* suffix) {
	size_t suffix_len = strlen(suffix);
	size_t new_len = dst->len + suffix_len;
	if (!strsafe_ensure_capacity(dst, new_len + 1)) return NULL;
	memcpy(dst->data + dst->len, suffix, suffix_len + 1);
	dst->len = new_len;
	return dst;
}

/**
 * @brief Appends multiple C-strings to a `StrSafe` string.
 *
 * Precalculates total length and performs a single allocation.
 * The list must be terminated with a `NULL` argument.
 *
 * @param dst The target string to append to.
 * @param suffix The first C-string to append.
 * @param ... Additional C-strings, terminated by `NULL`.
 * @return Pointer to `dst`, or `NULL` on allocation failure.
 */
static inline StrSafe* cstr_appendv(StrSafe* dst, const char* suffix, ...) {
	va_list args;
	va_start(args, suffix);
	size_t total_len = strlen(suffix);
	const char* s;
	while ((s = va_arg(args, const char*))) {
		total_len += strlen(s);
	}
	va_end(args);

	if (!strsafe_ensure_capacity(dst, dst->len + total_len + 1)) return NULL;

	va_start(args, suffix);
	memcpy(dst->data + dst->len, suffix, strlen(suffix));
	dst->len += strlen(suffix);
	while ((s = va_arg(args, const char*))) {
		size_t l = strlen(s);
		memcpy(dst->data + dst->len, s, l);
		dst->len += l;
	}
	dst->data[dst->len] = '\0';
	va_end(args);
	return dst;
}

/**
 * @brief Splits a `StrSafe` string into an array of substrings using a C-string delimiter.
 *
 * Allocates a new array and copies each segment into a separate `StrSafe`.
 * Caller must free the result using `strsafe_array_free`.
 *
 * @param src The source string to split.
 * @param delim The delimiter string.
 * @return A `StrSafe_array` containing the split substrings.
 */
static inline StrSafe_array cstr_split(StrSafe* src, const char* delim) {
	StrSafe_array result = { 0 };
	size_t delim_len = strlen(delim);
	char* start = src->data;
	char* end;

	while ((end = strstr(start, delim))) {
		size_t seg_len = end - start;
		result.arr = realloc(result.arr, sizeof(StrSafe) * (result.array_size + 1));
		StrSafe* seg = &result.arr[result.array_size++];
		seg->data = malloc(seg_len + 1);
		memcpy(seg->data, start, seg_len);
		seg->data[seg_len] = '\0';
		seg->len = seg_len;
		seg->cap = seg_len + 1;
		start = end + delim_len;
	}

	size_t rem_len = strlen(start);
	result.arr = realloc(result.arr, sizeof(StrSafe) * (result.array_size + 1));
	StrSafe* last = &result.arr[result.array_size++];
	last->data = malloc(rem_len + 1);
	memcpy(last->data, start, rem_len + 1);
	last->len = rem_len;
	last->cap = rem_len + 1;

	return result;
}

/**
 * @brief Sets the content of a `StrSafe` from a C-string.
 * @param dst Destination string.
 * @param src Source C-string.
 * @return Pointer to `dst`, or `NULL` on failure.
 */
static inline StrSafe* strsafe_set(StrSafe* dst, const char* src) {
	size_t src_len = strlen(src);
	if (!strsafe_ensure_capacity(dst, src_len + 1)) return NULL;

	memcpy(dst->data, src, src_len + 1);
	dst->len = src_len;
	return dst;
}

/**
 * @brief Finds the first occurrence of `needle` in `haystack`.
 * @param haystack The string to search.
 * @param needle The substring to find.
 * @return Position of match or -1 if not found.
 */
static inline ssize_t strsafe_find(StrSafe* haystack, const StrSafe* needle) {
	char* pos = strstr(haystack->data, needle->data);
	return pos ? (ssize_t)(pos - haystack->data) : -1;
}

/**
 * @brief Finds the first occurrence of `needle` in `haystack` starting from `pos`.
 * @param haystack The string to search.
 * @param needle The substring to find.
 * @param pos Starting position.
 * @return Position of match or -1 if not found.
 */
static inline ssize_t strsafe_find_from_pos(StrSafe* haystack, const StrSafe* needle, size_t pos) {
	if (pos >= haystack->len) return -1;
	char* start = haystack->data + pos;
	char* found = strstr(start, needle->data);
	return found ? (ssize_t)(found - haystack->data) : -1;
}

/**
 * @brief Compares two `StrSafe` strings for equality.
 * @param a First string.
 * @param b Second string.
 * @return `true` if equal, `false` otherwise.
 */
static inline bool strsafe_compare(const StrSafe* a, const StrSafe* b) {
	if (a->len != b->len) return false;
	return memcmp(a->data, b->data, a->len) == 0;
}

/**
 * @brief Copies the content of one `StrSafe` to another.
 * @param dst Destination string.
 * @param src Source string.
 * @return `true` if successful, `false` on allocation failure.
 */
static inline bool strsafe_copy(StrSafe* dst, const StrSafe* src) {
	if (!strsafe_ensure_capacity(dst, src->len + 1)) return false;
	memcpy(dst->data, src->data, src->len + 1);
	dst->len = src->len;
	return true;
}

/**
 * @brief Counts how many times `needle` appears in `dst`.
 * @param haystack The string to search.
 * @param needle The substring to count.
 * @return Number of occurrences.
 */
static inline size_t strsafe_count(const StrSafe* haystack, const StrSafe* needle) {
	size_t count = 0;
	const char* p = haystack->data;
	while ((p = strstr(p, needle->data))) {
		++count;
		p += needle->len;
	}
	return count;
}

/**
 * @brief Inserts a string into another at a given position.
 * @param dst Destination string.
 * @param pos Position to insert at.
 * @param ins String to insert.
 * @return `true` if successful, `false` otherwise.
 */
static inline bool strsafe_insert(StrSafe* dst, size_t pos, const StrSafe* ins) {
	if (pos > dst->len) return false;

	size_t new_len = dst->len + ins->len;
	if (!strsafe_ensure_capacity(dst, new_len + 1)) return false;

	memmove(dst->data + pos + ins->len, dst->data + pos, dst->len - pos + 1);
	memcpy(dst->data + pos, ins->data, ins->len);
	dst->len = new_len;
	return true;
}

/**
 * @brief Extracts a substring from a `StrSafe`.
 * @param sub_string Target string to hold the substring.
 * @param pos Starting position.
 * @param len Length of substring.
 * @return Pointer to `sub_string`, or `NULL` on failure.
 */
static inline StrSafe* strsafe_substr(StrSafe* sub_string, const size_t pos, const size_t len) {
	if (pos >= sub_string->len) {
		strsafe_free(sub_string);
		strsafe_init(sub_string);
		return sub_string;
	}

	size_t actual_len = (pos + len > sub_string->len) ? (sub_string->len - pos) : len;
	char* buffer = malloc(actual_len + 1);
	if (!buffer) return NULL;

	memcpy(buffer, sub_string->data + pos, actual_len);
	buffer[actual_len] = '\0';

	strsafe_free(sub_string);
	sub_string->data = buffer;
	sub_string->len = actual_len;
	sub_string->cap = actual_len + 1;
	return sub_string;
}

/**
 * @brief Replaces the first occurrence of a substring with another.
 * @param dst Target string.
 * @param old_str Substring to replace.
 * @param new_str Replacement string.
 * @return `true` if successful, `false` otherwise.
 */
static inline bool strsafe_replace(StrSafe* dst, const StrSafe* old_str, const StrSafe* new_str) {
	ssize_t pos = cstr_find(dst, old_str->data);
	if (pos < 0) return true;

	size_t final_len = dst->len - old_str->len + new_str->len;
	char* buffer = malloc(final_len + 1);
	if (!buffer) return false;

	memcpy(buffer, dst->data, pos);
	memcpy(buffer + pos, new_str->data, new_str->len);
	memcpy(buffer + pos + new_str->len, dst->data + pos + old_str->len, dst->len - pos - old_str->len);
	buffer[final_len] = '\0';

	free(dst->data);
	dst->data = buffer;
	dst->len = final_len;
	dst->cap = final_len + 1;
	return true;
}

/**
 * @brief Replaces all occurrences of a substring with another.
 * @param dst Target string.
 * @param old_str Substring to replace.
 * @param new_str Replacement string.
 * @return `true` if successful, `false` otherwise.
 */
static inline bool strsafe_replace_all(StrSafe* dst, const StrSafe* old_str, const StrSafe* new_str) {
	size_t count = strsafe_count(dst, old_str);
	if (count == 0) return true;

	size_t final_len = dst->len + count * (new_str->len - old_str->len);
	char* buffer = malloc(final_len + 1);
	if (!buffer) return false;

	char* src = dst->data;
	char* out = buffer;
	while (*src) {
		if (strncmp(src, old_str->data, old_str->len) == 0) {
			memcpy(out, new_str->data, new_str->len);
			out += new_str->len;
			src += old_str->len;
		}
		else {
			*out++ = *src++;
		}
	}
	*out = '\0';

	free(dst->data);
	dst->data = buffer;
	dst->len = final_len;
	dst->cap = final_len + 1;
	return true;
}

/**
 * @brief Removes the first occurrence of a substring.
 * @param dst Target string.
 * @param str_to_remove Substring to remove.
 * @return `true` if successful, `false` otherwise.
 */
static inline bool strsafe_remove(StrSafe* dst, const StrSafe* str_to_remove) {
	ssize_t pos = cstr_find(dst, str_to_remove->data);
	if (pos < 0) return true;

	memmove(dst->data + pos, dst->data + pos + str_to_remove->len, dst->len - pos - str_to_remove->len + 1);
	dst->len -= str_to_remove->len;
	strsafe_trim(dst);
	return true;
}

/**
 * @brief Removes all occurrences of a substring.
 * @param dst Target string.
 * @param str_to_remove Substring to remove.
 * @return `true` if successful, `false` otherwise.
 */
static inline bool strsafe_remove_all(StrSafe* dst, const StrSafe* str_to_remove) {
	char* src = dst->data;
	char* out = dst->data;
	while (*src) {
		if (strncmp(src, str_to_remove->data, str_to_remove->len) == 0) {
			src += str_to_remove->len;
		}
		else {
			*out++ = *src++;
		}
	}
	*out = '\0';
	dst->len = out - dst->data;
	strsafe_trim(dst);
	return true;
}

/**
 * @brief Appends one `StrSafe` string to another.
 * @param dst Destination string.
 * @param suffix String to append.
 * @return `true` if successful, `false` otherwise.
 */
static inline bool strsafe_append(StrSafe* dst, const StrSafe* suffix) {
	size_t new_len = dst->len + suffix->len;
	if (!strsafe_ensure_capacity(dst, new_len + 1)) return false;

	memcpy(dst->data + dst->len, suffix->data, suffix->len);
	dst->len = new_len;
	dst->data[new_len] = '\0';
	return true;
}

/**
 * @brief Appends multiple `StrSafe` strings to a destination.
 * @param dst Destination string.
 * @param first First string to append.
 * @param ... Additional strings, terminated by `NULL`.
 * @return `true` if successful, `false` otherwise.
 */
static inline bool strsafe_appendv(StrSafe* dst, const StrSafe* first, ...) {
	va_list args;
	va_start(args, first);

	size_t total_len = first->len;
	const StrSafe* s;
	while ((s = va_arg(args, const StrSafe*))) {
		total_len += s->len;
	}
	va_end(args);

	if (!strsafe_ensure_capacity(dst, dst->len + total_len + 1)) return false;

	va_start(args, first);
	memcpy(dst->data + dst->len, first->data, first->len);
	dst->len += first->len;

	while ((s = va_arg(args, const StrSafe*))) {
		memcpy(dst->data + dst->len, s->data, s->len);
		dst->len += s->len;
	}
	dst->data[dst->len] = '\0';
	va_end(args);

	return true;
}

/**
 * @brief Splits a string into an array using a delimiter.
 * @param src Source string.
 * @param delim Delimiter string.
 * @return Array of substrings.
 */
static inline StrSafe_array strsafe_split(StrSafe* src, const StrSafe* delim) {
	StrSafe_array result = { 0 };
	char* start = src->data;
	char* end;

	while ((end = strstr(start, delim->data))) {
		size_t seg_len = end - start;
		result.arr = realloc(result.arr, sizeof(StrSafe) * (result.array_size + 1));
		StrSafe* seg = &result.arr[result.array_size++];
		seg->data = malloc(seg_len + 1);
		memcpy(seg->data, start, seg_len);
		seg->data[seg_len] = '\0';
		seg->len = seg_len;
		seg->cap = seg_len + 1;
		start = end + delim->len;
	}

	size_t rem_len = strlen(start);
	result.arr = realloc(result.arr, sizeof(StrSafe) * (result.array_size + 1));
	StrSafe* last = &result.arr[result.array_size++];
	last->data = malloc(rem_len + 1);
	memcpy(last->data, start, rem_len + 1);
	last->len = rem_len;
	last->cap = rem_len + 1;

	return result;
}

#endif // SAFE_STR_H
