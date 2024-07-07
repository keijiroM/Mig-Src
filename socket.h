// Source
#include <string>
#include <vector>



struct SocketFD {
	int main_socket_fd;
	int main_fd;
#ifndef IDLE
	int kv_socket_fd;
	int kv_fd;
#endif
	std::vector<int> sst_socket_fd;
	std::vector<int> sst_fd;
};



// void OpenSocket(const std::string& id, SocketFD& socket_fd);
void OpenSocket(const std::string& id, const int& number_of_threads, SocketFD& socket_fd);
// void CloseSocket(const SocketFD& socket_fd);
void CloseSocket(const int& number_of_threads, const SocketFD& socket_fd);
void SendFlag(const int& fd, const short& flag);
short RecvFlag(const int& fd, const std::string& flag_name);
