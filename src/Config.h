#pragma once

namespace SkyrimNetLogger
{
    class Config
    {
    public:
        static Config* GetSingleton()
        {
            static Config singleton;
            return &singleton;
        }

        void Load();
        void Save();

        uint32_t GetToggleHotkey() const { return toggleHotkey; }
        int GetMessageCount() const { return messageCount; }

    private:
        Config() = default;
        ~Config() = default;
        Config(const Config&) = delete;
        Config& operator=(const Config&) = delete;

        uint32_t toggleHotkey = 197;  // PAUSE by default
        int messageCount = 10;       // Last 10 messages by default
    };
}
