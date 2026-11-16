#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#pragma comment(lib, "Ws2_32.lib")
typedef SOCKET socket_t;
#define CLOSESOCKET closesocket
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <time.h>
typedef int socket_t;
#define INVALID_SOCKET (-1)
#define SOCKET_ERROR (-1)
#define CLOSESOCKET close
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static int socket_startup(void) {
#ifdef _WIN32
    WSADATA wsa;
    return WSAStartup(MAKEWORD(2, 2), &wsa) == 0;
#else
    return 1;
#endif
}
static void socket_cleanup(void) {
#ifdef _WIN32
    WSACleanup();
#endif
}
static void print_socket_error(const char *message) {
#ifdef _WIN32
    printf("%s (Winsock error: %d)\n", message, WSAGetLastError());
#else
    perror(message);
#endif
}

#define PORT "13002"
int main(void) {
    socket_t server_socket, client_socket;
    struct sockaddr_in addr, client;
    int client_len = sizeof(client);
    char response[128];
    time_t now;
    struct tm *tm_info;

    if (!socket_startup()) { print_socket_error("Socket startup failed"); return 1; }
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET) { print_socket_error("socket"); socket_cleanup(); return 1; }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(atoi(PORT));
    if (bind(server_socket, (struct sockaddr *)&addr, sizeof(addr)) == SOCKET_ERROR ||
        listen(server_socket, 5) == SOCKET_ERROR) {
        print_socket_error("bind/listen"); CLOSESOCKET(server_socket); socket_cleanup(); return 1;
    }

    printf("Daytime TCP server running on port %s...\n", PORT);
    for (;;) {
        client_socket = accept(server_socket, (struct sockaddr *)&client, &client_len);
        if (client_socket == INVALID_SOCKET) { print_socket_error("accept"); continue; }
        now = time(NULL);
        tm_info = localtime(&now);
        strftime(response, sizeof(response), "%A, %d %B %Y %H:%M:%S", tm_info);
        send(client_socket, response, (int)strlen(response), 0);
        CLOSESOCKET(client_socket);
        client_len = sizeof(client);
    }
}
