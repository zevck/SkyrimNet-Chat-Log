#pragma once

namespace SkyrimNetLogger
{
    class SubtitleMonitor : public RE::BSTEventSink<RE::MenuOpenCloseEvent>,
                            public RE::BSTEventSink<RE::InputEvent*>
    {
    public:
        static SubtitleMonitor* GetSingleton()
        {
            static SubtitleMonitor singleton;
            return &singleton;
        }

        void CheckConversationLog();  // Monitor SkyrimNet's conversation_log.log
        void ToggleAutoDisplay();  // Toggle auto-display on console open
        
        RE::BSEventNotifyControl ProcessEvent(const RE::MenuOpenCloseEvent* a_event, RE::BSTEventSource<RE::MenuOpenCloseEvent>*) override;
        RE::BSEventNotifyControl ProcessEvent(RE::InputEvent* const* a_event, RE::BSTEventSource<RE::InputEvent*>*) override;

    private:
        SubtitleMonitor() = default;
        ~SubtitleMonitor() = default;
        SubtitleMonitor(const SubtitleMonitor&) = delete;
        SubtitleMonitor& operator=(const SubtitleMonitor&) = delete;

        bool autoDisplayEnabled = true; // Toggle for auto-display on console open
    };
}
