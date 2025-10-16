#include <string>
#include "storageManager.h"
#include "timeManager.h"


class alarmManager {
    public:
        alarmManager(storageManager& storage, timeManager& timeMgr);
        ~alarmManager();

        void setAlarm(const std::string& time);
        void loadFromStorage();
        void saveToStorage();
        bool isAlarmEnabled() const;

        std::string getAlarmTime() const { return alarmTime; }
        bool shouldTrigger();
        void resetTrigger();

    private:
        timeManager& timeMgr;
        bool alarmEnabled;
        bool alarmTriggered;
        std::string alarmTime;
        storageManager& storage;
};
