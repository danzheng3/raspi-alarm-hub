#!/bin/bash


echo "Stopping pigpiod daemon..."
sudo killall pigpiod 2>/dev/null

echo "Starting incremental build..."
if [ ! -d "build" ]; then
    mkdir -p build
    cd build
    cmake ..
    cd ..
fi

cd build
make -j4 
if [ $? -ne 0 ]; then
    echo "Build failed!"
    exit 1
fi

brightnessctl s 30
export DISPLAY=:0
export XAUTHORITY=/home/daniel/.Xauthority
xhost +local:
echo "Executing Raspi-Alarm-Hub (Output logged to ../outputLog.txt)"
sudo -E ./src/Raspi-Alarm-Hub > ../outputLog.txt 2>&1

cd ..