#include <string>

class BluetoothAdapter {
    public:
        BluetoothAdapter();
        ~BluetoothAdapter();
        void initialize();
        bool connectToDevice(const std::string& deviceAddress);
        void disconnect();
        bool isConnected();

    private:
        std::string speakerID;
        bool connected;
        bool bluetoothEnabled;
};