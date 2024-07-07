// Source
#include <iostream>
#include <thread>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "./sst.h"
#include "./socket.h"

using namespace rocksdb;


static void StartInstance(const std::string& id, const int& rate_of_workload, const int& number_of_threads) {
	std::cout << "Source Instance is started." << std::endl;

	
	// ソケットをオープン
	SocketFD socket_fd;
	OpenSocket(id, number_of_threads, socket_fd);

	// DBのパスを指定
	const std::string db_path = "/mig_inst" + id;
#ifdef ONLY_PMEM
	const std::string pmem_db_path = pmem_db_dir + db_path;
	const std::vector<std::string> dbname = {pmem_db_path};
#elif ONLY_DISK
	const std::string disk_db_path = disk_db_dir + db_path;
	const std::vector<std::string> dbname = {disk_db_path};
#else
	const std::string pmem_db_path = pmem_db_dir + db_path;
	const std::string disk_db_path = disk_db_dir + db_path;
	const std::vector<std::string> dbname = {pmem_db_path, disk_db_path};
#endif


	// Optionsを設定
	Options options;
	options.IncreaseParallelism();
	options.create_if_missing = true;
	// options.write_buffer_size = 64 << 20;
#ifdef MIXING
	options.db_paths = {{dbname[0], PMEMSIZE}, {dbname[1], DISKSIZE}};
#endif

	// DBを開く
	DB* db;
	Status status = DB::Open(options, dbname[0], &db);
	AssertStatus(status, "DB::Open");
	std::cout << "dbname: " << dbname[0] << std::endl;


/*-------------------------------------- Phase 1 --------------------------------------*/


	// SSTableメタデータを抽出
	std::vector<SstFileData> file_datas;
	std::vector<std::string> file_paths;
	int number_of_files;
	GetSstFileData(db, options, file_datas, file_paths, number_of_files);
	SendSstFileData(socket_fd.main_fd, file_datas);
	// std::cout << "number_of_files: " << number_of_files << std::endl;

	// OptionsをDSTに送信して，SRCでのDB作成完了の通知を受信
	SrcOptions src_options(options.write_buffer_size, number_of_files);
	SendOptions(socket_fd.main_fd, src_options);

	// DSTのDB作成完了を待機
	RecvFlag(socket_fd.main_fd, "db_creation");
	std::cout << "DB creation is completed on the destination." << std::endl;


/*-------------------------------------- Phase 2 --------------------------------------*/


	// SSTables転送スレッドに渡す引数を初期化
	TransferSstFilesArgs args(options, 
							  file_datas, 
							  file_paths, 
							  number_of_files,
							  number_of_threads, 
							  socket_fd.sst_fd, 
							  id);

	// SSTables転送スレッドを実行
	std::thread TransferSstFilesThread([&]{ TransferSstFiles(args); });
	TransferSstFilesThread.join();


/*-------------------------------------- Phase 3 --------------------------------------*/


	CloseSocket(number_of_threads, socket_fd);

	std::cout << "ok" << std::endl;
	

	delete db;
}



int main(int argc, char* argv[]) {
	if (argc != 4) {
		std::cerr << "argument error" << std::endl;
		return -1;
	}

	const std::string id = argv[1];
	int rate_of_workload = atoi(argv[2]);
	int number_of_threads = atoi(argv[3]);

	StartInstance(id, rate_of_workload, number_of_threads);

    return 0;
}
