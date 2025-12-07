#!/bin/bash


echo "Stopping pigpiod daemon..."
sudo killall pigpiod 2>/dev/null

echo "Starting incremental build..."
# Ensure 'build' exists and is initialized; 'cmake' is only needed once initially
if [ ! -d "build" ]; then
    mkdir -p build
    cd build
    cmake ..
    cd ..
fi

cd build
make -j4  # Use -j4 to speed up compilation by using 4 CPU cores
if [ $? -ne 0 ]; then
    echo "Build failed!"
    exit 1
fi

echo "Executing Raspi-Alarm-Hub (Output logged to ../outputLog.txt)"
# Execute with -E to preserve environment variables for SDL/GUI, and log all output
sudo -E ./src/Raspi-Alarm-Hub > ../outputLog.txt 2>&1

cd .. # Return to root