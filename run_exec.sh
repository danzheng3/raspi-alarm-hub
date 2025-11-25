echo "Building Raspi-Alarm-Hub"
rm -rf build
mkdir -p build
cd build
cmake ..
make
cd src
./Raspi-Alarm-Hub
