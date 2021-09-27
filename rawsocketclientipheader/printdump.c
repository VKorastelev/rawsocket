#include <stdio.h>
#include "printdump.h"


void print_dump(
        unsigned char const *const buf,
        ssize_t const size)
{
    int i = 0;
    int piece = 0;

    if (NULL != buf && 0 != size)
    {
        for (i = 0; i < size; i++)
        {
            printf("%02x ", buf[i]);

            if (15 == (i % 16))
            {
                putchar('|');
                print_chars(&buf[i - 15], 16);
                putchar('\n');
            }
        }

        piece = i % 16;

        if (0 != piece)
        {
            for (int j = 0; j < 16 - piece; j++)
            {
                printf("   ");
            }

            putchar('|');
            print_chars(&buf[i - piece], piece);
            putchar('\n');
        }

        putchar('\n');
    }
}


void print_chars(
        unsigned char const *const buf,
        ssize_t const size)
{
    if (NULL != buf && 0 != size)
    {
        for (int i = 0; i < size; i++)
        {
            if (buf[i] < 0x20 || buf[i] > 0x7E)
            {
                putchar(0x2E);
            }
            else
            {
                putchar(buf[i]);
            }
        }
    }
}
