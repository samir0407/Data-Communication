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
int main(int argc, char *argv[]) {
    const char *host = argc > 1 ? argv[1] : "127.0.0.1";
    socket_t s;
    struct sockaddr_in server;
    char response[256];
    int n;

    if (!socket_startup()) { print_socket_error("Socket startup failed"); return 1; }
    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == INVALID_SOCKET) { print_socket_error("socket"); socket_cleanup(); return 1; }

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(PORT));
    if (inet_pton(AF_INET, host, &server.sin_addr) != 1) {
        printf("Use an IPv4 address, for example 127.0.0.1\n");
        CLOSESOCKET(s); socket_cleanup(); return 1;
    }
    if (connect(s, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR) {
        print_socket_error("connect"); CLOSESOCKET(s); socket_cleanup(); return 1;
    }

    n = recv(s, response, sizeof(response) - 1, 0);
    if (n == SOCKET_ERROR) print_socket_error("recv");
    else { response[n] = '\0'; printf("Server date and time: %s\n", response); }

    CLOSESOCKET(s); socket_cleanup(); return 0;
}
