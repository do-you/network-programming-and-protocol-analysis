#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <stdio.h>
#include <windows.h>
#include <thread>
using std::thread;

#define serverport 12345

void readthread(SOCKET soc)
{
	char buf[100];
	while (fgets(buf, 100, stdin))
	{
		send(soc, buf, strlen(buf) - 1, 0);
	}
}

void chat(SOCKET soc)
{
	char buf[100];
	int len;
	new thread(readthread, soc);

	while ((len = recv(soc, buf, 100, 0)) && len != -1)
	{
		fwrite(buf, 1, len, stdout);
		printf("\n");
	}
}

void waitforconnect()
{
	/* Create a TCP listening socket */
	SOCKET ListenSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (ListenSocket == INVALID_SOCKET)
	{
		printf("socket() failed with error: %d\n", WSAGetLastError());
		return;
	}
	/*-----------------------------------------
	*  Set up the sock addr structure that the listening socket
	*  will be bound to. In this case, the structure holds the
	*  local IP address and the port specified.
	*/
	sockaddr_in service;
	service.sin_family = AF_INET;
	service.sin_port = htons(serverport);
	service.sin_addr.s_addr = htonl(INADDR_ANY);

	/*-----------------------------------------
	*  Bind the listening socket to the IP address.
	*  and port number specified by the sockaddr structure.
	*/
	int error = bind(ListenSocket, (SOCKADDR *)&service, sizeof(SOCKADDR));
	if (error == SOCKET_ERROR)
	{
		printf("bind() failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		return;
	}


	/* Make the socket listen for incoming connection requests */
	error = listen(ListenSocket, 50);
	if (error == SOCKET_ERROR)
	{
		printf("listen() failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		return;
	}
	printf("Listening...\n");


	/*-----------------------------------------
	*  Accept an incoming connnection request on the
	*  listening socket and transfer control to the
	*  accepting socket.
	*/
	SOCKET soc = accept(ListenSocket, NULL, NULL);
	chat(soc);
}

void connectto(char *host)
{
	/* Create a TCP listening socket */
	SOCKET soc = socket(AF_INET, SOCK_STREAM, 0);
	if (soc == INVALID_SOCKET)
	{
		printf("socket() failed with error: %d\n", WSAGetLastError());
		return;
	}

	/*-----------------------------------------
	*  Set up the sock addr structure that the listening socket
	*  will be bound to. In this case, the structure holds the
	*  local IP address and the port specified.
	*/
	sockaddr_in remote;
	remote.sin_family = AF_INET;
	remote.sin_port = htons(serverport);
	remote.sin_addr.s_addr = inet_addr(host);

	int iResult = connect(soc, (sockaddr*)&remote, sizeof(sockaddr));
	if (iResult == SOCKET_ERROR)
	{
		printf("connect function failed with error: %ld\n", WSAGetLastError());
		iResult = closesocket(soc);
		if (iResult == SOCKET_ERROR)
			printf("closesocket function failed with error: %ld\n", WSAGetLastError());
		return;
	}

	chat(soc);
}

int main(int argc, char **argv)
{
	WSADATA wsaData;
	/* Initialize Winsock */
	int error = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (error)
	{
		printf("WSAStartup() failed with error: %d\n", error);
		return 1;
	}

	if (argc == 2)
		connectto(argv[1]);
	else
		waitforconnect();

	WSACleanup();
	return 0;
}


