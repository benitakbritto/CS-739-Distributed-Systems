

#include <string>
#include <chrono>
#include <ctime>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <utime.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cmath>
#include <algorithm> 

#define DEBUG                 1                                     // for debugging
#define dbgprintf(...)        if (DEBUG) { printf(__VA_ARGS__); }   // for debugging
#define MAX_RETRY             5                                     // rpc retry
#define RETRY_TIME_START      1                                     // in seconds
#define RETRY_TIME_MULTIPLIER 2                                     // for rpc retry w backoff
#define LOCAL_CACHE_PREFIX    "/tmp/afs/"                           // location of local files
#define CHUNK_SIZE            1024                                  // for streaming
//#define SERVER_ADDR         "52.151.53.152:50051"                 // Server: VM1
//#define SERVER_ADDR         "20.69.154.6:50051"                   // Server: VM2
//#define SERVER_ADDR         "20.69.94.59:50051"                   // Server: VM3
#define SERVER_ADDR           "0.0.0.0:50051"                       // Server: self
#define PERFORMANCE           0                                     // set to 1 to run performant functions
#define CRASH_TEST                                                  //Remove to disable all crashes
#define SINGLE_LOG            1                                     // Turns on single log functionality

using namespace std;
using std::ifstream;
using std::ostringstream;


            bool FileExists(std::string path)
            {
                struct stat s;   
                return (stat(path.c_str(), &s) == 0); 
            }

            // For Crash Consistency
            // Log v1
            int checkModified_single_log(string path) {
                ifstream log;
                log.open("/tmp/afs/log", ios::in);
                if (log.is_open()) {
                    string line;
                    while (getline(log, line)) {
                        if (line == path)
                            return 1;
                    }
                }
                return 0;
            }

            // For Crash Consistency
            // Log v1
            void closeEntry_single_log(string path) {
                // Delete entry from log
                ifstream log;
                log.open("/tmp/afs/log", ios::in);
                ofstream newlog;
                newlog.open("/tmp/afs/newlog", ios::out | ios::trunc);
                if (log.is_open() && newlog.is_open()) {
                    string line;
                
                    while (getline(log, line)) {	
                        if (line != path) {
                            newlog << line << endl;     
                    }
                }
                    remove("/tmp/afs/log");
                    // If we crash here, we lose the log
                    rename("/tmp/afs/newlog", "/tmp/afs/log");
                }
            }

            // For Crash Consistency
            // Log v2
            int removePendingFile(string filename)
            {
                //dbgprintf("removePendingFile: Entering function\n");
                string command = "rm -f " + filename;
                //dbgprintf("removePendingFile: command %s\n", command.c_str());
                //dbgprintf("removePendingFile: Exiting function\n");
                return system(command.c_str());
            }
            
            // For Crash Consistency
            // Log v2
            string to_flat_file(string relative_path)
            {
                //dbgprintf("to_flat_file: Entering function\n");
                for (int i=0; i<relative_path.length(); i++)
                {
                    if (relative_path[i] == '/') {
                        relative_path[i] = '%';
                    }
                }
                string flat_file = LOCAL_CACHE_PREFIX + relative_path + ".tmp"; 
                //dbgprintf("to_flat_file: Exiting function\n");
                return flat_file;
            }

            // For Crash Consistency
            // Log v2
            string from_flat_file(string absolute_path)
            {
                //dbgprintf("from_flat_file: Entering function\n");

                int i = 0;
                while (i++<string(LOCAL_CACHE_PREFIX).length());

                for (; i<absolute_path.length(); i++)
                {
                    if (absolute_path[i] == '%') {
                        absolute_path[i] = '/';
                    }
                }
                //dbgprintf("from_flat_file: Exiting function\n");
                string rel_path = absolute_path.substr(string(LOCAL_CACHE_PREFIX).length(),absolute_path.length());
                return rel_path;
            }
            
            // For Crash Consistency
            // Log v2
            bool isFileModifiedv2(string rel_path)
            {
                return FileExists(to_flat_file(rel_path));
            }
            
            
// removes the '/' at the beginning
char * fs_relative_path(char * path)
{
    return path + 1;
}
            
// For Crash Consistency
// Log v1
void init_single_log() {
  
  ofstream log;
  log.open("/tmp/afs/log", ios::out | ios::trunc);
  
  //Close-log init implementation:
  // Handle edge case crash when switching from log to newlog
  /*ifstream check_log;
  check_log.open("/tmp/afs/log", ios::in);
  if (!check_log.is_open())
    rename("/tmp/afs/newlog", "/tmp/afs/log");

  ifstream log;
  log.open("/tmp/afs/log", ios::in);
  if (log.is_open()) {
    string line;
    while (getline(log, line)) {
      options.client->CloseFile(-1, line);
      ////dbgprintf("READ LOG ON INIT %s \n", line.c_str());
    }
  }*/
}

