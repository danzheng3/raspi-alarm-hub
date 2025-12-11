#!/bin/bash
sudo pkill -9 -u daniel -x mpg123 2>/dev/null

# 2. Kill GPIO Daemon (Fixes Port 8888 error)
sudo killall -9 pigpiod 2>/dev/null
sudo rm -f /var/run/pigpio.pid

# 3. WAIT for port 8888 to clear
# This loop waits until port 8888 is actually free
echo "Waiting for ports to clear..."
sudo lsof -i :8888 | awk '{print $2}' | grep -v PID | xargs -r kill -9 2>/dev/null

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
# Kill the desktop bluetooth applet to stop popups
sudo pkill -f blueman-applet
sudo -E ./src/Raspi-Alarm-Hub > ../outputLog.txt 2>&1

cd ..