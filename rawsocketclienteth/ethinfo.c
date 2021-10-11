#define _GNU_SOURCE
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
//#include <netdb.h>
//#include <arpa/inet.h>
#include <ifaddrs.h>
#include <string.h>
#include "ethinfo.h"

/*
int get_src_ethname_fnet(
        struct in_addr const *const ip_dest,
        struct in_addr *const ip_src
        )
{
}
*/
static struct ifaddrs *ifaddr_list = NULL;


static int get_ifaddrs_inet(
        struct in_addr const *const ip_dest,
        struct ifaddrs **item_addr);


static int get_ifaddrs_packet(
        char *if_name,
        struct ifaddrs **item_addr);


int get_src_ifattr_finet_IPv4(
        struct in_addr const *const ip_dest,
        char *ifname,
        struct in_addr *const ip_src
        )
{
    int ret = -1;
    struct ifaddrs *item_addr = NULL;

    if (NULL == ip_dest || NULL == ip_src)
    {
        goto finally;
    }

    if (-1 == get_ifaddrs_inet(ip_dest, &item_addr))
    {
        goto finally;
    }

    if (NULL != item_addr)
    {
        memcpy(
            ifname,
            item_addr->ifa_name,
            IF_NAMESIZE);

        memcpy(
            ip_src,
            &((struct sockaddr_in *)item_addr->ifa_addr)->sin_addr,
            sizeof(struct in_addr));

        ret = 0;
    }

    freeifaddrs(ifaddr_list);
    ifaddr_list = NULL;

 finally:

    return ret;
}


int get_src_addr_finet_IPv4(
        struct in_addr const *const ip_dest,
        struct in_addr *const ip_src
        )
{
    int ret = -1;
    struct ifaddrs *item_addr = NULL;

    if (NULL == ip_dest || NULL == ip_src)
    {
        goto finally;
    }

    if (-1 == get_ifaddrs_inet(ip_dest, &item_addr))
    {
        goto finally;
    }

    if (NULL != item_addr)
    {
        memcpy(
            ip_src,
            &((struct sockaddr_in *)item_addr->ifa_addr)->sin_addr,
            sizeof(struct in_addr));

        ret = 0;
    }

    freeifaddrs(ifaddr_list);
    ifaddr_list = NULL;

 finally:

    return ret;
}


int get_src_sllattr_fpacket(
        char *ifname,
        unsigned char *slladdr,
        unsigned char *sllhalen)
{
    int ret = -1;
    struct ifaddrs *item_addr = NULL;

    if (NULL == ifname
        || NULL == slladdr
        || NULL == sllhalen
        || 0 == strlen(ifname))
    {
        goto finally;
    }

    if (-1 == get_ifaddrs_packet(ifname, &item_addr))
    {
        goto finally;
    }

    if (NULL != item_addr)
    {
        *sllhalen = ((struct sockaddr_ll *)(item_addr->ifa_addr))->sll_halen;

        memcpy(
            slladdr,
            ((struct sockaddr_ll *)item_addr->ifa_addr)->sll_addr,
            *sllhalen);

        ret = 0;
    }

    freeifaddrs(ifaddr_list);
    ifaddr_list = NULL;

 finally:

    return ret;
}


static int get_ifaddrs_inet(
        struct in_addr const *const ip_dest,
        struct ifaddrs **item_addr)
{
    int ret = 0;
    struct ifaddrs *ifa = NULL;
    in_addr_t loc_ip = 0;
    in_addr_t loc_netmask = 0;

    if (-1 == (ret = getifaddrs(&ifaddr_list)))
    {
        perror("Error in getifaddrs(...) in function get_ifaddrs_inet(...)");
        goto finally;
    }

    *item_addr = NULL;

    for (ifa = ifaddr_list; NULL != ifa; ifa = ifa->ifa_next)
    {
        if (NULL == ifa->ifa_addr 
            || AF_INET != ifa->ifa_addr->sa_family)
        {
            continue;
        }

        loc_ip = ((struct sockaddr_in *)ifa->ifa_addr)->sin_addr.s_addr;
        loc_netmask = ((struct sockaddr_in *)ifa->ifa_netmask)->sin_addr.s_addr;

        if ((loc_netmask & loc_ip) == (loc_netmask & ip_dest->s_addr))
        {
            *item_addr = ifa;
            break;
        }
    }

 finally:

    return ret;
}


static int get_ifaddrs_packet(
        char *if_name,
        struct ifaddrs **item_addr)
{
    int ret = 0;
    struct ifaddrs *ifa = NULL;

    if (-1 == (ret = getifaddrs(&ifaddr_list)))
    {
        perror("Error in getifaddrs(...) in function get_ifaddrs_packet(...)");
        goto finally;
    }

    *item_addr = NULL;

    for (ifa = ifaddr_list; NULL != ifa; ifa = ifa->ifa_next)
    {
        if (NULL == ifa->ifa_addr
            || AF_PACKET != ifa->ifa_addr->sa_family
            || 0 != strcmp(ifa->ifa_name, if_name))
        {
            continue;
        }
        
        *item_addr = ifa;
        break;
    }

 finally:

    return ret;
}
