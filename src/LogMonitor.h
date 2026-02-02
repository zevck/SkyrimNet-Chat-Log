#pragma once

namespace SkyrimNetLogger
{
    class LogMonitor : public RE::BSTEventSink<RE::MenuOpenCloseEvent>,
                       public RE::BSTEventSink<RE::InputEvent*>
    {
    public:
        static LogMonitor* GetSingleton()
        {
            static LogMonitor singleton;
            return &singleton;
        }

        void CheckConversationLog();  // Monitor SkyrimNet's conversation_log.log
        void ToggleAutoDisplay();  // Toggle auto-display on console open
        
        RE::BSEventNotifyControl ProcessEvent(const RE::MenuOpenCloseEvent* a_event, RE::BSTEventSource<RE::MenuOpenCloseEvent>*) override;
        RE::BSEventNotifyControl ProcessEvent(RE::InputEvent* const* a_event, RE::BSTEventSource<RE::InputEvent*>*) override;

    private:
        LogMonitor() = default;
        ~LogMonitor() = default;
        LogMonitor(const LogMonitor&) = delete;
        LogMonitor& operator=(const LogMonitor&) = delete;

        bool autoDisplayEnabled = true; // Toggle for auto-display on console open
    };
}
