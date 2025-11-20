#!/bin/bash

# install required dependencies


nmcli device wifi connect "ER-S GRANT APT 13" password "CVG455wmz$" ifname wlan0

nmcli -t -f GENERAL.STATE device show wlan0 | grep -q "connected" || {
    echo "No network connected"
    exit 1
}

sudo apt-get update && sudo apt upgrade -y
sudo apt install -y build-essential cmake git pkg-config gdb
sudo apt-get install -y libsdl2-dev libsdl2-ttf-dev libsdl2-image-dev
sudo apt-get install -y libcurl4-openssl-dev libssl-dev

# load modules
sudo modprobe i2c-dev
sudo apt install i2c-tools
sudo apt install libgpiod-dev

#saving to json
sudo apt install nlohmann-json3-dev

#audio drivers
sudo apt install pulseaudio pulseaudio-utils libpulse-dev
sudo apt install libalsa-dev alsa-utils
sudo apt install mpg123

echo "dtparam=spi=on" | sudo tee -a /boot/firmware/config.txt
echo "dtoverlay=mmc-spi-slot" | sudo tee -a /boot/firmware/config.txt
echo "dtoverlay=pwm,pin=18,func=4" | sudo tee -a /boot/firmware/config.txt
#enable pwm
sudo modprobe spi_bcm2835
sudo modprobe mmc_spi
##NOTE: need to enable dtparam=i2c_arm=on in /boot/config.txt to configure I2C

# ALSO: NEED TO MODIFY THE RASPI CONFIG TO ENABLE PWM ON PIN 18


# for PILLBOX driver : need to use PWM
sudo apt-get install libpigpiod-if-dev libpigpiod-if2-1t64
sudo pigpiod


##NOTE2: PRE-SAVED CONFIGURATIONS UNDER 'CONFIG.JSON' in build/src