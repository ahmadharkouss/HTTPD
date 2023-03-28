#include <stddef.h>
#include <stdio.h>
#include <strings.h>

size_t my_strlen(const char *str)
{
    if (!str)
        return 0;
    size_t k = 0;
    while (str[k] != '\0')
        ++k;
    return k;
}

char to_lower(char s)
{
    if (s >= 'A' && s <= 'Z')
    {
        return s + 32;
    }

    return s;
}

int _my_strcasecmp(const char *s1, const char *s2)
{
    if (my_strlen(s1) > my_strlen(s2))
    {
        return 10;
    }
    else if (my_strlen(s1) < my_strlen(s2))
    {
        return -10;
    }
    else
    {
        while (*s1)
        {
            if (to_lower(*s1) != to_lower(*s2))
            {
                break;
            }

            s1++;
            s2++;
        }
        return *(const unsigned char *)s1 - *(const unsigned char *)s2;
    }
}

int my_strcasecmp(const char *s1, const char *s2)
{
    int r = _my_strcasecmp(s1, s2);
    if (r < 0)
    {
        return -10;
    }
    else if (r > 0)
    {
        return 10;
    }
    else
    {
        return 0;
    }
}