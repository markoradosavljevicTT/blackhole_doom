#ifndef STDLIB_H
#define STDLIB_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

char *stpcpy(char *restrict dst, const char *restrict src);
char *strcpy(char *restrict dst, const char *restrict src);
char *strncpy(char *dst, const char *restrict src, size_t dsize);
int abs(int j);
int atoi(const char *nptr);
int sprintf(char *restrict str, const char *restrict format, ...);
int strcasecmp(const char *s1, const char *s2);
int strcmp(const char *s1, const char *s2);
int strncasecmp(const char *s1, const char *s2, size_t n);
int tolower(int c);
int toupper(int c);
size_t strlen(const char *s);
void *memcpy(void *dest, const void *src, size_t n);
void *memset(void *s, int c, size_t n);
char *strcat(char *restrict dst, const char *restrict src);

#endif
