#pragma once
#include <map>
#include <vector>
#include <functional>
#include <typeindex>
#include <memory>
#include <iostream>
#include <any>

class EventBus {
    public:
/**
     * @brief Subscribe a member function of a class to an event.
     * @tparam T The Event struct (e.g., AlarmTriggeredEvent).
     * @tparam U The Subscriber class (e.g., audioManager).
     * @param subscriber The 'this' pointer of the subscriber.
     * @param callback The member function to call (e.g., &audioManager::onAlarmTriggered).
     */
    template<typename T, typename U>
    void subscribe(U* subscriber, void (U::*callback)(const T&)) {
        auto type = std::type_index(typeid(T));

        // wrapper for both subscriber and callback
        auto wrapper = [subscriber, callback, type](std::any event) {
            try {
                const T& typedEvent = std::any_cast<const T&>(event);
                (subscriber->*callback)(typedEvent);
            } catch (const std::bad_any_cast& e) {
                std::cerr << "EventBus: bad cast for type " << type.name() << std::endl;
            }
        };

        m_subscribers[type].push_back(wrapper);
        std::cout << "EventBus: New subscriber for " << type.name() << std::endl;
    }

    template<typename T>
    void publish(const T& event) {
        auto type = std::type_index(typeid(T));

        if (m_subscribers.find(type) == m_subscribers.end()) {
            std::cout << "EventBus: No subscribers for " << type.name() << std::endl;
            return;
        }

        std::cout << "EventBus: publishing " << type.name() << " to " << m_subscribers[type].size() << "subscribers" << std::endl;
        
        for (auto& subscriber : m_subscribers[type]) {
            subscriber(event);
        }
    }

    private:
    // uses type index to map event types to list of 'any' function wrappers
    std::map<std::type_index, std::vector<std::function<void (std::any)>>> m_subscribers;
};