#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdio.h>
#include <vector>

#pragma comment(lib, "Ws2_32.lib")

int main() {
	WSADATA wsData;

	int error_status = WSAStartup(MAKEWORD(2, 2), &wsData);

	if (error_status == 0) {
		SOCKET client_sock = socket(AF_INET, SOCK_STREAM, 0);

		if (client_sock == INVALID_SOCKET) {
			std::cout << "invalid socket, error: " << WSAGetLastError() << std::endl;

			closesocket(client_sock);
			WSACleanup();

			return -1;
		}

		in_addr ip_to_num;
		error_status = inet_pton(AF_INET, "127.0.0.1", &ip_to_num);

		if (error_status <= 0) {
			std::cout << "inet_pton, error with IP translation" << std::endl;
		}

		sockaddr_in serv_info;

		ZeroMemory(&serv_info, sizeof(serv_info));

		serv_info.sin_family = AF_INET;
		serv_info.sin_addr = ip_to_num;
		serv_info.sin_port = htons(9000);

		error_status = connect(client_sock, (sockaddr*)(&serv_info), sizeof(serv_info));

		if (error_status != 0) {
			std::cout << "connection is failed, error: " << WSAGetLastError() << std::endl;
			closesocket(client_sock);
			WSACleanup();
			return -1;
		}

		std::cout << "connection sucesfully" << std::endl;


		int buff_size = 1024;
		std::vector<char> serv_buf(buff_size);
		std::vector<char> client_buf(buff_size);
		short packet_size;


		while (true) {
			std::string client_message;

			std::cout << "enter yout message: " << std::endl;

			std::cin >> client_message;

			for (int i = 0; i < min((size_t)buff_size - 1, client_message.size()); i++) {
				client_buf[i] = client_message[i];
			}
			client_buf[min((size_t)buff_size, client_message.size())] = '\0';

			packet_size = send(client_sock, client_buf.data(), client_buf.size(), 0);

			if (packet_size == SOCKET_ERROR) {
				std::cout << "SOCKET_ERROR" << std::endl;
			}

			packet_size = recv(client_sock, serv_buf.data(), serv_buf.size(), 0);

			if (packet_size == SOCKET_ERROR) {
				std::cout << "SOCKET_ERROR" << std::endl;
			}

			std::cout << "client message: " << serv_buf.data() << std::endl;

		}

		closesocket(client_sock);
		WSACleanup();
	}
	else {
		std::cout << "WSAStartup error" << std::endl;

		return -1;
	}

	return 0;
}

