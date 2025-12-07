echo "Building Raspi-Alarm-Hub"
sudo rm -rf build
mkdir -p build
cd build
cmake ..
make
cd src
echo "Executing Raspi-Alarm-Hub"
sudo -E ./Raspi-Alarm-Hub > ../../outputLog.txt 2>&1
