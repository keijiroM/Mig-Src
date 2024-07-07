// Source
#include <iostream>
// #include <unistd.h>
#include <fcntl.h> // for debug
#include "./debug.h"



double ReturnRunTime(const std::chrono::system_clock::time_point& start, const std::chrono::system_clock::time_point& end) {
	return static_cast<double>(std::chrono::duration_cast<std::chrono::microseconds>(end - start).count());
}


int FileOpen(const std::string& file_path) {
	int file_fd = open(file_path.c_str(), O_CREAT | O_WRONLY | O_TRUNC , 00644);

	if (file_fd < 0) {
		std::cerr << "open()" << std::endl;
		exit(EXIT_FAILURE);
	}

	return file_fd;
}
