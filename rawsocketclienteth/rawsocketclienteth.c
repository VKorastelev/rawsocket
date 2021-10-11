#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <net/ethernet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ether.h>
#include <netinet/udp.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h>
#include "ethinfo.h"
#include "crc16.h"
#include "printmac.h"
#include "printdump.h"


#define BUF_SIZE 1454


int main(int argc, char *argv[])
{
    int fd_soc = 0;

    struct sockaddr_ll server_ll = {0};
    struct ether_header *eth_header = NULL;
    struct iphdr *ip_header = NULL;
    struct udphdr *udp_header = NULL;

    //char str_eth_addr[ETH_ALEN];
    struct ether_addr *eth_addr = NULL;
    struct ether_addr dest_eth_addr = {0};

    int dest_num_port;
    int src_num_port;
    struct in_addr dest_addr;
    struct in_addr src_addr;

    char buf[BUF_SIZE];
    ssize_t size_buf_data = 0;
    ssize_t num_send_data = 0;

    size_t sum_sizeof_headers = 0;

    char if_name[IF_NAMESIZE];

    if (5 != argc) {
        fprintf(stderr, "Usage: %s <server_port> <client_port> <server ip_address_v4>"
                " <server MAC (HEX format xx:xx:xx:xx:xx:xx)>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    dest_num_port = atoi(argv[1]);

    if (dest_num_port < 1024 || dest_num_port > 49151)
    {
        fprintf(stderr, "Invalid server port number (must be in the range:"
                " 1024-49151)\n");
        exit(EXIT_FAILURE);
    }

    src_num_port = atoi(argv[2]);

    if (src_num_port < 1024 || src_num_port > 49151)
    {
        fprintf(stderr, "Invalid client port number (must be in the range:"
                " 1024-49151)\n");
        exit(EXIT_FAILURE);
    }

    if (0 == inet_aton(argv[3], &dest_addr))
    {
        fprintf(stderr, "Invalid IP_v4 address (must be xxx.xxx.xxx.xxx)\n");
        exit(EXIT_FAILURE);
    }

    eth_addr = ether_aton_r(argv[4], &dest_eth_addr);
	if (NULL == eth_addr)
    {
        fprintf(stderr, "Invalid server MAC address (must be in 6 byte HEX"
                " xx:xx:xx:xx:xx:xx): %s\n", argv[4]);
        exit(EXIT_FAILURE);
    }

    fd_soc = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (-1 == fd_soc)
    {
        perror("Error in socket(...)");
        exit(EXIT_FAILURE);
    }

    if (-1 == get_src_ifattr_finet_IPv4(&dest_addr, if_name, &src_addr))
    {
        fprintf(stderr, "Error in get_src_ifattr_finet_IPv4(...),"
                " or IP address %s is not from the local network\n", argv[3]);
        close(fd_soc);
        exit(EXIT_FAILURE);
    }

    server_ll.sll_family = AF_PACKET;
    server_ll.sll_protocol = htons(ETH_P_IP);
    
    if (0 == (server_ll.sll_ifindex = if_nametoindex(if_name)))
    {
        perror("Error in if_nametoindex(...)");
        close(fd_soc);
        exit(EXIT_FAILURE);
    }

    printf ("Ethernet interface name: %s\n", if_name);
    printf ("Ethernet interface index: %d\n", server_ll.sll_ifindex);

    if (-1 == get_src_sllattr_fpacket(
                                    if_name,
                                    server_ll.sll_addr,
                                    &server_ll.sll_halen))
    {
        fprintf(stderr, "Error in get_src_sllattr_fpacket(...),\n"
                "or IP address %s is not from the local network\n", argv[3]);
        close(fd_soc);
        exit(EXIT_FAILURE);
    }
    else if (server_ll.sll_halen > ETH_ALEN)
    {
        fprintf(stderr, "The number of bytes in an ethernet (MAC) address %d >"
                " ETH_ALEN = %d\n", server_ll.sll_halen, ETH_ALEN);
        close(fd_soc);
        exit(EXIT_FAILURE);
    }

    printf("Destination interface MAC: ");
    print_eth_addr_mac(
        dest_eth_addr.ether_addr_octet,
        sizeof(dest_eth_addr.ether_addr_octet));
    
    //printf("Server MAC: %s\n", ether_ntoa_r(&dest_eth_addr, str_eth_addr));

    printf("Source interface MAC: ");
    print_eth_addr_mac(
        server_ll.sll_addr,
        server_ll.sll_halen);

    // Clear buffer
    memset(
        buf,
        0,
        sizeof(buf));
    size_buf_data = 0;
    
    // + Ethernet header
    eth_header = (struct ether_header *)buf;

    memcpy(
            &eth_header->ether_dhost,
            &dest_eth_addr,
            sizeof(eth_header->ether_dhost));
    
    memcpy(
            &eth_header->ether_shost, 
            &server_ll.sll_addr,
            sizeof(eth_header->ether_shost));
    
    eth_header->ether_type = htons(ETHERTYPE_IP);
    
    size_buf_data += sizeof(struct ether_header);

    // + IP header
    ip_header = (struct iphdr *)&buf[size_buf_data];

    ip_header->version = 4;
    ip_header->ihl = sizeof(struct iphdr) / 4;
    ip_header->tos = 0;
    // ip_header->tot_len - size IP header and payload later
    ip_header->id = 0;
    ip_header->frag_off = htons(IP_DF);
    ip_header->ttl = 64;
    ip_header->protocol = IPPROTO_UDP;
    ip_header->check = 0;
    ip_header->saddr = src_addr.s_addr;
    ip_header->daddr = dest_addr.s_addr;
    // IP header checksum later

    size_buf_data += sizeof(struct iphdr);

    // + UDP header and payload
    udp_header = (struct udphdr *)&buf[size_buf_data];

    udp_header->source = htons(src_num_port);
    udp_header->dest = htons(dest_num_port);
    // udp_header->len - size UDP header and payload later
    udp_header->check = 0;

    size_buf_data += sizeof(struct udphdr);

    snprintf(
            &buf[size_buf_data],
            sizeof(buf)
            - size_buf_data
            -1,
            "Hello!!! This is client with PID = %d",
            getpid());

    size_buf_data += strlen(&buf[size_buf_data]) + 1;

    // Size in UDP
    udp_header->len = htons(
                            size_buf_data
                            - sizeof(struct ether_header)
                            - sizeof(struct iphdr));
    // Size in IP
    ip_header->tot_len = htons(
                            size_buf_data
                            - sizeof(struct ether_header));
    // Checksum IP header
    ip_header->check = crc16(
                            (unsigned char *)&buf[sizeof(struct ether_header)],
                            sizeof(struct iphdr));

    sum_sizeof_headers = sizeof(struct ether_header)
                        + sizeof(struct iphdr)
                        + sizeof(struct udphdr);

    printf("The size of the send buffer (Ethernet header %ld + IP header %ld"
            " + UDP header %ld bytes"
            " + string %ld bytes including \\0) = %ld,\nstring: %s\n",
            sizeof(struct ether_header),
            sizeof(struct iphdr),
            sizeof(struct udphdr),
            size_buf_data - sum_sizeof_headers,
            size_buf_data,
            &buf[sum_sizeof_headers]);

    puts("\nSend buffer:");
    print_dump((unsigned char *)buf, size_buf_data);

    num_send_data = sendto(fd_soc, buf, size_buf_data, 0, 
            (struct sockaddr *) &server_ll, sizeof(struct sockaddr_ll));
    if (0 != errno || num_send_data != size_buf_data)
    {
        perror("Error in sendto(...)");
        fprintf(stderr, "Partial send?\n");
        close(fd_soc);
        exit(EXIT_FAILURE);
    }

    printf("This client send %ld bytes\n\n", num_send_data);

    puts("Waiting packet from the server");

    while (1)
    {
        size_buf_data = recvfrom(fd_soc, buf, sizeof(buf) - 1, 0, NULL, NULL);
        if (-1 == size_buf_data)
        {
            perror("Error in recvfrom(...)");
            close(fd_soc);
            exit(EXIT_FAILURE);
        }

        udp_header = (struct udphdr *)&buf[sizeof(struct ether_header) 
                                            + sizeof(struct iphdr)];

        if (htons(src_num_port) == udp_header->dest)
        {
            buf[size_buf_data]='\0';

            printf("Client recive %ld bytes from server\n", size_buf_data);
            printf("From server receive string: %s\n", &buf[sum_sizeof_headers]);

            puts("\nReceive buffer:");
            print_dump((unsigned char *)buf, size_buf_data);

            break;
        }
    }

    close(fd_soc);

    return 0;
}
