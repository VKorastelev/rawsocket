#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
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
    struct sockaddr_storage client_addr;
    socklen_t client_addr_size;

    int num_port;
    struct in_addr addr;

    int ret = 0;

    char buf[BUF_SIZE];
    ssize_t size_buf_data = 0;
    ssize_t num_send_data = 0;

    if (argc != 3) {
        fprintf(stderr, "Usage: %s port ip_address_v4\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if (0 == inet_aton(argv[2], &addr))
    {
        fprintf(stderr, "Invalid IP_v4 address (must be xxx.xxx.xxx.xxx)\n");
        fprintf(stderr, "Usage: %s port ip_address_v4\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    num_port = atoi(argv[1]);

    if (num_port < 1024 || num_port > 49151)
    {
        fprintf(stderr, "Invalid port number (must be in the range: 1024-49151)\n");
        fprintf(stderr, "Usage: %s port ip_address_v4\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    memset(&server, 0, sizeof(struct sockaddr_in));

    server.sin_family = AF_INET;

    server.sin_port = htons(num_port);

    server.sin_addr.s_addr = addr.s_addr;

    printf("Server IP address in network byte order:%d\n", server.sin_addr.s_addr);
    printf("Server port number in network byte order:%d\n", server.sin_port);

    fd_soc = socket(AF_INET, SOCK_DGRAM, 0);
    if (-1 == fd_soc)
    {
        perror("Error in socket(...)");
        exit(EXIT_FAILURE);
    }
    if (-1 == bind(fd_soc, (struct sockaddr *) &server, sizeof(struct sockaddr_in)))
    {
        perror("Error in bind(...)");
        close(fd_soc);
        exit(EXIT_FAILURE);
    }

    puts("Binding OK! Waiting packet from the client");

    client_addr_size = sizeof(struct sockaddr_storage);
    size_buf_data = recvfrom(fd_soc, buf, sizeof(buf) - 1, 0,
                       (struct sockaddr *)&client_addr, &client_addr_size);
    if (-1 == size_buf_data)
    {
        perror("Error in recvfrom(...)");
        close(fd_soc);
        exit(EXIT_FAILURE);
    }

    buf[size_buf_data]='\0';

    char host[NI_MAXHOST], service[NI_MAXSERV];

    ret = getnameinfo((struct sockaddr *)&client_addr, client_addr_size,
            host, NI_MAXHOST,
            service, NI_MAXSERV,
            NI_NUMERICSERV);

    if (0 != ret)
    {
        fprintf(stderr, "Error in getnameinfo(...): %s\n", gai_strerror(ret));
    }
    else
    {
        printf("From client (%s:%s) receive %ld bytes: %s\n\n",
                host,
                service,
                size_buf_data,
                buf);
    }

    snprintf(buf, sizeof(buf) - 1, "This server! Server PID = %d",
            getpid());
        
    size_buf_data = strlen(buf) + 1;

    printf("Send data string (size = %ld, len = %ld): %s\n",
            size_buf_data,
            strlen(buf),
            buf);

    num_send_data = sendto(fd_soc, buf, size_buf_data, 0, 
            (struct sockaddr *)&client_addr, client_addr_size);
    if (0 != errno || num_send_data != size_buf_data)
    {
        perror("Error in sendto(...)");
        fprintf(stderr, "Partial send?\n");
        close(fd_soc);
        exit(EXIT_FAILURE);
    }

    printf("This server send %ld bytes\n", num_send_data);

    close(fd_soc);

    return 0;
}
