#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <atomic>
#include <mutex>
#include <regex>

using namespace std;
using namespace chrono;

//print usage information
void print_usage() {
    std::cout << "Usage:\n"
    << "  -jN       Number of threads to run concurrently (default: 1)\n"
    << "  -b=N[KMG] Buffer size with optional unit (K, M, or G). Default is 1G.\n"
    << "  -nN       Number of iterations to perform (default: 10)\n";
}

// Parse buffer size string with suffix (e.g., "2G", "512M", "128K")
size_t parse_size(const string& str) {
    regex re(R"(^(\d+)([KMG]?)$)", regex::icase);
    smatch match;
    if (!regex_match(str, match, re)) {
        //throw invalid_argument("Invalid buffer size format.");
        throw invalid_argument("");
    }
    size_t base = stoull(match[1]);
    string suffix = match[2].str();
    if (suffix == "K" || suffix == "k") return base * (1ULL << 10);
    if (suffix == "M" || suffix == "m") return base * (1ULL << 20);
    if (suffix == "G" || suffix == "g") return base * (1ULL << 30);
    return base;
}
// Command-line argument parser
std::tuple<int, int, size_t> parse_args(int argc, char* argv[]) {
    int num_threads = 1;
    size_t buffer_size = 1ULL << 30; // 1GB
    int num_iterations = 10;

    for (int i = 1; i < argc; ++i) {
        string arg = argv[i];
        if (arg.rfind("-j", 0) == 0) {
            num_threads = stoi(arg.substr(2));
        } else if (arg.rfind("-b=", 0) == 0) {
            try {
                buffer_size = parse_size(arg.substr(3));
            }
            catch(...) {
                cerr << "Invalid buffer size format." << endl;
                goto exit;
            }
        } else if (arg.rfind("-n", 0) == 0) {
            num_iterations = stoi(arg.substr(2));
        } else {
            cerr << "Unknown argument: " << arg << endl;
            goto exit;
        }
    }
    return {num_threads, num_iterations, buffer_size};
    exit:
    print_usage();
    exit(1);
}

// Single-threaded RAM test (for one thread)
void ram_test(int thread_id, size_t buffer_size, int iterations,
              double& write_speed_out, double& read_speed_out)
{
    void* buf_ptr;
    if (posix_memalign(&buf_ptr, 64, buffer_size) != 0) {
        cerr << "Thread " << thread_id << ": Memory allocation failed." << endl;
        return;
    }
    uint8_t* test_buf = static_cast<uint8_t*>(buf_ptr);
    memset(test_buf, 0xAA, buffer_size);
    auto start_write = high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        for (size_t j = 0; j < buffer_size; ++j) {
            test_buf[j] = static_cast<uint8_t>(j);
        }
    }
    auto end_write = high_resolution_clock::now();
    double write_time = duration<double>(end_write - start_write).count();
    write_speed_out = (buffer_size * iterations) / (1024.0 * 1024.0 * write_time);
    // Sequential block reading test
    memset(test_buf, 0x55, buffer_size);
    volatile uint8_t sink = 0;
    auto start_read = high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        for (size_t j = 0; j < buffer_size; j += 64) {
            uint64_t* ptr = reinterpret_cast<uint64_t*>(&test_buf[j]);
            sink ^= static_cast<uint8_t>(
                ptr[0] ^ ptr[1] ^ ptr[2] ^ ptr[3] ^ ptr[4] ^ ptr[5] ^ ptr[6] ^ ptr[7]
            );
        }
    }
    auto end_read = high_resolution_clock::now();
    double read_time = duration<double>(end_read - start_read).count();
    read_speed_out = (buffer_size * iterations) / (1024.0 * 1024.0 * read_time);
    // in case of "sink" is optimized out and the test shows wrong (too high) values,
    // use "sink" for output the following:
    //    cout << "Thread " << thread_id << " checksum (ignore): " << static_cast<int>(sink) << endl;
    free(buf_ptr);
}

int main(int argc, char* argv[]) {
    auto [num_threads, num_iterations, buffer_size] = parse_args(argc, argv);

    cout << "Running with " << num_threads << " thread(s), "
         << (buffer_size >> 20) << " MB buffer per thread, "
         << num_iterations << " iteration(s)" << endl;

    vector<thread> threads;
    vector<double> write_speeds(num_threads, 0.0);
    vector<double> read_speeds(num_threads, 0.0);

    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back(ram_test, i, buffer_size, num_iterations,
                             ref(write_speeds[i]), ref(read_speeds[i]));
    }

    for (auto& t : threads) {
        t.join();
    }

    double total_write = 0.0, total_read = 0.0;
    for (int i = 0; i < num_threads; ++i) {
        total_write += write_speeds[i];
        total_read += read_speeds[i];
    }

    cout << "\n=== Aggregate Results ===" << endl;
    cout << "Total Write Speed: " << total_write << " MB/s" << endl;
    cout << "Total Read Speed:  " << total_read  << " MB/s" << endl;

    return 0;
}
