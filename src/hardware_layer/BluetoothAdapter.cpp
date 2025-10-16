#include <hardware_layer/BluetoothAdapter.h>
#include <iostream>
#include <memory>
#include <array>

BluetoothAdapter::BluetoothAdapter() : connected(false), speakerID(""), bluetoothEnabled(false) {}
BluetoothAdapter::~BluetoothAdapter() {}

static std::string runCommand(const std::string& command) {
    std::array<char, 128> buffer;
    std::string result;
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) {
        return "ERROR";
    }
    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        result += buffer.data();
    }
    pclose(pipe);
    return result;
}

void BluetoothAdapter::initialize() {
    std::cout << "initialize bluetooth" << std::endl;
    runCommand("sudo rfkill unblock bluetooth");
    runCommand("sudo systemctl start bluetooth");
    runCommand("bluetoothctl power on");
    // bluetooth initialization commands for raspi
}

bool BluetoothAdapter::isConnected() {
    std::string output = runCommand("bluetoothctl info " + speakerID);
    return output.find("Connected: yes") != std::string::npos;
}

void BluetoothAdapter::connectToDevice(const std::string& deviceAddress) {
    speakerID = deviceAddress;
    std::cout << "Connecting to device: " << deviceAddress << std::endl;
    runCommand("bluetoothctl trust " + deviceAddress);
    runCommand("bluetoothctl connect " + deviceAddress);
    connected = isConnected();
    if (connected) {
        std::cout << "Connected to " << deviceAddress << std::endl;
    } else {
        std::cout << "Failed to connect to " << deviceAddress << std::endl;
    }
}

void BluetoothAdapter::disconnect() {
    if (speakerID.empty()) {
        std::cout << "No device connected to disconnect" << std::endl;
        return;
    }
    std::cout << "Disconnecting from device: " << speakerID << std::endl;
    runCommand("bluetoothctl disconnect " + speakerID);
    connected = false;
    speakerID = "";
}

