#include <iostream>
#include <fstream>
#include <chrono>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <cstring>
#include <vector>

struct Options {
    int s = 1; // MS_SYNC by default
    int n = 1; // MS_NOCACHE=1 by default
    bool help = false;
};

void print_help(const char* program_name) {
    std::cout << "Usage: " << program_name << " [-s=0|1] [-n=0|1] [-h]\n"
              << "  -s=0     Use MS_ASYNC\n"
              << "  -s=1     Use MS_SYNC (default)\n"
              << "  -n=0     F_NOCACHE=0\n"
              << "  -n=1     F_NOCACHE=1 (default)\n"
              << "  -h       Show this help message\n";
}

Options parse_args(int argc, char* argv[]) {
    Options opts;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "-h") {
            opts.help = true;
        } else if (arg.rfind("-s=", 0) == 0) {
            opts.s = std::stoi(arg.substr(3));
            if (opts.s != 0 && opts.s != 1) {
                std::cerr << "Invalid value for -s: " << arg << "\n";
                opts.help = true;
            }
        } else if (arg.rfind("-n=", 0) == 0) {
            opts.n = std::stoi(arg.substr(3));
            if (opts.n != 0 && opts.n != 1) {
                std::cerr << "Invalid value for -n: " << arg << "\n";
                opts.help = true;
            }
        } else {
            std::cerr << "Unknown argument: " << arg << "\n";
            opts.help = true;
        }
    }

    return opts;
}

constexpr size_t BUFFER_SIZE = 1 * 1024 * 1024; // 1 MB

void benchmark(size_t totalSizeMB, bool ms_sync, int f_nocache) {
    std::string filename = "test_mmap_file.bin";
    size_t totalSize = totalSizeMB * 1024 * 1024;

    // Open a file ans set zero size
    int fd = open(filename.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        perror("open");
        return;
    }

    // Caching off
    if (fcntl(fd, F_NOCACHE, f_nocache) < 0) {
        perror("fcntl F_NOCACHE");
        exit(1);
    }

    if (ftruncate(fd, totalSize) != 0) {
        perror("ftruncate");
        close(fd);
        return;
    }

    // mmap
    void* map = mmap(nullptr, totalSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (map == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return;
    }

    std::vector<uint8_t> buffer(BUFFER_SIZE);
    for (size_t i = 0; i < BUFFER_SIZE; ++i) buffer[i] = i % 256;

    size_t count = totalSize / BUFFER_SIZE;

    // --- Write + msync ---
    auto writeStart = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < count; ++i) {
        uint8_t* curr_p = static_cast<uint8_t*>(map) + i * BUFFER_SIZE;
        std::memcpy(curr_p, buffer.data(), BUFFER_SIZE);
        // Synchronizing
        if (msync(curr_p, BUFFER_SIZE, ms_sync ? MS_SYNC : MS_ASYNC) != 0) {
            perror("msync");
        }
    }

    auto writeEnd = std::chrono::high_resolution_clock::now();
    munmap(map, totalSize);
    close(fd);

    // --- Read ---
    // Open a file
    fd = open(filename.c_str(), O_RDWR, 0644);
    if (fd < 0) {
        perror("open");
        return;
    }

    // Caching off
    if (fcntl(fd, F_NOCACHE, f_nocache) < 0) {
        perror("fcntl F_NOCACHE");
        exit(1);
    }

    // mmap
    map = mmap(nullptr, totalSize, PROT_READ, MAP_SHARED, fd, 0);
    if (map == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return;
    }

    uint64_t checksum = 0;
    auto readStart = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < count; ++i) {
        uint8_t* ptr = static_cast<uint8_t*>(map) + i * BUFFER_SIZE;
        for (size_t j = 0; j < BUFFER_SIZE; ++j) {
            checksum += ptr[j];
        }
    }
    auto readEnd = std::chrono::high_resolution_clock::now();

    // Time and speed
    auto writeTime = std::chrono::duration<double>(writeEnd - writeStart).count();
    auto readTime = std::chrono::duration<double>(readEnd - readStart).count();

    std::cout << "Size: " << totalSizeMB << " MB\n";
    std::cout << "Write+msync: " << (totalSize / (1024.0 * 1024.0)) / writeTime << " MB/s\n";
    std::cout << "Read       : " << (totalSize / (1024.0 * 1024.0)) / readTime << " MB/s\n";
    std::cout << "Checksum   : " << checksum << "\n\n";

    munmap(map, totalSize);
    close(fd);
    unlink(filename.c_str());
}

int main(int argc, char* argv[]) {
    Options options = parse_args(argc, argv);

    if (options.help) {
        print_help(argv[0]);
        return 0;
    }

    std::cout << "Using MS_" << (options.s ? "" : "A") << "SYNC\n";
    std::cout << "F_NOCACHE=" << options.n << "\n";
    std::vector<size_t> sizes = {100, 512, 1024, 2048, 4096, 8192
        //, 12288
        };
    for (size_t sz : sizes) {
        benchmark(sz, options.s, options.n);
    }
    return 0;
}
