#include "managers/audioManager.h"
audioManager::audioManager() {
    int rmdir = system("rmdir /mnt/sdcard/audioDataTemp && mkdir /mnt/sdcard/audioDataTemp");
    int mount_sd = system("sudo mount /dev/mmcblk1p1 /mnt/sdcard");

    if (rmdir != 0 || mount_sd != 0) {
        // Handle error
        std::cout << "error mounting sd card" << std::endl; 
    }

    

}