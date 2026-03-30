//
// Simple socket wrapper by cat02e@fsu.edu
// ------------------------------------------
// UDPSocket supports polling and writing.
// Before you can use these features, you must
// first call beginWinsock().
//
module;
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")

export module Sockets;

import <iostream>;


export extern const uint32_t PacketMaxLength{ 520 };


//////// Error codes ////////

export std::string WSAGetErrorString(int code)
{
    switch (code) {
    case WSANOTINITIALISED:         return "WSANOTINITIALISED";
    case WSAENETDOWN:               return "WSAENETDOWN";
    case WSAEINPROGRESS:            return "WSAEINPROGRESS";
    case WSA_NOT_ENOUGH_MEMORY:     return "WSA_NOT_ENOUGH_MEMORY";
    case WSA_INVALID_HANDLE:        return "WSA_INVALID_HANDLE";
    case WSA_INVALID_PARAMETER:     return "WSA_INVALID_PARAMETER";
    case WSAEFAULT:                 return "WSAEFAULT";
    case WSAEINTR:                  return "WSAEINTR";
    case WSAEINVAL:                 return "WSAEINVAL";
    case WSAEISCONN:                return "WSAEISCONN";
    case WSAENETRESET:              return "WSAENETRESET";
    case WSAENOTSOCK:               return "WSAENOTSOCK";
    case WSAEOPNOTSUPP:             return "WSAEOPNOTSUPP";
    case WSAESOCKTNOSUPPORT:        return "WSAESOCKTNOSUPPORT";
    case WSAESHUTDOWN:              return "WSAESHUTDOWN";
    case WSAEWOULDBLOCK:            return "WSAEWOULDBLOCK";
    case WSAEMSGSIZE:               return "WSAEMSGSIZE";
    case WSAETIMEDOUT:              return "WSAETIMEDOUT";
    case WSAECONNRESET:             return "WSAECONNRESET";
    case WSAENOTCONN:               return "WSAENOTCONN";
    case WSAEDISCON:                return "WSAEDISCON";
    case WSA_IO_PENDING:            return "WSA_IO_PENDING";
    case WSA_OPERATION_ABORTED:     return "WSA_OPERATION_ABORTED";
    default:                        return "Unknown error";
    };
}


//////// Address resolution ////////

// Reverse uint8_t order
export uint16_t HTONS(uint16_t hostshort)
{
    return (hostshort >> 8) | (hostshort << 8);
}


// Resolve Internet address from dotted quad representations
export uint32_t resolveHostname(std::string_view name)
{
    uint32_t ip = inet_addr(name.data());

    if (ip == SOCKET_ERROR)
    {
        hostent* host = gethostbyname(name.data());

        if (host)
            ip = *((uint32_t*)*(host->h_addr_list));
    }

    return ip;
}


// Resolve the first routable Internet IP
export uint32_t getnetworkip()
{
    char hostname[80];

    if (gethostname(hostname, sizeof(hostname)) == SOCKET_ERROR)
    {
        std::cout << "getnetworkip()::gethostname() failed\n";

        return resolveHostname("127.0.0.1");
    }

    std::cout << "Hostname: " << hostname << "\n";

    hostent* host = gethostbyname(hostname);
    if (!host)
    {
        std::cout << "getnetworkip()::gethostbyname() failed\n";

        return resolveHostname("127.0.0.1");
    }

    uint32_t ip = resolveHostname("127.0.0.1"),
        routable = 0xffffffff;

    for (uint32_t i = 0; (host->h_addr_list[i] != 0); ++i)
    {
        in_addr addr;
        memcpy(&addr, host->h_addr_list[i], sizeof(in_addr));

        ip = addr.S_un.S_addr;
        if ((ip & 0x000000FF) == 0x0000007f)
        {    // 127.*.*.*
            std::cout << "Non-routable: " << inet_ntoa(addr) << "\n";

            continue;
        }
        if ((ip & 0x000000FF) == 0x0000000a)
        {    // 10.*.*.*
            std::cout << "Non-routable: " << inet_ntoa(addr) << "\n";

            continue;
        }
        if ((ip & 0x0000FFFF) == 0x0000a8c0)
        {    // 192.168.*.*
            std::cout << "Non-routable: " << inet_ntoa(addr) << "\n";

            continue;
        }
        if ((ip & 0x000000FF) == 0x000000ac)
        {    // 172.16.*.* - 172.31.*.*
            uint32_t node = (ip & 0x0000FF00) >> 8;

            if (node >= 16 && node <= 31)
            {
                std::cout << "Non-routable: " << inet_ntoa(addr) << "\n";

                continue;
            }
        }

        std::cout << "Routable: " << inet_ntoa(addr) << "\n";

        routable = ip;
    }

    if (routable == -1)    routable = ip;

    return routable;
}


