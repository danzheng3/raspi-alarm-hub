#include <hardware_layer/BluetoothAdapter.h>
#include <iostream>
#include <memory>
#include <array>

BluetoothAdapter::BluetoothAdapter() : connected(false), speakerID("44:1D:64:AE:53:4E"), bluetoothEnabled(false) {}
BluetoothAdapter::~BluetoothAdapter() {}

std::string BluetoothAdapter::stringToHex(const std::string& input) {
    std::ostringstream ret;
    for (unsigned char c : input) {
        ret << std::hex << std::setw(2) << std::setfill('0') << (int)c;
    }
    return ret.str();
}

bool BluetoothAdapter::sendGattMessage(const std::string& handle, const std::string& message, std::string deviceAddr) {
    std::string target = speakerID;
    std::cout << "[bt] send gatt to : " << target << std::endl;

    std::string hexMsg = stringToHex("alarm");
    
    // Construct command: gatttool -b <MAC> --char-write-req -a <HANDLE> -n <HEX_VALUE>
    // Note: gatttool often requires root privileges to access the HCI interface directly.
    std::string gattCommand = "connect\\nchar-write-cmd 0x002a 616c61726d";
    std::string fullCommand = "echo -e \"" + gattCommand + "\" | sudo gatttool -i hci0 -b 44:1D:64:AE:53:4E -I";

    std::cout << "[BLE] Executing send" << std::endl;
    
    int result = system("sudo gatttool -i hci0 -b 44:1D:64:AE:53:4E --char-write-req -a 0x002a -n 616c61726d");
    
    if (result == 0) {
        std::cout << "[BLE] Message sent successfully." << std::endl;
        return true;
    } else {
        std::cerr << "[BLE] Failed to send message. Exit code: " << result << std::endl;
        return false;
    }
}

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

bool BluetoothAdapter::connectToDevice(const std::string& deviceAddress) {
    speakerID = deviceAddress;
    std::cout << "Connecting to device: " << deviceAddress << std::endl;
    runCommand("bluetoothctl trust " + deviceAddress);
    runCommand("bluetoothctl connect " + deviceAddress);
    connected = isConnected();
    if (connected) {
        std::cout << "Connected to " << deviceAddress << std::endl;
        return true;
    } else {
        std::cout << "Failed to connect to " << deviceAddress << std::endl;
        return false;
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

