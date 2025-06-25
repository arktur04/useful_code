#include <iostream>
#include <iomanip>
#include <vector>
#include <thread>
#include <chrono>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <atomic>
#include <mutex>
#include <regex>
#include <immintrin.h> // For AVX/SSE

using namespace std::chrono;
using namespace std;

// settings
struct settings {
    size_t buffer_size = 1ULL << 30; // 1 GB
    int num_iterations = 10;
    int num_threads = 1;
    bool use_simd = false;
 } settings;

// Parse buffer size string with suffix (e.g., "2G", "512M", "128K")
size_t parse_size(const std::string &str)
{
    std::regex re(R"(^(\d+)([KMG]?)$)", std::regex::icase);
    std::smatch match;
    if (!std::regex_match(str, match, re))
    {
        throw std::invalid_argument("Invalid buffer size format.");
    }
    size_t base = std::stoull(match[1]);
    std::string suffix = match[2].str();
    if (suffix == "K" || suffix == "k")
        return base * (1ULL << 10);
    if (suffix == "M" || suffix == "m")
        return base * (1ULL << 20);
    if (suffix == "G" || suffix == "g")
        return base * (1ULL << 30);
    return base;
}
/*
void ram_test(int thread_id, int iterations, double &write_speed_out,
    double &read_speed_out)
{
    auto memory_err = "Memory allocation failed.";
    void *buff_ptr;
    if (posix_memalign(&buff_ptr, 64, settings.buffer_size) != 0)
    {
        // memory allocation error
        std::cerr << memory_err << std::endl;
        return;
    }

    uint8_t *write_buf = static_cast<uint8_t *>(buff_ptr);

    memset(write_buf, 0xAA, settings.buffer_size);

    // Write benchmark
    auto start_write = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i)
    {
        for (size_t j = 0; j < settings.buffer_size; ++j)
        {
            write_buf[j] = static_cast<uint8_t>(j);
        }
    }
 
    auto end_write = std::chrono::high_resolution_clock::now();
    double write_time = std::chrono::duration<double>(end_write - start_write).count();
    write_speed_out = (settings.buffer_size * iterations) / (1024.0 * 1024.0 * write_time);

    // Read benchmark — realistic sequential read
    uint8_t *read_buf = static_cast<uint8_t *>(buff_ptr);

    memset(write_buf, 0xAA, settings.buffer_size);

    size_t checksum = 0;
    std::chrono::steady_clock::time_point start_read, end_read;
    if (settings.use_simd)
    {
        volatile uint8_t sink = 0;
        start_read = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < iterations; ++i)
        {
            __m256i acc = _mm256_setzero_si256();
            for (size_t j = 0; j < settings.buffer_size; j += 32)
            { // 32 bytes = 256 bits
                __m256i data = _mm256_load_si256(reinterpret_cast<const __m256i *>(&read_buf[j]));
                acc = _mm256_xor_si256(acc, data); // XOR withoun dependencies
            }
            // Calculate XOR sum, to avois a compiler optimization
            uint8_t *p = reinterpret_cast<uint8_t *>(&acc);
            for (int k = 0; k < 32; ++k)
                sink ^= p[k];
        }
        end_read = std::chrono::high_resolution_clock::now();
    }
    else
    {
        start_read = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < iterations; ++i)
        {
            for (size_t j = 0; j < settings.buffer_size; ++j)
            {
                checksum += read_buf[j];
            }
        }
        end_read = std::chrono::high_resolution_clock::now();
    }
    free(buff_ptr);
    double read_time = std::chrono::duration<double>(end_read - start_read).count();
    read_speed_out = (settings.buffer_size * iterations) / (1024.0 * 1024.0 * read_time);
    std::cout << "Thread " << thread_id << " checksum (ignore): " << checksum << std::endl;
} */
/*
void ram_test(int thread_id, int iterations,
    double& write_speed_out, double& read_speed_out)
{
void* write_ptr;
void* read_ptr;
if (posix_memalign(&write_ptr, 64, settings.buffer_size) != 0 ||
posix_memalign(&read_ptr, 64, settings.buffer_size) != 0) {
std::cerr << "Thread " << thread_id << ": Memory allocation failed." << std::endl;
return;
}

uint8_t* write_buf = static_cast<uint8_t*>(write_ptr);
uint8_t* read_buf = static_cast<uint8_t*>(read_ptr);

memset(write_buf, 0xAA, settings.buffer_size);
memset(read_buf, 0x55, settings.buffer_size);

// Write benchmark
auto start_write = std::chrono::high_resolution_clock::now();
for (int i = 0; i < iterations; ++i) {
for (size_t j = 0; j < settings.buffer_size; ++j) {
  write_buf[j] = static_cast<uint8_t>(j);
}
}
auto end_write = std::chrono::high_resolution_clock::now();
double write_time = std::chrono::duration<double>(end_write - start_write).count();
write_speed_out = (settings.buffer_size * iterations) / (1024.0 * 1024.0 * write_time);

// Read benchmark — realistic sequential read
size_t checksum = 0;
auto start_read = std::chrono::high_resolution_clock::now();
for (int i = 0; i < iterations; ++i) {
for (size_t j = 0; j < settings.buffer_size; ++j) {
  checksum += read_buf[j];
}
}
auto end_read = std::chrono::high_resolution_clock::now();
double read_time = std::chrono::duration<double>(end_read - start_read).count();
read_speed_out = (settings.buffer_size * iterations) / (1024.0 * 1024.0 * read_time);

std::cout << "Thread " << thread_id << " checksum (ignore): " << checksum << std::endl;

free(write_ptr);
free(read_ptr);
}
*/
//    ======= old version ========
// Single-threaded RAM test (for one thread)
void ram_test(int thread_id, size_t buffer_size, int iterations,
              double& write_speed_out, double& read_speed_out)
{
    void* write_ptr;
    void* read_ptr;
    if (posix_memalign(&write_ptr, 64, buffer_size) != 0 ||
        posix_memalign(&read_ptr, 64, buffer_size) != 0) {
        cerr << "Thread " << thread_id << ": Memory allocation failed." << endl;
        return;
    }

    uint8_t* write_buf = static_cast<uint8_t*>(write_ptr);
    uint8_t* read_buf = static_cast<uint8_t*>(read_ptr);

    memset(write_buf, 0xAA, buffer_size);
    memset(read_buf, 0x55, buffer_size);

    auto start_write = high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        for (size_t j = 0; j < buffer_size; ++j) {
            write_buf[j] = static_cast<uint8_t>(j);
        }
    }
    auto end_write = high_resolution_clock::now();
    double write_time = duration<double>(end_write - start_write).count();
    write_speed_out = (buffer_size * iterations) / (1024.0 * 1024.0 * write_time);

    // Sequential block read
    volatile uint8_t sink = 0;
    auto start_read = high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        for (size_t j = 0; j < buffer_size; j += 64) {
            uint64_t* ptr = reinterpret_cast<uint64_t*>(&read_buf[j]);
            sink ^= static_cast<uint8_t>(
                ptr[0] ^ ptr[1] ^ ptr[2] ^ ptr[3] ^ ptr[4] ^ ptr[5] ^ ptr[6] ^ ptr[7]
            );
        }
    }
    auto end_read = high_resolution_clock::now();
    double read_time = duration<double>(end_read - start_read).count();
    read_speed_out = (buffer_size * iterations) / (1024.0 * 1024.0 * read_time);

    cout << "Thread " << thread_id << " checksum (ignore): " << static_cast<int>(sink) << endl;

    free(write_ptr);
    free(read_ptr);
}

