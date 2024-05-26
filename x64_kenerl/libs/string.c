
#include "libs/string.h"
#include "linux/type.h"

char *strcpy(char *dest, const char *src)
{
    char *ptr = dest;
    while (true)
    {
        *ptr++ = *src;
        if (*src++ == EOS)
            return dest;
    }
}

char *strcat(char *dest, const char *src)
{
    char *ptr = dest;
    while (*ptr != EOS)
    {
        ptr++;
    }
    while (true)
    {
        *ptr++ = *src;
        if (*src++ == EOS)
        {
            return dest;
        }
    }
}

size_t strlen(const char *str)
{
    char *ptr = (char *)str;
    while (*ptr != EOS)
    {
        ptr++;
    }
    return ptr - str;
}

int strcmp(const char *lhs, const char *rhs)
{
    while (*lhs == *rhs && *lhs != EOS && *rhs != EOS)
    {
        lhs++;
        rhs++;
    }
    return *lhs < *rhs ? -1 : *lhs > *rhs;
}

char *strchr(const char *str, int ch)
{
    char *ptr = (char *)str;
    while (true)
    {
        if (*ptr == ch)
        {
            return ptr;
        }
        if (*ptr++ == EOS)
        {
            return NULL;
        }
    }
}

char *strrchr(const char *str, int ch)
{
    char *last = NULL;
    char *ptr = (char *)str;
    while (true)
    {
        if (*ptr == ch)
        {
            last = ptr;
        }
        if (*ptr++ == EOS)
        {
            return last;
        }
    }
}

int memcmp(const void *cs, const void *ct, size_t count)
{
	const unsigned char *su1, *su2;
	int res = 0;

	for (su1 = cs, su2 = ct; 0 < count; ++su1, ++su2, count--)
		if ((res = *su1 - *su2) != 0)
			break;
	return res;
}

void *memset(void *dest, int ch, size_t count)
{
    char *ptr = dest;
    while (count--)
    {
        *ptr++ = ch;
    }
    return dest;
}

void *memcpy(void *dest, const void *src, size_t count)
{
    char *ptr = dest;
    while (count--)
    {
        *ptr++ = *((char *)(src++));
    }
    return dest;
}

void *memchr(const void *str, int ch, size_t count)
{
    char *ptr = (char *)str;
    while (count--)
    {
        if (*ptr == ch)
        {
            return (void *)ptr;
        }
        ptr++;
    }
}