#ifndef _ETHINFO_H_
#define _ETHINFO_H_


int get_src_ifattr_finet_IPv4(
        struct in_addr const *const ip_dest,
        char *ifname,
        struct in_addr *const ip_src);


int get_src_addr_finet_IPv4(
        struct in_addr const *ip_dest,
        struct in_addr *ip_src);


int get_src_sllattr_fpacket(
        char *ifname,
        unsigned char *slladdr,
        unsigned char *sllhalen);

#endif
