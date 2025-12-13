#include <string>
#include <vector>
#include <sstream>
#include <iomanip>

class BluetoothAdapter {
    public:
        BluetoothAdapter();
        ~BluetoothAdapter();
        void initialize();
        bool connectToDevice(const std::string& deviceAddress);
        void disconnect();
        bool isConnected();

        bool sendGattMessage(const std::string& handle, const std::string& message, std::string deviceAddr = ""); // hard coded, need to adjust in the future.

    private:
        std::string speakerID;
        bool connected;
        bool bluetoothEnabled;
        std::string stringToHex(const std::string& input);
};