#include "LogMonitor.h"
#include "Config.h"
#include <fstream>
#include <filesystem>
#include <sstream>

namespace SkyrimNetLogger
{
    void LogMonitor::CheckConversationLog()
    {
        // Build path to SkyrimNet conversation log
        auto sksePluginsPath = std::filesystem::current_path() / "Data" / "SKSE" / "Plugins" / "SkyrimNet" / "logs" / "conversation_log.log";
        
        auto console = RE::ConsoleLog::GetSingleton();
        
        if (!std::filesystem::exists(sksePluginsPath)) {
            SKSE::log::info("[DEBUG] Conversation log not found");
            if (console) {
                console->Print("[SkyrimNet Chat Log] conversation_log.log not found");
            }
            return;
        }
        
        std::ifstream logFile(sksePluginsPath);
        if (!logFile.is_open()) {
            SKSE::log::info("[DEBUG] Could not open conversation log");
            if (console) {
                console->Print("[SkyrimNet Chat Log] Could not open conversation_log.log");
            }
            return;
        }
        
        // Get config first
        auto config = Config::GetSingleton();
        int messageCount = config->GetMessageCount();
        
        // Read file backwards to get exactly N message entries
        logFile.seekg(0, std::ios::end);
        std::streampos fileSize = logFile.tellg();
        
        if (fileSize == 0) {
            logFile.close();
            if (console) {
                console->Print("[SkyrimNet Chat Log] Conversation log is empty");
            }
            return;
        }
        
        // Read file in chunks from the end, looking for message entries
        const size_t chunkSize = 8192;
        std::vector<std::string> lines;
        std::string buffer;
        
        std::streampos pos = fileSize;
        bool foundEnough = false;
        int entriesFound = 0;
        
        while (pos > 0 && !foundEnough) {
            // Calculate chunk to read
            size_t toRead = std::min(static_cast<size_t>(pos), chunkSize);
            pos -= toRead;
            
            // Read chunk
            logFile.seekg(pos);
            std::vector<char> chunk(toRead);
            logFile.read(chunk.data(), toRead);
            
            // Prepend chunk to buffer
            std::string chunkStr(chunk.begin(), chunk.end());
            buffer = chunkStr + buffer;
            
            // Extract complete lines from buffer
            size_t lastNewline = buffer.find_last_of('\n');
            if (lastNewline != std::string::npos && pos > 0) {
                // Keep partial line in buffer for next iteration
                std::string completeLines = buffer.substr(0, lastNewline);
                buffer = buffer.substr(lastNewline + 1);
                
                // Split into lines and prepend to our lines vector
                std::istringstream stream(completeLines);
                std::string line;
                std::vector<std::string> newLines;
                while (std::getline(stream, line)) {
                    newLines.push_back(line);
                }
                lines.insert(lines.begin(), newLines.begin(), newLines.end());
            }
            
            // Count message entries (lines starting with '[')
            entriesFound = 0;
            for (const auto& line : lines) {
                if (!line.empty() && line[0] == '[') {
                    entriesFound++;
                    if (entriesFound >= messageCount) {
                        foundEnough = true;
                        break;
                    }
                }
            }
        }
        
        // Add any remaining buffer content (this is at the END of the file)
        if (!buffer.empty()) {
            std::istringstream stream(buffer);
            std::string line;
            while (std::getline(stream, line)) {
                lines.push_back(line);  // Add to END, not beginning
            }
        }
        
        logFile.close();
        
        // Now group lines into message entries
        std::vector<std::vector<std::string>> messages;
        for (const auto& line : lines) {
            if (line.empty()) {
                continue;
            }
            
            if (line[0] == '[') {
                messages.push_back({line});
            } else if (!messages.empty()) {
                messages.back().push_back(line);
            }
        }
        
        // Get last N message entries
        
        if (!console) {
            return;
        }
        
        size_t startIdx = messages.size() > static_cast<size_t>(messageCount) ? messages.size() - messageCount : 0;
        SKSE::log::info("[DEBUG] Outputting last {} conversation entries", messages.size() - startIdx);
        
        console->Print("=== Last SkyrimNet Conversations ===");
        for (size_t i = startIdx; i < messages.size(); ++i) {
            for (const auto& line : messages[i]) {
                // Strip timestamp from first line only (format: [YYYY-MM-DD HH:MM:SS] message)
                std::string output = line;
                if (line[0] == '[') {
                    auto bracketEnd = line.find(']');
                    if (bracketEnd != std::string::npos && bracketEnd + 2 < line.size()) {
                        output = line.substr(bracketEnd + 2);  // Skip "] "
                        
                        // Also strip "[NPC] " prefix if present
                        if (output.size() >= 6 && output.substr(0, 6) == "[NPC] ") {
                            output = output.substr(6);
                        }
                    }
                }
                console->Print(output.c_str());
            }
        }
    }

    void LogMonitor::ToggleAutoDisplay()
    {
        autoDisplayEnabled = !autoDisplayEnabled;
        
        if (autoDisplayEnabled) {
            RE::DebugNotification("SkyrimNet Chat Log: Auto-display ENABLED");
        } else {
            RE::DebugNotification("SkyrimNet Chat Log: Auto-display DISABLED");
        }
        
        SKSE::log::info("[DEBUG] Auto-display toggled: {}", autoDisplayEnabled ? "ON" : "OFF");
    }

    RE::BSEventNotifyControl LogMonitor::ProcessEvent(const RE::MenuOpenCloseEvent* a_event, RE::BSTEventSource<RE::MenuOpenCloseEvent>*)
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

    RE::BSEventNotifyControl LogMonitor::ProcessEvent(RE::InputEvent* const* a_event, RE::BSTEventSource<RE::InputEvent*>*)
    {
        if (!a_event) {
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
