#include <string>
#include "storageManager.h"

class timeManager {
    public:
        timeManager(storageManager& storage);
        ~timeManager();

        std::string getCurrentTime() const;
        void setTime(const std::string& time);
        void syncFromRTC();
    
    private:
        std::string currentTime;
        storageManager& storage;
};