#include "Config.h"
#include <fstream>
#include <filesystem>
#include <sstream>

namespace SkyrimNetLogger
{
    void Config::Load()
    {
        auto configPath = std::filesystem::current_path() / "Data" / "SKSE" / "Plugins" / "SkyrimNetChatLog.ini";
        
        if (!std::filesystem::exists(configPath)) {
            SKSE::log::info("Config file not found, creating default config at: {}", configPath.string());
            Save();
            return;
        }
        
        std::ifstream file(configPath);
        if (!file.is_open()) {
            SKSE::log::error("Failed to open config file");
            return;
        }
        
        std::string line;
        while (std::getline(file, line)) {
            // Skip comments and empty lines
            if (line.empty() || line[0] == ';' || line[0] == '#') {
                continue;
            }
            
            // Parse key=value
            auto pos = line.find('=');
            if (pos == std::string::npos) {
                continue;
            }
            
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);
            
            // Trim whitespace
            key.erase(0, key.find_first_not_of(" \t"));
            key.erase(key.find_last_not_of(" \t") + 1);
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t") + 1);
            
            if (key == "ToggleHotkey") {
                try {
                    toggleHotkey = std::stoul(value);
                    SKSE::log::info("Loaded ToggleHotkey: {}", toggleHotkey);
                } catch (...) {
                    SKSE::log::error("Invalid ToggleHotkey value: {}", value);
                }
            }
            else if (key == "MessageCount") {
                try {
                    messageCount = std::stoi(value);
                    if (messageCount < 1) messageCount = 1;
                    if (messageCount > 100) messageCount = 100;
                    SKSE::log::info("Loaded MessageCount: {}", messageCount);
                } catch (...) {
                    SKSE::log::error("Invalid MessageCount value: {}", value);
                }
            }
        }
        
        file.close();
    }
    
    void Config::Save()
    {
        auto configPath = std::filesystem::current_path() / "Data" / "SKSE" / "Plugins" / "SkyrimNetChatLog.ini";
        
        // Ensure directory exists
        std::filesystem::create_directories(configPath.parent_path());
        
        std::ofstream file(configPath);
        if (!file.is_open()) {
            SKSE::log::error("Failed to create config file");
            return;
        }
        
        file << "; SkyrimNetLogger Configuration\n";
        file << "; https://wiki.nexusmods.com/index.php/DirectX_Scancodes_And_How_To_Use_Them\n";
        file << ";\n";
        file << "; Common scancodes:\n";
        file << "; F1=59, F2=60, F3=61, F4=62, F5=63, F6=64, F7=65, F8=66, F9=67, F10=68\n";
        file << "; F11=87, F12=88, PAUSE=197\n";
        file << ";\n";
        file << "\n";
        file << "; Hotkey scancode to toggle auto-display (default: PAUSE=197, works globally)\n";
        file << "ToggleHotkey=" << toggleHotkey << "\n";
        file << "\n";
        file << "; Number of recent SkyrimNet messages to display (1-100)\n";
        file << "MessageCount=" << messageCount << "\n";
        
        file.close();
        SKSE::log::info("Config file saved");
    }
}
