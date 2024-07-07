// Source
#include <iostream>
#include <thread>
#include <sys/socket.h>
#include "./sst.h"
#include "./socket.h"
#include "./debug.h"
#include "rocksdb/sst_file_reader.h"
// #include <unistd.h> // for debug


RunTime export_time, send_key_time, send_value_time;



// int count = 0;
void SstFile::ExportKVPairs(const Options& options, 
							const std::string& file_path, 
							moodycamel::ConcurrentQueue<std::string>& key_queue, 
							moodycamel::ConcurrentQueue<std::string>& value_queue) {
	SstFileReader sst_file_reader(options);
	Status status = sst_file_reader.Open(file_path);
	AssertStatus(status, "EXportKVPairs(Open)");

	Iterator* iter = sst_file_reader.NewIterator(ReadOptions());


	export_time.start = std::chrono::system_clock::now();	// for debug


	for (iter->SeekToFirst(); iter->Valid(); iter->Next()) {
		const std::string key   = iter->key().ToString();
		const std::string value = iter->value().ToString();

		// export_time.end = std::chrono::system_clock::now();

		key_queue.enqueue(key);
		value_queue.enqueue(value);
	}


	// for debug
	export_time.end = std::chrono::system_clock::now();
	export_time.time += ReturnRunTime(export_time.start, export_time.end);


	delete iter;


	// std::cout << "count: " << count << std::endl;
	// ++count;
}



void SstFile::SendKeys(const int& fd, 
					   const uint64_t& number_of_entries, 
					   moodycamel::ConcurrentQueue<std::string>& key_queue) {
	for (uint64_t i = 0; i < number_of_entries; ++i) {
		std::string data;
		while (!key_queue.try_dequeue(data)) {
            std::this_thread::yield();
        }


		send_key_time.start = std::chrono::system_clock::now(); // for debug


		ssize_t send_size = send(fd, data.c_str(), data.size(), 0);
		if (send_size <= 0) {
			std::cerr << "send(SendKeys)" << std::endl;
			exit(EXIT_FAILURE);
		}


		//for debug
		send_key_time.end = std::chrono::system_clock::now();
		send_key_time.time += ReturnRunTime(send_key_time.start, send_key_time.end);
	}
}



void SstFile::SendValues(const int& fd, 
						 const uint64_t& number_of_entries, 
						 moodycamel::ConcurrentQueue<std::string>& value_queue) {
	for (uint64_t i = 0; i < number_of_entries; ++i) {
		std::string data;
		while (!value_queue.try_dequeue(data)) {
            std::this_thread::yield();
        }


		send_value_time.start = std::chrono::system_clock::now(); // for debug


		ssize_t send_size = send(fd, data.c_str(), data.size(), 0);
		if (send_size <= 0) {
			std::cerr << "send(SendValues)" << std::endl;
			exit(EXIT_FAILURE);
		}


		//for debug
		send_value_time.end = std::chrono::system_clock::now();
		send_value_time.time += ReturnRunTime(send_value_time.start, send_value_time.end);
	}
}



static void SendSstFiles(TransferSstFilesArgs args, const int& thread_number) {
	// int file_number = 0;
	int file_number = thread_number;

	while (file_number < args.number_of_files) {
		SstFile sst_file;
		moodycamel::ConcurrentQueue<std::string> key_queue;
		moodycamel::ConcurrentQueue<std::string> value_queue;

		std::thread ExportKVPairsThread([&]{ sst_file.ExportKVPairs(args.options, 
																	args.file_paths[file_number], 
																	std::ref(key_queue), 
																	std::ref(value_queue)); });
        std::thread SendKeysThread([&]{ sst_file.SendKeys(args.sst_fd[2*thread_number], 
														  args.file_datas[file_number].number_of_entries, 
														  std::ref(key_queue)); });
		std::thread SendValuesThread([&]{ sst_file.SendValues(args.sst_fd[1+2*thread_number], 
															  args.file_datas[file_number].number_of_entries, 
															  std::ref(value_queue)); });
		ExportKVPairsThread.join();
		SendKeysThread.join();
		SendValuesThread.join();


		std::cout << "file_number: " << file_number << std::endl;
		// ++file_number;
		file_number += args.number_of_threads;
	}
}



void TransferSstFiles(TransferSstFilesArgs args) {
	std::cout << "TransferSstFilesThread is started." << std::endl;


    // std::thread SendSstFilesThread([&]{ SendSstFiles(args); });
	// std::thread SendSstFilesThread(SendSstFiles, args);
	// SendSstFilesThread.join();


	std::vector<std::thread> threads;
	for (int i = 0; i < args.number_of_threads; ++i) {
		threads.emplace_back(SendSstFiles, args, i);
	}
	for (auto& t : threads) {
		t.join();
	}


	std::cout << "ExportKVPiars: " << export_time.time / 1000000 << " sec" << std::endl;
	std::cout << "SendKeys     : " << send_key_time.time / 1000000 << " sec" << std::endl;
	std::cout << "SendValues   : " << send_value_time.time / 1000000 << " sec" << std::endl;


	std::cout << "TransferSstFilesThread is ended." << std::endl;
}
