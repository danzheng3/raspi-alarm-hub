#include <string>
#include <vector>

class connectivityManager {
public:
    connectivityManager();
    ~connectivityManager();

    bool initBluetooth();
    bool initWifiEnabled();
    bool isBluetoothConnected();
    bool isWifiConnected();

    bool connectToWifi(const std::string& ssid, const std::string& password);
    bool disconnectWifi();
    bool scanForWifiNetworks(std::vector<std::string>& networks);
    bool connectToSpeaker(const std::string& deviceAddress);
    bool disconnectSpeaker();
private:
    bool bluetoothConnected;
    bool wifiConnected;
    std::string currentSSID;

};