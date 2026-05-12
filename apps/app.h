#pragma once
#include <atomic>

namespace iot {

/**
 * @brief Application orchestrator — Facade pattern.
 *
 * Design Patterns: Facade
 *   - Single entry point for the entire system lifecycle
 *   - Owns and wires all services and controllers
 *   - Simple API: initialize() → run() → shutdown()
 *
 * Modern C++: std::unique_ptr ownership, RAII, std::atomic
 */
class App {
public:
    App();
    ~App();

    App(const App&) = delete;
    App& operator=(const App&) = delete;

    /**
     * @brief Initialize all subsystems.
     * @param configPath Path to JSON config file
     * @return true if all init succeeded
     */
    bool initialize(const char *configPath = "config/config.json");

    /**
     * @brief Run the application (starts all services).
     * @param maxSensorCycles Max sensor reading cycles (0 = infinite)
     */
    void run(int maxSensorCycles = 0) const;

    /**
     * @brief Gracefully shutdown all subsystems.
     */
    void shutdown();

private:
    static void printBanner();
    std::atomic<bool> initialized_{false};
};

} // namespace iot
