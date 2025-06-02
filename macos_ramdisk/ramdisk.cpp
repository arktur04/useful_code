#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstdio>
#include <string>
#include <unistd.h>

// RAM disk size in megabytes
constexpr int RAMDISK_SIZE_MB = 16;
const std::string VOLUME_NAME = "RAMDisk";
const std::string MOUNT_PATH = "/Volumes/" + VOLUME_NAME;

// Get the device ID for the RAM disk (e.g., /dev/disk3)
std::string create_ramdisk_device(int size_mb) {
    int blocks = size_mb * 1024 * 1024 / 512;
    std::string cmd = "hdiutil attach -nomount ram://" + std::to_string(blocks);

    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) return "";

    char buffer[128];
    std::string result;
    while (fgets(buffer, sizeof(buffer), pipe)) {
        result += buffer;
    }

    pclose(pipe);
    // Remove trailing spaces and newlines
    result.erase(result.find_last_not_of(" \n\r\t") + 1);
    std::cout << "Created RAM disk device: " << result << std::endl;
    // Ensure the result is a valid device path
    if (result.empty() || result.find("/dev/disk") == std::string::npos) {
        std::cerr << "Failed to create RAM disk device." << std::endl;
        return "";
    }

    return result;
}

// Format and mount the RAM disk as a volume
bool mount_ramdisk(const std::string& device, const std::string& volume_name) {
    std::string cmd = "diskutil erasevolume HFS+ " + volume_name + " " + device +
                      " >/dev/null 2>&1";
    return system(cmd.c_str()) == 0;
}

// Unmount and eject the RAM disk
bool eject_ramdisk(const std::string& mount_path) {
    std::string cmd = "diskutil eject \"" + mount_path + "\" >/dev/null 2>&1";
    return system(cmd.c_str()) == 0;
}

// Check if the RAM disk exists in /Volumes
bool is_ramdisk_mounted(const std::string& mount_path) {
    return access(mount_path.c_str(), F_OK) == 0;
}

void write_and_read_file(const std::string& path) {
    std::string file_path = path + "/tempfile.txt";

    std::ofstream ofs(file_path);
    if (!ofs) {
        std::cerr << "Error writing to " << file_path << std::endl;
        return;
    }
    ofs << "Hello from RAM!" << std::endl;
    ofs.close();

    std::ifstream ifs(file_path);
    std::string line;
    std::getline(ifs, line);
    std::cout << "Read: " << line << std::endl;
}

int main() {
    std::cout << "Creating RAM disk..." << std::endl;
    std::string device = create_ramdisk_device(RAMDISK_SIZE_MB);
    if (device.empty()) {
        std::cerr << "Failed to create RAM disk device!" << std::endl;
        return 1;
    }

    std::cout << "Mounting RAM disk..." << std::endl;
    if (!mount_ramdisk(device, VOLUME_NAME)) {
        std::cerr << "Failed to mount RAM disk!" << std::endl;
        return 1;
    }

    // Wait a moment for the volume to appear in /Volumes
    for (int i = 0; i < 10; ++i) {
        if (is_ramdisk_mounted(MOUNT_PATH)) break;
        usleep(100000); // 100 ms
    }

    // Check if the RAM disk is mounted
    if (!is_ramdisk_mounted(MOUNT_PATH)) {
        std::cerr << "RAM disk not found in /Volumes!" << std::endl;
        return 1;
    }

    std::cout << "RAM disk mounted at path: " << MOUNT_PATH << std::endl;
    write_and_read_file(MOUNT_PATH);

    // Unmount the RAM disk
    std::cout << "Ejecting RAM disk..." << std::endl;
    if (eject_ramdisk(MOUNT_PATH)) {
        std::cout << "RAM disk ejected." << std::endl;
    } else {
        std::cerr << "Error ejecting RAM disk!" << std::endl;
        return 1;
    }

    // Check if the RAM disk was successfully ejected
    if (is_ramdisk_mounted(MOUNT_PATH)) {
        std::cerr << "RAM disk still exists in /Volumes!" << std::endl;
        return 1;
    }

    std::cout << "RAM disk successfully removed." << std::endl;

    return 0;
}
