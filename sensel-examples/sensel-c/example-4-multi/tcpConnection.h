#pragma once

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>


// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")


#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

extern WSADATA wsaData;
extern SOCKET ConnectSocket;
extern SOCKET ListenSocket;
extern SOCKET ClientSocket;

enum TCPMode
{
    Client,
    Server
};

extern bool isTcpSetup;
int tcpSetup(TCPMode tcpMode);
int tcpClientSetup();
int tcpServerSetup();
int tcpSendMsg(TCPMode tcpMode, std::string msg = "");
int tcpClose(TCPMode tcpMode);
int tcpListen();