// For Crash Consistency
// Log v1
void write_single_log(const char *path){
  // Update log to say this file was modified
  ofstream log;
  log.open("/tmp/afs/log", ios::out | ios::app);
  if (log.is_open())
    log << fs_relative_path((char* ) path) << endl;
}

// For Crash Consistency
// Log v2
int createPendingFile(string rel_path)
{
	//dbgprintf("createPendingFile: Entering function\n");
    string command = "touch " + to_flat_file(rel_path);
    //dbgprintf("createPendingFile: command %s\n", command.c_str());
	//dbgprintf("createPendingFile: Exiting function\n");
    return system(command.c_str());
}

// For Crash Consistency
// Log v2
int init_multi_log()
{
	// This would close all dirty writes
	// //dbgprintf("init_multi_log: Entering function\n");
	// string pattern = string(LOCAL_CACHE_PREFIX) + "*.tmp";
	// vector<string> to_remove = glob(pattern.c_str());
	// //dbgprintf("init_multi_log: to_remove size = %ld\n", to_remove.size());
	// //dbgprintf("init_multi_log: pattern = %s\n", pattern.c_str());
	// for (auto file : to_remove)
	// {
	// 	//dbgprintf("init_multi_log: file = %s\n", file.c_str());
	// 	options.client->CloseFile(-1, from_flat_file(file));
	// }
	// //dbgprintf("init_multi_log: Exiting function\n");
    // return 0;
	// This would delete all .tmp files
	//dbgprintf("init_multi_log: Entering function\n");
	string command = "rm -rf /tmp/afs/*.tmp";
	if (system(command.c_str()) != 0)
	{
		//dbgprintf("init_multi_log: system() failed\n");
	}
	//dbgprintf("init_multi_log: Exiting function\n");
    return 0;
}
            
void flush() {
	sync();

	std::ofstream ofs("/proc/sys/vm/drop_caches");
	ofs << "3" << std::endl;
}
            
void getstats(string desc, double stats [100]) {
	
	std::sort(stats, stats+100);
	double min = stats[0];
	double max = stats[99];
	float avg = 0.0;
	float med = (stats[49] + stats[50]) / 2.0;
	float std_dev = 0.0;
	
	for (int i = 0; i < 100; i++) {
		avg += stats[i];
	}
	
	avg = avg / 100.0;
	
	for (int i = 0; i < 100; i++) {
		std_dev += pow((float)stats[i] - avg, 2);	
	}
	std_dev = sqrt(std_dev / 100.0);
	printf("%s min: %f\n", desc.c_str(), min);
	printf("%s max: %f \n", desc.c_str(), max);
	printf("%s avg: %f \n", desc.c_str(), avg);
	printf("%s med: %f \n", desc.c_str(), med);
	printf("%s std_dev: %f \n", desc.c_str(), std_dev);
}
            
