#!/bin/bash

# install required dependencies

sudo apt-get update && sudo apt upgrade -y
sudo apt install -y build-essential cmake git pkg-config gdb
sudo apt-get install -y libsdl2-dev libsdl2-ttf-dev libsdl2-image-dev
sudo apt-get install -y libcurl4-openssl-dev libssl-dev
