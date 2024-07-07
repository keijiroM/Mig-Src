// Source
#include <string>
#include <chrono>



struct RunTime {
	std::chrono::system_clock::time_point start, end;
	double time = 0.0;
};



double ReturnRunTime(const std::chrono::system_clock::time_point& start, 
					 const std::chrono::system_clock::time_point& end);
int FileOpen(const std::string& file_path);
