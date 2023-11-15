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
		SOCKET serv_sock =  socket(AF_INET, SOCK_STREAM, 0);

		if (serv_sock == INVALID_SOCKET) {
			std::cout << WSAGetLastError() << std::endl;

			closesocket(serv_sock);
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

		error_status = bind(serv_sock, (sockaddr*)(&serv_info), sizeof(serv_info));

		if (error_status != 0) {
			std::cout << "bind error: " << WSAGetLastError() << std::endl;
			closesocket(serv_sock);
			WSACleanup();
			return -1;
		}

		std::cout << "binding is OK" << std::endl;

		error_status = listen(serv_sock, SOMAXCONN);

		if (error_status != 0) {
			std::cout << "listen error: " << WSAGetLastError << std::endl;
			closesocket(serv_sock);
			WSACleanup();
			return -1;
		}

		std::cout << "listening started" << std::endl;

		sockaddr_in client_info;

		ZeroMemory(&client_info, sizeof(client_info));

		int client_info_size = sizeof(client_info);

		SOCKET client_conn = accept(serv_sock, (sockaddr*)(&client_info), &client_info_size);

		if (client_conn == INVALID_SOCKET) {
			std::cout << "connection failed: " << WSAGetLastError << std::endl;
			closesocket(serv_sock);
			closesocket(client_conn);
			WSACleanup();
			return -1;
		}

		int buff_size = 1024;
		std::vector<char> serv_buf(buff_size);
		std::vector<char> client_buf(buff_size);
		short packet_size;


		while (true) {
			packet_size = recv(client_conn, serv_buf.data(), serv_buf.size(), 0);
			std::cout << "client message: " << serv_buf.data() << std::endl;

			std::string serv_message;

			std::cout << "enter yout message: " << std::endl;

			std::cin >> serv_message;

			for (int i = 0; i < min((size_t)buff_size - 1, serv_message.size()); i++) {
				client_buf[i] = serv_message[i];
			}
			client_buf[min((size_t)buff_size, serv_message.size())] = '\0';

			packet_size = send(client_conn, client_buf.data(), client_buf.size(), 0);

			if (packet_size == SOCKET_ERROR) {
				std::cout << "SOCKET_ERROR" << std::endl;
			}
		}

		closesocket(serv_sock);
		closesocket(client_conn);
		WSACleanup();
	}
	else {
		std::cout << "WSAStartup error" << std::endl;

		return -1;
	}

	return 0;
}

