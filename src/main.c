#include "ip.h"
#include "net.h"
#include "tcp.h"
#include "hw.h"
#include "ndp_daemon.h"

#include <arpa/inet.h>

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stddef.h>

int main(int argc, const char *argv[]) {
    if(argc < 3)
    {
        fprintf(stderr, "Usage: %s <interface_name> <ip_addr>\n", argv[0]);
        return 1;
    }

    const char *ifname = argv[1];
    uint8_t src_ip_addr[IP_ADDR_LEN];
    if(!inet_pton(AF_INET6, argv[2], src_ip_addr))
    {
        fprintf(stderr, "Error: Invalid source IPv6 addr!\n");
        return 1;
    }

    printf("Options:\n");
    printf("  udp  <ip_addr> <port> <text> - send a UDP packet containing text\n");
    printf("  recv <port>                  - wait for a UDP packet and print out its data\n");
    printf("  tcp  <ip_addr> <port> <text> - send a TCP packet containing text\n");
    printf("  listen <port>                - wait for a TCP packet and print out its data\n");

    while(true)
    {
        printf("> ");

        char dst_addr[1024];
        uint8_t dst_ip_addr[IP_ADDR_LEN];
        int port;
        char buffer[1024];
        scanf("%s", buffer);
        if(strcmp(buffer, "udp") == 0)
        {
            // Get line
            if( scanf("%s %d %s", dst_addr, &port, buffer) != 3 ) {
                continue;
            }

            if(!inet_pton(AF_INET6, dst_addr, dst_ip_addr)) {
                fprintf(stderr, "Error: Invalid destination IPv6 addr!\n");
                continue;
            }

            session_t *session = net_init(ifname, src_ip_addr, port, 0, 0, UDP, -1);
            net_send(session, dst_ip_addr, port, (uint8_t*) buffer, strlen(buffer));

            net_free(session);
        }
        else if(strcmp(buffer, "listen") == 0)
        {
            if( scanf("%d", &port) != 1 ) {
                continue;
            }

            session_t *session = net_init(ifname, src_ip_addr, port, 0, 0, TCP_NOCONNECT, -1);

            session_t *sess = tcp_listen(session, src_ip_addr, port);
            size_t recv = tcp_recv(sess, (uint8_t*) buffer, sizeof(buffer));

            buffer[recv] = 0;
            printf("Received data: '%s'\n", buffer);

            net_free(session);

        }
        else if(strcmp(buffer, "tcp") == 0)
        {
            // Get line
            if( scanf("%s %d %s", dst_addr, &port, buffer) != 3 ) {
                continue;
            }

            if(!inet_pton(AF_INET6, dst_addr, dst_ip_addr)) {
                fprintf(stderr, "Error: Invalid destination IPv6 addr!\n");
                continue;
            }
            
            session_t *session = net_init(ifname, src_ip_addr, port, dst_ip_addr, port, TCP, -1);
            tcp_send(session, (uint8_t*) buffer, strlen(buffer));

            continue;
            
            net_free(session);
        }
        else if(strcmp(buffer, "recv") == 0)
        {
            if( scanf("%d", &port) != 1 ) {
                continue;
            }

            session_t *session = net_init(ifname, src_ip_addr, port, 0, 0, UDP, -1);
            size_t recv = net_recv(session, (uint8_t*) buffer, sizeof(buffer));
            net_free(session);

            buffer[recv] = 0;
            printf("Received data: '%s'\n", buffer);
        }
        else if(strcmp(buffer, "ndp") == 0)
        {
            ndp_table_print();
        }

        // Flush stdin
        scanf("%*[^\n]\n");
    }

    return 0;
}
