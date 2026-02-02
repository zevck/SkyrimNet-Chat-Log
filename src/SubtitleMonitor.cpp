#include "SubtitleMonitor.h"
#include "Config.h"
#include <fstream>
#include <filesystem>

namespace SkyrimNetLogger
{
    void SubtitleMonitor::CheckConversationLog()
    {
        // Build path to SkyrimNet conversation log
        auto sksePluginsPath = std::filesystem::current_path() / "Data" / "SKSE" / "Plugins" / "SkyrimNet" / "logs" / "conversation_log.log";
        
        auto console = RE::ConsoleLog::GetSingleton();
        
        if (!std::filesystem::exists(sksePluginsPath)) {
            SKSE::log::info("[DEBUG] Conversation log not found");
            if (console) {
                console->Print("[SkyrimNetLogger] conversation_log.log not found");
            }
            return;
        }
        
        std::ifstream logFile(sksePluginsPath);
        if (!logFile.is_open()) {
            SKSE::log::info("[DEBUG] Could not open conversation log");
            if (console) {
                console->Print("[SkyrimNetLogger] Could not open conversation_log.log");
            }
            return;
        }
        
        // Read all lines into a vector
        std::vector<std::string> lines;
        std::string line;
        while (std::getline(logFile, line)) {
            if (!line.empty()) {
                lines.push_back(line);
            }
        }
        logFile.close();
        
        // Get last N lines based on config
        auto config = Config::GetSingleton();
        int messageCount = config->GetMessageCount();
        
        if (!console) {
            return;
        }
        
        size_t startIdx = lines.size() > static_cast<size_t>(messageCount) ? lines.size() - messageCount : 0;
        SKSE::log::info("[DEBUG] Outputting last {} conversation log entries", lines.size() - startIdx);
        
        console->Print("=== Last SkyrimNet Conversations ===");
        for (size_t i = startIdx; i < lines.size(); ++i) {
            // Strip timestamp from line (format: [YYYY-MM-DD HH:MM:SS] message)
            std::string line = lines[i];
            auto bracketEnd = line.find(']');
            if (bracketEnd != std::string::npos && bracketEnd + 2 < line.size()) {
                line = line.substr(bracketEnd + 2);  // Skip "] "
            }
            console->Print(line.c_str());
        }
    }

    void SubtitleMonitor::ToggleAutoDisplay()
    {
        autoDisplayEnabled = !autoDisplayEnabled;
        auto console = RE::ConsoleLog::GetSingleton();
        if (console) {
            if (autoDisplayEnabled) {
                console->Print("[SkyrimNetLogger] Auto-display ENABLED - conversation log will show on console open");
            } else {
                console->Print("[SkyrimNetLogger] Auto-display DISABLED");
            }
        }
        SKSE::log::info("[DEBUG] Auto-display toggled: {}", autoDisplayEnabled ? "ON" : "OFF");
    }

    RE::BSEventNotifyControl SubtitleMonitor::ProcessEvent(const RE::MenuOpenCloseEvent* a_event, RE::BSTEventSource<RE::MenuOpenCloseEvent>*)
    {
        if (!a_event) {
            return RE::BSEventNotifyControl::kContinue;
        }
        
        // Check if console is opening
        if (a_event->menuName == RE::Console::MENU_NAME && a_event->opening) {
            if (autoDisplayEnabled) {
                CheckConversationLog();
            }
        }
        
        return RE::BSEventNotifyControl::kContinue;
    }

    RE::BSEventNotifyControl SubtitleMonitor::ProcessEvent(RE::InputEvent* const* a_event, RE::BSTEventSource<RE::InputEvent*>*)
    {
        if (!a_event) {
            return RE::BSEventNotifyControl::kContinue;
        }
        
        // Only process input when console is open
        auto ui = RE::UI::GetSingleton();
        if (!ui || !ui->IsMenuOpen(RE::Console::MENU_NAME)) {
            return RE::BSEventNotifyControl::kContinue;
        }
        
        for (auto* event = *a_event; event; event = event->next) {
            if (event->eventType != RE::INPUT_EVENT_TYPE::kButton) {
                continue;
            }
            
            auto* buttonEvent = event->AsButtonEvent();
            if (!buttonEvent || !buttonEvent->IsDown()) {
                continue;
            }
            
            auto key = buttonEvent->idCode;
            auto config = Config::GetSingleton();
            
            // Check if pressed key matches configured toggle hotkey
            if (key == config->GetToggleHotkey()) {
                ToggleAutoDisplay();
            }
        }
        
        return RE::BSEventNotifyControl::kContinue;
    }
}
