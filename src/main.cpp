#include "LogMonitor.h"
#include "Config.h"
#include <spdlog/sinks/basic_file_sink.h>

namespace
{
    void OnMessage(SKSE::MessagingInterface::Message* msg)
    {
        switch (msg->type) {
        case SKSE::MessagingInterface::kDataLoaded:
            SKSE::log::info("Game data loaded - subtitle monitoring active");
            
            // Load configuration
            SkyrimNetLogger::Config::GetSingleton()->Load();
            
            // Register for menu open/close events
            auto ui = RE::UI::GetSingleton();
            if (ui) {
                ui->AddEventSink<RE::MenuOpenCloseEvent>(SkyrimNetLogger::LogMonitor::GetSingleton());
                SKSE::log::info("Registered menu event sink");
            }
            
            // Register for input events (for hotkeys)
            auto inputDeviceMgr = RE::BSInputDeviceManager::GetSingleton();
            if (inputDeviceMgr) {
                inputDeviceMgr->AddEventSink<RE::InputEvent*>(SkyrimNetLogger::LogMonitor::GetSingleton());
                SKSE::log::info("Registered input event sink");
            }
            
            auto consoleLog = RE::ConsoleLog::GetSingleton();
            if (consoleLog) {
                auto config = SkyrimNetLogger::Config::GetSingleton();
                auto msg = std::format("[SkyrimNetLogger] Ready! Hotkey: {} | Messages: {} | Edit SKSE/Plugins/SkyrimNetLogger.ini to customize",
                    config->GetToggleHotkey(), config->GetMessageCount());
                consoleLog->Print(msg.c_str());
            }
            break;
        }
    }

    void InitializeLog()
    {
        auto path = SKSE::log::log_directory();
        if (!path) {
            return;
        }

        *path /= "SkyrimNetLogger.log"sv;
        auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);

        auto log = std::make_shared<spdlog::logger>("global log"s, std::move(sink));
        log->set_level(spdlog::level::info);
        log->flush_on(spdlog::level::info);

        spdlog::set_default_logger(std::move(log));
        spdlog::set_pattern("[%H:%M:%S] [%l] %v"s);

        SKSE::log::info("SkyrimNetLogger v{}", "1.0.0");
    }
}

extern "C" DLLEXPORT constinit auto SKSEPlugin_Version = []() {
    SKSE::PluginVersionData v{};
    v.PluginVersion({ 1, 0, 0, 0 });
    v.PluginName("SkyrimNetLogger");
    v.AuthorName("SkyrimModder");
    v.UsesAddressLibrary(true);
    v.HasNoStructUse(true);
    return v;
}();

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Query(const SKSE::QueryInterface* a_skse, SKSE::PluginInfo* a_info)
{
    a_info->infoVersion = SKSE::PluginInfo::kVersion;
    a_info->name = "SkyrimNetLogger";
    a_info->version = 1;

    if (a_skse->IsEditor()) {
        return false;
    }

    return true;
}

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* a_skse)
{
    InitializeLog();
    SKSE::log::info("Loading SkyrimNetLogger...");

    SKSE::Init(a_skse);
    
    auto messaging = SKSE::GetMessagingInterface();
    if (messaging) {
        messaging->RegisterListener(OnMessage);
    }

    SKSE::log::info("SkyrimNetLogger loaded successfully!");
    return true;
}
