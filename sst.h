// Source
#include "./concurrentqueue.h"
#include "./db.h"
#include "rocksdb/options.h"

using namespace rocksdb;


struct TransferSstFilesArgs {
	Options options;
	std::vector<SstFileData> file_datas;
	std::vector<std::string> file_paths;
	int number_of_files;
	int number_of_threads;
	std::vector<int> sst_fd;
	std::string id;

	TransferSstFilesArgs(const Options& options_, 
						 const std::vector<SstFileData>& file_datas_, 
						 const std::vector<std::string>& file_paths_, 
						 const int number_of_files_, 
						 const int number_of_threads_, 
						 const std::vector<int>& sst_fd_, 
						 const std::string& id_) {
		options = options_;
		file_datas = file_datas_;
		file_paths = file_paths_;
		number_of_files = number_of_files_;
		number_of_threads = number_of_threads_;
		sst_fd = sst_fd_;
		id = id_;
	}
};



class SstFile {
private:

public:
	void ExportKVPairs(const Options& options, 
					   const std::string& file_path, 
					   moodycamel::ConcurrentQueue<std::string>& key_queue, 
					   moodycamel::ConcurrentQueue<std::string>& value_queue);
	void SendKeys(const int& fd, 
				  const uint64_t& number_of_entries, 
				  moodycamel::ConcurrentQueue<std::string>& key_queue);
	void SendValues(const int& fd, 
					const uint64_t& number_of_entries, 
					moodycamel::ConcurrentQueue<std::string>& value_queue);
};



void TransferSstFiles(TransferSstFilesArgs args);