void print_usage()
{
    std::cout << "Usage:\n";
    std::cout << std::left;
    std::cout << "  " << std::setw(10) << "-jN"
              << "Number of threads to run concurrently (default: 1).\n";
    std::cout << "    " << std::setw(10) << ""
              << "Example: -j4 runs the test on 4 threads in parallel.\n\n";

    std::cout << "  " << std::setw(10) << "-b=N[KMG]"
              << "Buffer size per thread with optional unit (default: 1G).\n";
    std::cout << "    " << std::setw(10) << ""
              << "K=Kilobytes, M=Megabytes, G=Gigabytes.\n";
    std::cout << "    " << std::setw(10) << ""
              << "Example: -b512M sets buffer size to 512 Megabytes.\n\n";

    std::cout << "  " << std::setw(10) << "-nN"
              << "Number of iterations to perform (default: 10).\n";
    std::cout << "    " << std::setw(10) << ""
              << "More iterations improve measurement stability.\n\n";

    std::cout << "  " << std::setw(10) << "-s"
              << "Enable SIMD optimizations (default: disabled).\n";
    std::cout << "    " << std::setw(10) << ""
              << "May improve performance on supported CPUs.\n\n";

    std::cout << "Example:\n"
              << "  ./ram_speed_test -j4 -b=2G -n20 -s\n";
}

// Command-line argument parser
void parse_args(int argc, char *argv[])
{
    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];
        if (arg.rfind("-j", 0) == 0)
        {
            settings.num_threads = std::stoi(arg.substr(2));
        }
        else if (arg.rfind("-b=", 0) == 0)
        {
            settings.buffer_size = parse_size(arg.substr(3));
        }
        else if (arg.rfind("-n", 0) == 0)
        {
            settings.num_iterations = std::stoi(arg.substr(2));
        }
        else if (arg.rfind("-s", 0) == 0)
        {
            settings.use_simd = true;
        }
        else if (arg.rfind("-h", 0) == 0)
        {
            print_usage();
            exit(0);
        }
        else
        {
            std::cerr << "Unknown argument: " << arg << std::endl;
            print_usage();
            exit(1);
        }
    }
}

int main(int argc, char *argv[])
{
    parse_args(argc, argv);

    std::cout << "Running with " << settings.num_threads << " thread(s), "
              << (settings.buffer_size >> 20) << " MB buffer per thread, "
              << settings.num_iterations << " iteration(s)"
              << (settings.use_simd? ", SIMD reading": "") << std::endl;

    std::vector<std::thread> threads;
    std::vector<double> write_speeds(settings.num_threads, 0.0);
    std::vector<double> read_speeds(settings.num_threads, 0.0);

    for (int i = 0; i < settings.num_threads; ++i)
    {
        threads.emplace_back(ram_test, i, settings.num_iterations, settings.buffer_size,
                             std::ref(write_speeds[i]), std::ref(read_speeds[i]));
    }

    for (auto &t : threads)
    {
        t.join();
    }

    double total_write = 0.0, total_read = 0.0;
    for (int i = 0; i < settings.num_threads; ++i)
    {
        total_write += write_speeds[i];
        total_read += read_speeds[i];
    }

    std::cout << "\n=== Aggregate Results ===" << std::endl;
    std::cout << "Total Write Speed: " << total_write << " MB/s" << std::endl;
    std::cout << "Total Read Speed:  " << total_read << " MB/s" << std::endl;

    return 0;
}
