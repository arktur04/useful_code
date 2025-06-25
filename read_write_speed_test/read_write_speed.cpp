
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <cstring>
#include <iostream>
#include <vector>

double get_time_sec() {
    timeval tv;
    gettimeofday(&tv, nullptr);
    return tv.tv_sec + tv.tv_usec / 1e6;
}

void benchmark(int size_mb, const char* path) {
    const size_t block_size = 1024 * 1024;
    size_t total_size = size_mb * block_size;
    std::vector<char> buffer(block_size, 'A');

    // ==== Write ====
    int fd = open(path, O_CREAT | O_WRONLY, 0666);
    if (fd < 0) {
        perror("open write");
        return;
    }

    // Disable caching on macOS
    fcntl(fd, F_NOCACHE, 1);

    double write_start = get_time_sec();
    for (size_t written = 0; written < total_size; written += block_size) {
        if (write(fd, buffer.data(), block_size) != block_size) {
            perror("write");
            close(fd);
            return;
        }
    }
    fsync(fd);
    double write_end = get_time_sec();
    close(fd);

    double write_speed = size_mb / (write_end - write_start);

    // ==== Read ====
    fd = open(path, O_RDONLY);
    if (fd < 0) {
        perror("open read");
        return;
    }

    fcntl(fd, F_NOCACHE, 1);

    double read_start = get_time_sec();
    for (size_t read_bytes = 0; read_bytes < total_size; read_bytes += block_size) {
        if (read(fd, buffer.data(), block_size) != block_size) {
            perror("read");
            close(fd);
            return;
        }
    }
    double read_end = get_time_sec();
    close(fd);

    double read_speed = size_mb / (read_end - read_start);

    // ==== Output ====
    std::cout << "Size: " << size_mb << " MB | "
              << "Write: " << write_speed << " MB/s | "
              << "Read: " << read_speed << " MB/s\n";
}

int main() {
    const char* path = "/tmp/ssd_benchmark_test.dat";
    std::vector<int> sizes_mb = {100, 512, 1024, 2048, 4096, 8192, 12288};

    for (int size : sizes_mb) {
        benchmark(size, path);
    }

    unlink(path); // Clean up
    return 0;
}
