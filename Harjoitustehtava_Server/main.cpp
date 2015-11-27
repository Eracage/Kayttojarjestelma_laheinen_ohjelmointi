//#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include <winsock2.h>
#include <ws2tcpip.h>

#include <iostream>
#include <thread>
#include <mutex>

#include <string>

#pragma comment (lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT 50000

std::mutex printMutex;

void ThreadSafePrint(const char* buf)
{
	printMutex.lock();
	std::cout << buf << std::endl;
	printMutex.unlock();
}

void clientListener(SOCKET ListenSocket)
{
	int iResult;
	SOCKET ClientSocket = INVALID_SOCKET;

	iResult = listen(ListenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR)
	{
		std::cout << "listen failed with error: " << WSAGetLastError() << std::endl;
		closesocket(ListenSocket);
		return;
	}

	// Accept a client socket
	ClientSocket = accept(ListenSocket, nullptr, nullptr);
	if (ClientSocket == INVALID_SOCKET)
	{
		std::cout << "accept failed with error: " << WSAGetLastError() << std::endl;
		closesocket(ListenSocket);
		return;
	}

	// No longer need server socket
	closesocket(ListenSocket);

}

int main()
{
	WSADATA wsaData;
	int iResult;

	SOCKET ListenSocket = INVALID_SOCKET;
	SOCKET ClientSocket = INVALID_SOCKET;

	struct addrinfo *result = nullptr;
	struct addrinfo hints;

	int iSendResult;
	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
	{
		std::cout << "WSAStartup failed with error: " << iResult << std::endl;
		return 1;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	int nextSocket = DEFAULT_PORT;

	while (true)
	{
		// Resolve the server address and port
		iResult = getaddrinfo(nullptr, std::to_string(DEFAULT_PORT).c_str(), &hints, &result);
		if (iResult != 0)
		{
			std::cout << "getaddrinfo failed with error: " << iResult << std::endl;
			WSACleanup();
			return 1;
		}

		// Create a SOCKET for connecting to server
		ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
		if (ListenSocket == INVALID_SOCKET)
		{
			std::cout << "socket failed with error: " << WSAGetLastError() << std::endl;
			freeaddrinfo(result);
			WSACleanup();
			return 1;
		}

		// Setup the TCP listening socket
		iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
		if (iResult == SOCKET_ERROR)
		{
			std::cout << "bind failed with error: " << WSAGetLastError() << std::endl;
			freeaddrinfo(result);
			closesocket(ListenSocket);
			WSACleanup();
			return 1;
		}

		freeaddrinfo(result);

		iResult = listen(ListenSocket, SOMAXCONN);
		if (iResult == SOCKET_ERROR)
		{
			std::cout << "listen failed with error: " << WSAGetLastError() << std::endl;
			closesocket(ListenSocket);
			WSACleanup();
			return 1;
		}

		// Accept a client socket
		ClientSocket = accept(ListenSocket, nullptr, nullptr);
		if (ClientSocket == INVALID_SOCKET)
		{
			std::cout << "accept failed with error: " << WSAGetLastError() << std::endl;
			closesocket(ListenSocket);
			WSACleanup();
			return 1;
		}

		// No longer need server socket
		closesocket(ListenSocket);

		while (nextSocket < 65000)
		{
			nextSocket++;
			// Resolve the server address and port
			iResult = getaddrinfo(nullptr, std::to_string(nextSocket).c_str(), &hints, &result);
			if (iResult != 0)
			{
				std::cout << "getaddrinfo failed with error: " << iResult << std::endl;
				break;
			}

			// Create a SOCKET for connecting to server
			ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
			if (ListenSocket == INVALID_SOCKET)
			{
				std::cout << "socket failed with error: " << WSAGetLastError() << std::endl;
				freeaddrinfo(result);
				WSACleanup();
				return 1;
			}

			// Setup the TCP listening socket
			iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
			if (iResult == SOCKET_ERROR)
			{
				std::cout << "bind failed with error: " << WSAGetLastError() << std::endl;
				freeaddrinfo(result);
				closesocket(ListenSocket);
				WSACleanup();
				return 1;
			}

			freeaddrinfo(result);
		}
		//TODO: continue
		iSendResult = send(ClientSocket, recvbuf, sizeof(UINT32), 0);
		if (iSendResult == SOCKET_ERROR)
		{
			std::cout << "send failed with error: " << WSAGetLastError() << std::endl;
			closesocket(ClientSocket);
			WSACleanup();
			return 1;
		}

		closesocket(ClientSocket);
	}

	// Receive until the peer shuts down the connection
	do {

		iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0)
		{
			std::cout << "Bytes received: " << iResult << std::endl;

			// Echo the buffer back to the sender
			iSendResult = send(ClientSocket, recvbuf, iResult, 0);
			if (iSendResult == SOCKET_ERROR)
			{
				std::cout << "send failed with error: " << WSAGetLastError() << std::endl;
				closesocket(ClientSocket);
				WSACleanup();
				return 1;
			}
			std::cout << "Bytes sent: " << iSendResult << std::endl;
		}
		else if (iResult == 0)
			std::cout << "Connection closing..." << std::endl;
		else
		{
			std::cout << "recv failed with error: " << WSAGetLastError() << std::endl;
			closesocket(ClientSocket);
			WSACleanup();
			return 1;
		}

	} while (iResult > 0);

	// shutdown the connection since we're done
	iResult = shutdown(ClientSocket, SD_SEND);
	if (iResult == SOCKET_ERROR)
	{
		std::cout << "shutdown failed with error: " << WSAGetLastError() << std::endl;
		closesocket(ClientSocket);
		WSACleanup();
		return 1;
	}

	// cleanup
	closesocket(ClientSocket);
	WSACleanup();

	return 0;
}
#if False
int main()
{
	WSADATA wsaData;
	int iResult;

	SOCKET ListenSocket = INVALID_SOCKET;
	SOCKET ClientSocket = INVALID_SOCKET;

	struct addrinfo *result = nullptr;
	struct addrinfo hints;

	int iSendResult;
	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;

	// Initialize Winsock
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(nullptr, std::to_string(DEFAULT_PORT).c_str(), &hints, &result);
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	freeaddrinfo(result);

	listen(ListenSocket, SOMAXCONN);
	ClientSocket = accept(ListenSocket, nullptr, nullptr);
	closesocket(ListenSocket);

	do {
		iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0) {
			std::cout << "Bytes received: " << iResult << std::endl;
			iSendResult = send(ClientSocket, recvbuf, iResult, 0);
			std::cout << "Bytes sent: " << iSendResult << std::endl;
		}
		else if (iResult == 0)
			std::cout << "Connection closing..." << std::endl;

	} while (iResult > 0);

	closesocket(ClientSocket);
	WSACleanup();
	return 0;
}
#endif