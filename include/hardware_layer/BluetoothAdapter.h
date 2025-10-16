#include <string>

class BluetoothAdapter {
    public:
        BluetoothAdapter();
        ~BluetoothAdapter();
        void initialize();
        void connectToDevice(const std::string& deviceAddress);
        void disconnect();
        bool isConnected();

    private:
        std::string speakerID;
        bool connected;
        bool bluetoothEnabled;
};