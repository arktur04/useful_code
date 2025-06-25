#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstdio>
#include <string>
#include <unistd.h>

// RAM disk size in megabytes
constexpr int RAMDISK_SIZE_MB = 2047;
const std::string VOLUME_NAME = "RAMDisk";
const std::string MOUNT_PATH = "/Volumes/" + VOLUME_NAME;

// Get the device ID for the RAM disk (e.g., /dev/disk3)
std::string create_ramdisk_device(int size_mb, bool verbose) {
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
    if (verbose) {
        std::cout << "Created RAM disk device: " << result << std::endl;
    }
    // Ensure the result is a valid device path
    if (result.empty() || result.find("/dev/disk") == std::string::npos) {
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

void write_and_read_file(const std::string& file_path) {
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

constexpr int FAILED_CREATE = 1; //Failed to create RAM disk device 
constexpr int FAILED_MOUNT = 2; //Failed to mount RAM disk
constexpr int DISK_NOT_FOUND = 3; //RAM disk not found in /Volumes

int create_ramdisk(bool verbose) {
    if (verbose) {
        std::cout << "Creating RAM disk..." << std::endl;
    }
    // create a device in RAM
    std::string device = create_ramdisk_device(RAMDISK_SIZE_MB, verbose);
    if (device.empty()) {
        return FAILED_CREATE;
    }
    if (verbose) {
        std::cout << "Mounting RAM disk..." << std::endl;
    }
    if (!mount_ramdisk(device, VOLUME_NAME)) {
        return FAILED_MOUNT;
    }
    // Wait a moment for the volume to appear in /Volumes
    for (int i = 0; i < 10; ++i) {
        if (is_ramdisk_mounted(MOUNT_PATH)) break;
        usleep(100000); // 100 ms
    }
    // Check if the RAM disk is mounted
    if (!is_ramdisk_mounted(MOUNT_PATH)) {
        return DISK_NOT_FOUND;
    }
    if (verbose) {
        std::cout << "RAM disk mounted at path: " << MOUNT_PATH << std::endl;
    }
    return 0;
}

constexpr int ERROR_EJECTING = 1; //Error ejecting RAM disk
constexpr int RAMDISK_STILL_EXISTS = 2; //RAM disk still exists in /Volumes

int unmount_ramdisk(bool verbose) {
    // Unmount the RAM disk
    if(verbose) {
        std::cout << "Ejecting RAM disk..." << std::endl;
    }
    if (eject_ramdisk(MOUNT_PATH)) {
        if(verbose) {
            std::cout << "RAM disk ejected." << std::endl;
        }
    } else {
        return ERROR_EJECTING;
    }
    // Check if the RAM disk was successfully ejected
    if (is_ramdisk_mounted(MOUNT_PATH)) {
        return RAMDISK_STILL_EXISTS;
    }
    if (verbose) {
        std::cout << "RAM disk successfully removed." << std::endl;
    }
    return 0;
}

int main() {
    bool verbose = false;
    bool ramdisk = true;
    std::string mount_path;
    if (ramdisk) {
        int err = create_ramdisk(verbose);
        switch (err) {
            case FAILED_CREATE: //Failed to create RAM disk device
            std::cerr << "Failed to create RAM disk device!" << std::endl;
            return 1;
            case FAILED_MOUNT: //Failed to mount RAM disk
            std::cerr << "Failed to mount RAM disk!" << std::endl;
            return 1;
            case DISK_NOT_FOUND: //RAM disk not found in /Volumes
            std::cerr << "RAM disk not found in /Volumes!" << std::endl;
            return 1;
        }
        mount_path = MOUNT_PATH;
    }
    else {
        mount_path = ".";
    }
    //===================
    const std::string TMPFILE_NAME = "tempfile.txt";

    write_and_read_file(mount_path + "/" + TMPFILE_NAME);
    //===================
    if (ramdisk) {
        int err = unmount_ramdisk(verbose);
        switch (err) {
            case ERROR_EJECTING: //Error ejecting RAM disk
                std::cerr << "Error ejecting RAM disk" << std::endl;
                return 1;
            case RAMDISK_STILL_EXISTS: //RAM disk still exists in /Volumes
                std::cerr << "RAM disk still exists in /Volumes" << std::endl;
                return 1;
        }
    }
    else {
        if(verbose) {
            std::cout << "A temporary file removing..." << std::endl;
        }
        int err = remove((mount_path + "/" + TMPFILE_NAME).c_str());
        if (err) {
            std::cerr << "An error occured during deleting the temporary file" << std::endl;
            return 1;
        }
        if(verbose) {
            std::cout << "The temporary file removed successfully." << std::endl;
        }
    }
    return 0;
}
