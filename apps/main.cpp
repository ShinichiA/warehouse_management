#include <iostream>
#include "app.h"
#include "utils/logger.h"
#include "utils/datetime.h"
#include "utils/terminal.h"
#include <thread>
#include <chrono>

int main() {
    LOG_INFO("--- System Startup ---");
    
    // Example Terminal usage
    int exitCode = -1;

    LOG_INFO("Detecting OS information...");
    std::string osInfo = iot::Terminal::execute("lsb_release -d | cut -f2", exitCode);
    LOG_INFO("OS: " + osInfo);

    std::string currentUser = iot::Terminal::execute("whoami");
    LOG_INFO("Current User: " + currentUser);

    if (iot::Terminal::exists("cmake")) {
        LOG_INFO("CMake is available on this system.");
    }
    
    iot::App app;
    if (app.initialize("config/config.json")) {
        app.run();
        
        LOG_INFO("System is running. Press Ctrl+C to stop (Simulated for 30s)...");
        std::this_thread::sleep_for(std::chrono::seconds(30));
    } else {
        LOG_ERROR("Failed to initialize App!");
    }

    app.shutdown();
    
    LOG_INFO("--- System Exit ---");
    return 0;
}