int
main(int argc, char *argv[])
{


	struct timespec start_time;
	struct timespec end_time;
	double total_time_single_init [100] = {};
	double total_time_multi_init [100] = {};
	
	double total_time_single_write_0 [100] = {};
	double total_time_multi_write_0 [100] = {};
	double total_time_single_write_50 [100] = {};
	double total_time_multi_write_50 [100] = {};
	double total_time_single_write_100 [100] = {};
	double total_time_multi_write_100 [100] = {};
	
	double total_time_single_checkmodify_0 [100] = {};
	double total_time_multi_checkmodify_0 [100] = {};
	double total_time_single_checkmodify_50 [100] = {};
	double total_time_multi_checkmodify_50 [100] = {};
	double total_time_single_checkmodify_100 [100] = {};
	double total_time_multi_checkmodify_100 [100] = {};
	double total_time_single_checkmodify_not_in_log [100] = {};
	double total_time_multi_checkmodify_not_in_log [100] = {};
	
	double total_time_single_remove_0 [100] = {};
	double total_time_multi_remove_0 [100] = {};
	double total_time_single_remove_50 [100] = {};
	double total_time_multi_remove_50 [100] = {};
	double total_time_single_remove_100 [100] = {};
	double total_time_multi_remove_100 [100] = {};
	double total_time_single_remove_not_in_log [100] = {};
	double total_time_multi_remove_not_in_log [100] = {};
	

	init_single_log();
	init_multi_log();

	for (int n = 0; n < 100; n++) {

	//Set up long v1 logs
	string name = "hello.txt";
	for (int i = 0; i < 50; i++) {
		write_single_log(("/" + name + to_string(i)).c_str());
	}
	std::system("cp /tmp/afs/log /tmp/afs/log50");

	for (int i = 0; i < 50; i++) {
		write_single_log(("/" + name + to_string(50+i)).c_str());
	}
	std::system("cp /tmp/afs/log /tmp/afs/log100");

	//TEST INIT
	for (int i = 0; i < 100; i++) {
		write_single_log(("/" + name + to_string(i)).c_str());
		createPendingFile(name + to_string(i));
	}

	
	auto start = chrono::steady_clock::now();
	init_single_log();	
	auto end = chrono::steady_clock::now();
	total_time_single_init[n] = std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();

	
	start = chrono::steady_clock::now();
	init_multi_log();
	end = chrono::steady_clock::now();
	total_time_multi_init[n] = std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();

	//TEST WRITE
	for (int i = 0; i < 0; i++) {
		write_single_log(("/" + name + to_string(i)).c_str());
		createPendingFile(name + to_string(i));
	}
	flush();
	
	start = chrono::steady_clock::now();
	write_single_log("/write_test_0");	
	end = chrono::steady_clock::now();
	total_time_single_write_0[n] = std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();

	
	start = chrono::steady_clock::now();
	createPendingFile("write_test_0");
	end = chrono::steady_clock::now();
	total_time_multi_write_0[n] = std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();

	
	init_single_log();
	init_multi_log();
	
	for (int i = 0; i < 50; i++) {
		createPendingFile(name + to_string(i));
	}
	remove("/tmp/afs/log");
	std::system("cp /tmp/afs/log50 /tmp/afs/log");
	flush();
	
	start = chrono::steady_clock::now();
	write_single_log("write_test_50");	
	end = chrono::steady_clock::now();
	total_time_single_write_50[n] = std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();

	
	start = chrono::steady_clock::now();
	createPendingFile("/write_test_50");
	end = chrono::steady_clock::now();
	total_time_multi_write_50[n] = std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();

	
	init_single_log();
	init_multi_log();
	
	for (int i = 0; i < 100; i++) {
		createPendingFile(name + to_string(i));
	}
	remove("/tmp/afs/log");
	std::system("cp /tmp/afs/log100 /tmp/afs/log");
	flush();
	
	start = chrono::steady_clock::now();
	write_single_log("/write_test_100");	
	end = chrono::steady_clock::now();
	total_time_single_write_100[n] = std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();

	
	start = chrono::steady_clock::now();
	createPendingFile("write_test_100");
	end = chrono::steady_clock::now();
	total_time_multi_write_100[n] = std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();

	
	init_single_log();
	init_multi_log();
	
	//TEST CHECK MODIFIED
	for (int i = 0; i < 100; i++) {
		createPendingFile(name + to_string(i));
	}
	remove("/tmp/afs/log");
	std::system("cp /tmp/afs/log100 /tmp/afs/log");
	flush();
	
	start = chrono::steady_clock::now();
	checkModified_single_log("hello.txt0");
	end = chrono::steady_clock::now();
	total_time_single_checkmodify_0[n] = std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();

	
	start = chrono::steady_clock::now();
	isFileModifiedv2("hello.txt0");
	end = chrono::steady_clock::now();
	total_time_multi_checkmodify_0[n] = std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();

	
	remove("/tmp/afs/log");
	std::system("cp /tmp/afs/log100 /tmp/afs/log");
	flush();
	
	start = chrono::steady_clock::now();
	checkModified_single_log("hello.txt49");
	end = chrono::steady_clock::now();
	total_time_single_checkmodify_50[n] = std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();

	
	start = chrono::steady_clock::now();
	isFileModifiedv2("hello.txt49");
	end = chrono::steady_clock::now();
	total_time_multi_checkmodify_50[n] = std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();

	
	remove("/tmp/afs/log");
	std::system("cp /tmp/afs/log100 /tmp/afs/log");
	flush();
	
	start = chrono::steady_clock::now();
	checkModified_single_log("hello.txt99");
	end = chrono::steady_clock::now();
	total_time_single_checkmodify_100[n] = std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();

	
	start = chrono::steady_clock::now();
	isFileModifiedv2("hello.txt99");
	end = chrono::steady_clock::now();
	total_time_multi_checkmodify_100[n] = std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();

	
	remove("/tmp/afs/log");
	std::system("cp /tmp/afs/log100 /tmp/afs/log");
	flush();
	
	start = chrono::steady_clock::now();
	checkModified_single_log("not_in_log.txt");
	end = chrono::steady_clock::now();
	total_time_single_checkmodify_not_in_log[n] = std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();

	
	start = chrono::steady_clock::now();
	isFileModifiedv2("not_in_log.txt");
	end = chrono::steady_clock::now();
	total_time_multi_checkmodify_not_in_log[n] = std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();

	
	init_single_log();
	init_multi_log();
	
	//TEST REMOVE ENTRY
	for (int i = 0; i < 100; i++) {
		createPendingFile(name + to_string(i));
	}
	remove("/tmp/afs/log");
	std::system("cp /tmp/afs/log100 /tmp/afs/log");
	flush();
	
	start = chrono::steady_clock::now();
   	closeEntry_single_log("hello.txt0");
	end = chrono::steady_clock::now();
	total_time_single_remove_0[n] = std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();

	
	start = chrono::steady_clock::now();
   	removePendingFile(to_flat_file("hello.txt0"));
	end = chrono::steady_clock::now();
	total_time_multi_remove_0[n] = std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();

	
	remove("/tmp/afs/log");
	std::system("cp /tmp/afs/log100 /tmp/afs/log");
	flush();
	
	start = chrono::steady_clock::now();
   	closeEntry_single_log("hello.txt49");
	end = chrono::steady_clock::now();
	total_time_single_remove_50[n] = std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();

	
	start = chrono::steady_clock::now();
   	removePendingFile(to_flat_file("hello.txt49"));
	end = chrono::steady_clock::now();
	total_time_multi_remove_50[n] = std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();

	
	remove("/tmp/afs/log");
	std::system("cp /tmp/afs/log100 /tmp/afs/log");
	flush();
	
	start = chrono::steady_clock::now();
   	closeEntry_single_log("hello.txt99");
	end = chrono::steady_clock::now();
	total_time_single_remove_100[n] = std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();

	
	start = chrono::steady_clock::now();
   	removePendingFile(to_flat_file("hello.txt99"));
	end = chrono::steady_clock::now();
	total_time_multi_remove_100[n] = std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();

	
	remove("/tmp/afs/log");
	std::system("cp /tmp/afs/log100 /tmp/afs/log");
	flush();
	
	start = chrono::steady_clock::now();
   	closeEntry_single_log("not_in_lot.txt");
	end = chrono::steady_clock::now();
	total_time_single_remove_not_in_log[n] = std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();

	
	start = chrono::steady_clock::now();
   	removePendingFile(to_flat_file("not_in_log.txt"));
	end = chrono::steady_clock::now();
	total_time_multi_remove_not_in_log[n] = std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();

	
	init_single_log();
	init_multi_log();
	
	}
 
	int n = 0; 
	getstats("Single log init", total_time_single_init);
	getstats("Multi log init", total_time_multi_init);
	
	getstats("Single log write 0", total_time_single_write_0);
	getstats("Multi log write 0", total_time_multi_write_0);
	getstats("Single log write 50", total_time_single_write_50);
	getstats("Multi log write 50", total_time_multi_write_50);
	getstats("Single log write 100", total_time_single_write_100);
	getstats("Multi log write 100", total_time_multi_write_100);
	
	getstats("Single log check modify beginning", total_time_single_checkmodify_0);
	getstats("Multi log check modify beginning", total_time_multi_checkmodify_0);
	getstats("Single log check modify middle", total_time_single_checkmodify_50);	
	getstats("Multi log check modify middle", total_time_multi_checkmodify_50);	
	getstats("Single log check modify end", total_time_single_checkmodify_100);
	getstats("Multi log check modify end", total_time_multi_checkmodify_100);
	getstats("Single log check modify not in log", total_time_single_checkmodify_not_in_log);
	getstats("Multi log check modify not in log", total_time_multi_checkmodify_not_in_log);
	
	getstats("Single log remove beginning", total_time_single_remove_0);
	getstats("Multi log remove beginning", total_time_multi_remove_0);
	getstats("Single log remove middle", total_time_single_remove_50);
	getstats("Multi log remove middle", total_time_multi_remove_50);
	getstats("Single log remove end", total_time_single_remove_100);	
	getstats("Multi log remove end", total_time_multi_remove_100);
	getstats("Single log remove not in log", total_time_single_remove_not_in_log);
	getstats("Multi log remove not in log", total_time_multi_remove_not_in_log);

	
	/*init_single_log();
	write_single_log("/hello.txt");
   	dbgprintf("%d \n", checkModified_single_log("hello.txt"));
   	closeEntry_single_log("hello.txt");
   	
	init_multi_log();
	createPendingFile("hello.txt");   
   	dbgprintf("%d \n", isFileModifiedv2("hello.txt"));
   	removePendingFile(to_flat_file("hello.txt"));*/
   	
	return 0;
}