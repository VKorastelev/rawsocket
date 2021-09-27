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


#define BUF_SIZE 1440


int main(int argc, char *argv[])
{
    int fd_soc = 0;

    struct sockaddr_in server;
    struct udphdr udp_header = {0};
    struct udphdr *pudp_header = NULL;

    int srv_num_port;
    int cli_num_port;
    struct in_addr addr;

    char buf[BUF_SIZE];
    ssize_t size_buf_data = 0;
    ssize_t num_send_data = 0;

    if (argc != 4) {
        fprintf(stderr, "Usage: %s server_port client_port ip_address_v4\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if (0 == inet_aton(argv[3], &addr))
    {
        fprintf(stderr, "Invalid IP_v4 address (must be xxx.xxx.xxx.xxx)\n");
        fprintf(stderr, "Usage: %s server_port client_port ip_address_v4\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    srv_num_port = atoi(argv[1]);

    if (srv_num_port < 1024 || srv_num_port > 49151)
    {
        fprintf(stderr, "Invalid server port number (must be in the range:"
                " 1024-49151)\n");
        fprintf(stderr, "Usage: %s server_port client_port ip_address_v4\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    cli_num_port = atoi(argv[2]);

    if (cli_num_port < 1024 || cli_num_port > 49151)
    {
        fprintf(stderr, "Invalid client port number (must be in the range:"
                " 1024-49151)\n");
        fprintf(stderr, "Usage: %s server_port client_port ip_address_v4\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    memset(&server, 0, sizeof(struct sockaddr_in));
    server.sin_family = AF_INET;
    server.sin_port = htons(srv_num_port);
    server.sin_addr.s_addr = addr.s_addr;

    printf("Server IP address in network byte order:%d\n", server.sin_addr.s_addr);
    printf("Server port number in network byte order:%d\n", server.sin_port);

    fd_soc = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
    if (-1 == fd_soc)
    {
        perror("Error in socket(...)");
        exit(EXIT_FAILURE);
    }

    snprintf(buf, sizeof(buf) - 1 - sizeof(udp_header), "Hello!!! Client PID = %d",
            getpid());
    size_buf_data = strlen(buf) + 1;

    udp_header.source = htons(cli_num_port);
    udp_header.dest = htons(srv_num_port);
    udp_header.len = htons(sizeof(udp_header) + size_buf_data);
    udp_header.check = 0;

    printf("udp_header.len = %d sizeof(udphdr) = %ld\n",
            udp_header.len,
            sizeof(udp_header));

    memmove(&buf[sizeof(udp_header)], buf, size_buf_data);

    memcpy(buf, &udp_header, sizeof(udp_header));

    size_buf_data += sizeof(udp_header);

    printf("The size of the sent buffer (UDP header %ld bytes + string %ld"
            " bytes including \\0) = %ld, string: %s\n",
            sizeof(udp_header),
            strlen(&buf[sizeof(udp_header)]) + 1,
            size_buf_data,
            &buf[sizeof(udp_header)]);

    num_send_data = sendto(fd_soc, buf, size_buf_data, 0, 
            (struct sockaddr *) &server, sizeof(struct sockaddr_in));
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

        pudp_header = (struct udphdr *)&buf[20];

        if (htons(cli_num_port) == pudp_header->dest)
        {
            buf[size_buf_data]='\0';

            printf("Client recive %ld bytes from server\n", size_buf_data);
            printf("From server receive string: %s\n", &buf[20 + sizeof(udp_header)]);
            break;
        }
    }

    close(fd_soc);

    return 0;
}
