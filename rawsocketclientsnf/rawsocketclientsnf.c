#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/udp.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h>
#include "printdump.h"


#define BUF_SIZE 1440


int main(int argc, char *argv[])
{
    int fd_soc = 0;

    struct sockaddr_in server_in = {0};
    struct udphdr *udp_header = NULL;

    int dest_num_port;
    int src_num_port;
    struct in_addr dest_addr;

    char buf[BUF_SIZE];
    ssize_t size_buf_data = 0;
    ssize_t num_send_data = 0;

    if (4 != argc) {
        fprintf(stderr, "Usage: %s <server_port> <client_port> <server ip_address_v4>\n",
                argv[0]);
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

    //memset(&server_in, 0, sizeof(struct sockaddr_in));
    server_in.sin_family = AF_INET;
    server_in.sin_port = htons(dest_num_port);
    server_in.sin_addr.s_addr = dest_addr.s_addr;

    printf("Server port number in network byte order:%x\n", server_in.sin_port);
    printf("Client port number in network byte order:%x\n", htons(src_num_port));
    printf("Server IP address in network byte order:%x\n", server_in.sin_addr.s_addr);

    fd_soc = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
    if (-1 == fd_soc)
    {
        perror("Error in socket(...)");
        exit(EXIT_FAILURE);
    }

    // Clear buffer
    memset(
        buf,
        0,
        sizeof(buf));
    size_buf_data = 0;

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
            - 1,
            "Hello!!! This is client with PID = %d",
            getpid());

    size_buf_data += strlen(&buf[size_buf_data]) + 1;

    // Size in UDP
    udp_header->len = htons(size_buf_data);

    printf("The size of the send buffer (UDP header %ld bytes"
            " + string %ld bytes including \\0) = %ld,\nstring: %s\n",
            sizeof(struct udphdr),
            size_buf_data - sizeof(struct udphdr),
            size_buf_data,
            &buf[sizeof(udp_header)]);

    puts("\nSend buffer:");
    print_dump((unsigned char *)buf, size_buf_data);

    num_send_data = sendto(fd_soc, buf, size_buf_data, 0, 
            (struct sockaddr *) &server_in, sizeof(struct sockaddr_in));
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

        printf("Client recive %ld bytes from UDP-RAW socket:\n",
                size_buf_data);

        print_dump((unsigned char *)buf, size_buf_data);
    }

    close(fd_soc);

    return 0;
}
