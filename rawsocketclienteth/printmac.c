#include <stdio.h>
#include "printmac.h"


void print_eth_addr_mac(
        unsigned char const *const eth_addr,
        size_t const eth_addr_len)
{
    for (size_t i = 0; i < eth_addr_len; i++)
    {
        printf("%02x%c",
            *(eth_addr + i),
            i < (eth_addr_len - 1) ? ':' : '\n');
    }
}