//////// Winsock2 interface ////////

// Register process with winsock2
export void beginWinsock()
{
    WSADATA wsaData;

    if (WSAStartup(MAKEWORD(2, 0), &wsaData) == SOCKET_ERROR)
    {
        std::cout << "WSAStartup(0x202, &wsaData) error : " << WSAGetErrorString(WSAGetLastError()) 
            << " [" << WSAGetLastError() << "]\n";
    }
}


// Unregister process from winsock2
export void endWinsock()
{
    WSACleanup();
}


export struct INADDR
{
    unsigned family : 16;   // Socket family (Always INET_ADDR)
    unsigned port : 16; // Socket subclassing
    unsigned ip : 32;   // Host address
    unsigned padding0 : 32; // Unused space
    unsigned padding1 : 32; // Unused space

    //////// Addressing ////////

    // Fill address
    INADDR(uint32_t nip, uint16_t nport)
    {
        family = AF_INET;
        ip = nip;
        port = htons(nport);
    }

    // Fill address
    INADDR()
    {
        family = AF_INET;
        ip = 0;
        port = 0;
    }

    // Validate source
    bool operator==(INADDR& other)
    {
        return (other.ip == ip) && (other.port == port);
    }

    // Copy source
    void operator=(INADDR& other)
    {
        ip = other.ip;
        port = other.port;
    }

    // Dotted quad representation
    std::string getString()
    {
        in_addr in;
        in.S_un.S_addr = ip;

        return inet_ntoa(in);
    }

    // Return m_host-order
    int getPort()
    {
        return HTONS(port);
    }

    // Passable address
    sockaddr* getAddress()
    {
        return (sockaddr*)this;
    }

    // Re-fill address
    void set(uint32_t nip, uint16_t nport)
    {
        ip = nip;
        port = HTONS(nport);
    }
};


export struct UDPPacket
{
    INADDR  src;  // Sender address
    char msg[PacketMaxLength];   // Message contents
    uint32_t    len;    // Message length
};


//////// UDP Sockets ////////

export class UDPSocket
{
    SOCKET sid; // Socket identifier
    HANDLE event;   // Read event polling
    INADDR remote;  // Remote address

    UDPPacket packet;   // Speedup, not thread-safe

public:
    UDPSocket()     // Start up
    {
        sid = (SOCKET)SOCKET_ERROR;
    }

    ~UDPSocket()    // Clean up
    {
        if (sid != SOCKET_ERROR)
        {
            WSACloseEvent(event);
            closesocket(sid);
        }
    }

    SOCKET create(uint16_t port)    // Bind to a port
    {
        if (sid != SOCKET_ERROR)
        {
            WSACloseEvent(event);
            closesocket(sid);
        }

        sid = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

        if (sid == SOCKET_ERROR)
        {
            std::cout << "socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP) error : " << WSAGetErrorString(WSAGetLastError()) 
                << " [" << WSAGetLastError() << "]\n";
            return (SOCKET)SOCKET_ERROR;
        }

        INADDR sa(0, port);

        if (bind(sid, sa.getAddress(), sizeof(INADDR)) == SOCKET_ERROR)
        {
            std::cout << "bind(" << port << ") error\n";
            return (SOCKET)SOCKET_ERROR;
        }

        event = WSACreateEvent();
        WSAEventSelect(sid, event, FD_READ);

        return sid;
    }

    void set(uint32_t ip, uint16_t port)    // Set m_host
    {
        remote.set(ip, port);
    }

    void set(INADDR host)   // Set m_host
    {
        remote = host;
    }

    bool send(uint8_t* msg, size_t len)    // Send a datagram
    {
        return sendto(sid, (char*)msg, (int)len, 0, remote.getAddress(), sizeof(INADDR)) != SOCKET_ERROR;
    }

    bool send(char* msg, size_t len)   // Send a datagram
    {
        return sendto(sid, msg, (int)len, 0, remote.getAddress(), sizeof(INADDR)) != SOCKET_ERROR;
    }

    UDPPacket* poll()   // Recv a datagram
    {
        if (WSAWaitForMultipleEvents(1, &event, false, 0, true) == WSA_WAIT_EVENT_0)
        {
            int len, FromLen = sizeof(INADDR);
            INADDR src;

            len = recvfrom(sid, packet.msg, PacketMaxLength, 0, src.getAddress(), &FromLen);

            if (len <= 0)
                return NULL;

            packet.src = src;
            packet.len = len;
            return &packet;
        }
        return NULL;
    }
};
