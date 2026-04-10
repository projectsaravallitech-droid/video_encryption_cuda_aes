#pragma once

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

using SocketFd = SOCKET;
using SockLen = int;                // Windows uses int for addrLen
constexpr SocketFd INVALID_FD = INVALID_SOCKET;

inline void closeSocket(SocketFd fd) { closesocket(fd); }
inline int  getSocketError() { return WSAGetLastError(); }

#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>

using SocketFd = int;
using SockLen = socklen_t;          // Linux uses socklen_t for addrLen
constexpr SocketFd INVALID_FD = -1;

inline void closeSocket(SocketFd fd) { close(fd); }
inline int  getSocketError() { return errno; }
#endif

#include <stdexcept>

// WinSock lifecycle manager — does nothing on Linux
class PlatformInit {
public:
    PlatformInit() {
#ifdef _WIN32
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
            throw std::runtime_error("WSAStartup failed");
#endif
    }
    ~PlatformInit() {
#ifdef _WIN32
        WSACleanup();
#endif
    }
    PlatformInit(const PlatformInit&) = delete;
    PlatformInit& operator=(const PlatformInit&) = delete;
};