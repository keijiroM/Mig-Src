// Source
#include <string>
#include <vector>
#include <cereal/archives/json.hpp>
#include <cereal/types/vector.hpp>
#include "rocksdb/db.h"

#define PMEMSIZE 16*1024*1024*1024ULL
#define DISKSIZE 128*1024*1024*1024ULL
#define KEYMAX   240*1024*1024
#define KEYLEN   9
#define VALUELEN 4*1024

using namespace rocksdb;


#ifndef _MYDB__
#define _MYDB__
const std::string pmem_dir = "/mnt/pmem0";
const std::string disk_dir = "/mnt/sdc0";

#ifdef ONLY_PMEM
const std::string pmem_db_dir = pmem_dir + "/only_pmem";
#elif ONLY_DISK
const std::string disk_db_dir = disk_dir + "/only_disk";
#else
const std::string pmem_db_dir = pmem_dir + "/mixing";
const std::string disk_db_dir = disk_dir + "/mixing";
#endif


struct SrcOptions {
	size_t write_buffer_size;
	int number_of_files;

	SrcOptions(size_t write_buffer_size, int number_of_files) {
		this->write_buffer_size = write_buffer_size;
		this->number_of_files = number_of_files;
	}
};


struct SstFileData {
	bool on_disk;
	int level;
	uint64_t number_of_entries;

	template<class Archive>
	void serialize(Archive &archive) {
		archive(on_disk, level, number_of_entries);
	}
};
#endif


void AssertStatus(Status status, const std::string& flag_name);
void GetSstFileData(DB* db, const Options& options, std::vector<SstFileData>& file_datas, std::vector<std::string>& file_paths, int& number_of_files);
void SendSstFileData(const int& fd, const std::vector<SstFileData>& file_datas);
void SendOptions(const int& fd, const SrcOptions& src_options);
