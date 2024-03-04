#define WIN32_LEAN_AND_MEAN

#include <iostream>
#include <windows.h>
#include <ws2tcpip.h>
using namespace std;

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

#define PAUSE 1

// Attempt to connect to an address until one succeeds
SOCKET ConnectSocket = INVALID_SOCKET;

DWORD WINAPI Sender(void* param)
{
	while (true)
	{
		char message[200];
		cin.getline(message, 199);

		int iResult = send(ConnectSocket, message, (int)strlen(message), 0);
		if (iResult == SOCKET_ERROR) {
			cout << "send failed with error: " << WSAGetLastError() << "\n";
			closesocket(ConnectSocket);
			WSACleanup();
			return 15;
		}
	}
	return 0;
}

DWORD WINAPI Receiver(void* param)
{
	while (true)
	{
		char answer[DEFAULT_BUFLEN];

		int iResult = recv(ConnectSocket, answer, DEFAULT_BUFLEN, 0);
		answer[iResult] = '\0';

		if (iResult > 0) {
			cout << answer << "\n";
			// cout << "байтов получено: " << iResult << "\n";
		}
		else if (iResult == 0)
			cout << "соединение с сервером закрыто.\n";
		else
			cout << "recv failed with error: " << WSAGetLastError() << "\n";

	}
	return 0;
}

int main()
{
	setlocale(0, "");
	system("title CLIENT SIDE");

	/////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Initialize Winsock
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		cout << "WSAStartup failed with error: " << iResult << "\n";
		return 11;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////

	addrinfo hints;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Resolve the server address and port
	const char* ip = "localhost"; // по умолчанию, оба приложения, и клиент, и сервер, запускаются на одной и той же машине
	//cout << "Please, enter server name: ";
	//cin.getline(ip, 199);


	// iResult = getaddrinfo(argv[1], DEFAULT_PORT, &hints, &result); // раскомментировать, если нужно будет читать имя сервера из командной строки
	addrinfo* result = NULL;
	iResult = getaddrinfo(ip, DEFAULT_PORT, &hints, &result);

	if (iResult != 0) {
		cout << "getaddrinfo failed with error: " << iResult << "\n";
		WSACleanup();
		return 12;
	}
	else {
		// cout << "получение адреса и порта клиента прошло успешно!\n";
		Sleep(PAUSE);
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////

	for (addrinfo* ptr = result; ptr != NULL; ptr = ptr->ai_next) { // серверов может быть несколько, поэтому не помешает цикл

		// Create a SOCKET for connecting to server
		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

		if (ConnectSocket == INVALID_SOCKET) {
			cout << "socket failed with error: " << WSAGetLastError() << "\n";
			WSACleanup();
			return 13;
		}

		// Connect to server
		iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			closesocket(ConnectSocket);
			ConnectSocket = INVALID_SOCKET;
			continue;
		}

		break;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////

	freeaddrinfo(result);

	if (ConnectSocket == INVALID_SOCKET) {
		cout << "невозможно подключиться к серверу!\n";
		WSACleanup();
		return 14;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////

	CreateThread(0, 0, Sender, 0, 0, 0);
	CreateThread(0, 0, Receiver, 0, 0, 0);

	Sleep(INFINITE);
}