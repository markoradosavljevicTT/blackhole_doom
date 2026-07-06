#include "stdlib.h"

#include <stdarg.h>
#define MAX_INT 2147483647

__attribute__((section(".critical")))
int abs(int j)
{
	return (j >= 0) ? j : -j;
}

char *strcpy(char *restrict dst, const char *restrict src)
{
	char *tmp = dst;
	while ((*dst++ = *src++) != '\0')
		;
	return tmp;
}

__attribute__((section(".critical")))
char *strncpy(char *dst, const char *src, size_t n)
{
	char *start = dst;
	size_t i;

	for (i = 0; i < n && src[i] != '\0'; i++) {
		dst[i] = src[i];
	}

	for (; i < n; i++) {
		dst[i] = '\0';
	}

	return start;
}

int atoi(const char *nptr)
{
	int result = 0;
	int sign = 1;

	while (*nptr == ' ' || *nptr == '\t' || *nptr == '\n' || *nptr == '\v' || *nptr == '\f' || *nptr == '\r') {
		nptr++;
	}

	if (*nptr == '-' || *nptr == '+') {
		if (*nptr == '-') {
			sign = -1;
		}
		nptr++;
	}

	while (*nptr >= '0' && *nptr <= '9') {
		result = result * 10 + (*nptr - '0');
		nptr++;
	}

	return sign * result;
}

void append_char(char *str, size_t *index, size_t size, char c)
{
	if (*index < size - 1) {
		str[*index] = c;
	}
	(*index)++;
}

void append_string(char *str, size_t *index, size_t size, const char *s)
{
	while (*s) {
		append_char(str, index, size, *s++);
	}
}

void append_int(char *str, size_t *index, size_t size, int value, int num_size)
{
	if (value < 0) {
		append_char(str, index, size, '-');
		value = -value;
	}

	int power_of_10 = 1;
	int num_copy = value;
	int cnt = 0;
	while (num_copy >= 10) {
		num_copy /= 10;
		power_of_10 *= 10;
		cnt++;
	}

	while (cnt < --num_size) {
		append_char(str, index, size, '0');
	}

	while (power_of_10 > 0) {
		int digit = 0;
		while (value >= power_of_10) {
			value -= power_of_10;
			digit++;
		}
		append_char(str, index, size, digit + '0');
		power_of_10 /= 10;
	}
}

int sprintf(char *restrict str, const char *restrict format, ...)
{
	va_list args;
	va_start(args, format);
	size_t index = 0;

	for (; *format; format++) {
		if (*format == '%') {
			format++;
			if (*format == 'd' || *format == 'i') {
				int value = va_arg(args, int);
				append_int(str, &index, MAX_INT, value, 0);
			} else if (*format == 's') {
				const char *value = va_arg(args, const char *);
				append_string(str, &index, MAX_INT, value);
			} else if (*format == '.' && *(format + 1) > '0' && *(format + 1) < '9') {
				format++;
				if (*(format + 1) == 'd' || *(format + 1) == 'i') {
					int value = va_arg(args, int);
					append_int(str, &index, MAX_INT, value, *format - '0');
					format++;
				} else {
					//__asm__ __volatile__("ecall");
				}
			} else {
				//__asm__ __volatile__("ecall");
			}
		} else {
			append_char(str, &index, MAX_INT, *format);
		}
	}

	str[index] = '\0';

	va_end(args);

	return (int)index;
}

int tolower(int c)
{
	if (c >= 'A' && c <= 'Z') {
		return c + ('a' - 'A');
	}
	return c;
}

int toupper(int c)
{
	if (c >= 'a' && c <= 'z') {
		return c - ('a' - 'A');
	}
	return c;
}

int strcasecmp(const char *s1, const char *s2)
{
	while (*s1 && *s2) {
		char c1 = tolower((unsigned char)*s1);
		char c2 = tolower((unsigned char)*s2);

		if (c1 != c2) {
			return (unsigned char)c1 - (unsigned char)c2;
		}
		s1++;
		s2++;
	}
	return (unsigned char)tolower((unsigned char)*s1) - (unsigned char)tolower((unsigned char)*s2);
}

int strcmp(const char *s1, const char *s2)
{
	while (*s1 && (*s1 == *s2)) {
		s1++;
		s2++;
	}
	return *s1 - *s2;
}

int strncasecmp(const char *s1, const char *s2, size_t n)
{
	while (--n > 0 && *s1 && *s2) {
		char c1 = tolower((unsigned char)*s1);
		char c2 = tolower((unsigned char)*s2);

		if (c1 != c2) {
			return (unsigned char)c1 - (unsigned char)c2;
		}
		s1++;
		s2++;
	}
	return (unsigned char)tolower((unsigned char)*s1) - (unsigned char)tolower((unsigned char)*s2);
}

size_t strlen(const char *s)
{
	if (s == NULL) {
		return 0;
	}

	const char *p = s;
	while (*p != '\0') {
		p++;
	}

	return p - s;
}

__attribute__((section(".critical")))
void *memcpy(void *dest, const void *src, size_t n) {
    char *d = (char *)dest;
    const char *s = (const char *)src;

    size_t i = 0;
    if (((size_t)d & 3) == 0 && ((size_t)s & 3) == 0) {
        for (; i + 4 <= n; i += 4) {
            *(int *)(d + i) = *(const int *)(s + i);
        }
    }

    for (; i < n; ++i) {
        d[i] = s[i];
    }

    return dest;
}

__attribute__((section(".critical")))
void *memset(void *s, int c, size_t n) {
    unsigned char *p = (unsigned char *)s;
    unsigned int value = (unsigned char)c;

    value |= (value << 8);
    value |= (value << 16);

    size_t i = 0;
    if (((size_t)p & 3) == 0) {
        for (; i + 4 <= n; i += 4) {
            *(unsigned int *)(p + i) = value;
        }
    }

    for (; i < n; ++i) {
        p[i] = (unsigned char)c;
    }

    return s;
}

char *strcat(char *restrict dst, const char *restrict src)
{
	char *ptr = dst;
	while (*ptr) {
		ptr++;
	}
	while (*src) {
		*ptr++ = *src++;
	}
	*ptr = '\0';
	return dst;
}
