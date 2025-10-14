#!/bin/bash

# install required dependencies

sudo apt-get update && sudo apt upgrade -y
sudo apt install -y build-essential cmake git pkg-config gdb
sudo apt-get install -y libsdl2-dev libsdl2-ttf-dev libsdl2-image-dev
sudo apt-get install -y libcurl4-openssl-dev libssl-dev

# load modules
sudo modprobe i2c-dev
sudo apt install i2c-tools
sudo apt install libgpiod-dev

##NOTE: need to enable dtparam=i2c_arm=on in /boot/config.txt to configure I2C
