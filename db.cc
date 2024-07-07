// Source
#include <iostream>
// #include <sstream>
#include <sys/socket.h>
#include "./db.h"
#include "./socket.h"

using namespace rocksdb;



void AssertStatus(Status status, const std::string& flag_name) {
	if (!status.ok()) {
		std::cout << status.ToString() << "("  + flag_name + ")" << std::endl;
		exit(EXIT_FAILURE);
    }
}



void GetSstFileData(DB* db, const Options& options, std::vector<SstFileData>& file_datas, std::vector<std::string>& file_paths, int& number_of_files) {
	std::vector<LiveFileMetaData> live_file_metadata;
	db->GetLiveFilesMetaData(&live_file_metadata);

	for (const auto& meta : live_file_metadata) {
		SstFileData file_data;

		// on_disk
#ifdef ONLY_PMEM
		file_data.on_oisk = false;
#elif ONLY_DISK
		file_data.on_disk = true;
#else
		if (meta.db_path == options.db_paths[0].path)
			file_data.on_disk = false;
		else
			file_data.on_disk = true;
#endif
		// level
		file_data.level = meta.level;

		// number_of_entries
		file_data.number_of_entries = meta.num_entries;

		file_datas.push_back(file_data);

		// file_path
		const std::string file_path = meta.db_path + meta.name;
		file_paths.push_back(file_path);
	}

	number_of_files = file_datas.size();
}



void SendSstFileData(const int& fd, const std::vector<SstFileData>& file_datas) {
	// シリアライズ
	std::ostringstream oss;
	{
		cereal::JSONOutputArchive archive(oss);
		archive(file_datas);
	}
	std::string serialized_data = oss.str();
	
	// std::cout << serialized_data << std::endl;  // for debug

	size_t size = serialized_data.size();
	// std::cout << "size: " << size << std::endl; // for debug
	if (send(fd, &size, sizeof(size_t), 0) <= 0) {
		std::cerr << "send()" << std::endl;
		exit(EXIT_FAILURE);
	}
	
	RecvFlag(fd, "SendSstFileData");


	// 送信
	size_t total_size = 0;
	while (total_size < serialized_data.size()) {
		ssize_t send_size = send(fd, serialized_data.data()+total_size, 4096, 0);
		if (send_size < 0) {
            std::cerr << "send()" << std::endl;
            exit(EXIT_FAILURE);
		}
		total_size += send_size;
	}


	// if (send(fd, serialized_data.c_str(), serialized_data.size(), 0) <= 0) {
	//	std::cerr << "send()" << std::endl;
	//	exit(EXIT_FAILURE);
	// }

	RecvFlag(fd, "SendSstFileData");
}



void SendOptions(const int& fd, const SrcOptions& src_options) {
	if (send(fd, &src_options, sizeof(SrcOptions), 0) <= 0) {
		std::cerr << "send(SendOptions)" << std::endl;
		exit(EXIT_FAILURE);
	}
}
