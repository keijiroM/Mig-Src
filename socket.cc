// Source
#include <iostream>
#include <cstring>
#include <thread>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "./socket.h"


static int ListenSocket(const int& port_number) {
	sockaddr_in saddr;
	int         socket_fd;

	if ((socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		std::cerr << "socket()" << std::endl;
		exit(EXIT_FAILURE);
	}

	memset(&saddr, 0, sizeof(saddr));
	saddr.sin_family      = AF_INET;
	saddr.sin_port        = htons(port_number);
	saddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(socket_fd, (sockaddr*)&saddr, sizeof(saddr)) < 0) {
		std::cerr << "bind()" << std::endl;
		close(socket_fd);
		exit(EXIT_FAILURE);
	}

	if (listen(socket_fd, SOMAXCONN) < 0) {
		std::cerr << "listen()" << std::endl;;
		close(socket_fd);
		exit(EXIT_FAILURE);
	}

	return socket_fd;
}



static int AcceptSocket(const int& socket_fd) {
	sockaddr_in caddr;
	int         fd;

	socklen_t len = sizeof(caddr);
	if ((fd = accept(socket_fd, (sockaddr*)&caddr, &len)) < 0) {
		std::cerr << "accept()" << std::endl;
		close(fd);
		close(socket_fd);
		exit(EXIT_FAILURE);
	}

	return fd;
}



// void OpenSocket(const std::string& id, SocketFD& socket_fd) {
void OpenSocket(const std::string& id, const int& number_of_threads, SocketFD& socket_fd) {
	int port_number = 20000 + (atoi(id.c_str()) * 100);

	socket_fd.main_socket_fd = ListenSocket(port_number++);
#ifndef IDLE
	socket_fd.kv_socket_fd = ListenSocket(port_number++);
#endif
	// for (int i = 0; i < 2; ++i) {
	for (int i = 0; i < 2*number_of_threads; ++i) {
		socket_fd.sst_socket_fd.emplace_back(ListenSocket(port_number++));
	}

	socket_fd.main_fd = AcceptSocket(socket_fd.main_socket_fd);
#ifndef IDLE
	socket_fd.kv_fd = AcceptSocket(socket_fd.kv_socket_fd);
#endif
	// for (int i = 0; i < 2; ++i) {
	for (int i = 0; i < 2*number_of_threads; ++i) {
		socket_fd.sst_fd.emplace_back(AcceptSocket(socket_fd.sst_socket_fd[i]));
	}
}



// void CloseSocket(const SocketFD& socket_fd) {
void CloseSocket(const int& number_of_threads, const SocketFD& socket_fd) {
	close(socket_fd.main_socket_fd);
	close(socket_fd.main_fd);
#ifndef IDLE
	close(socket_fd.kv_socket_fd);
	close(socket_fd.kv_fd);
#endif
	// for (int i = 0; i < 2; ++i) {
	for (int i = 0; i < 2*number_of_threads; ++i) {
		close(socket_fd.sst_socket_fd[i]);
		close(socket_fd.sst_fd[i]);
	}
}



void SendFlag(const int& fd, const short& flag) {
	send(fd, &flag, sizeof(short), 0);
}



short RecvFlag(const int& fd, const std::string& flag_name) {
	short flag = 0;

	if (recv(fd, &flag, sizeof(short), 0) < 0) {
		std::cerr << "recv(" + flag_name + ")" << std::endl;
		exit(EXIT_FAILURE);
	}

	return flag;
}
