#pragma once

#include <string>
#include <iostream>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <array>
#include <vector>
#include <sys/wait.h>

namespace iot {

/**
 * @brief Utility for communicating with the Ubuntu/Linux terminal.
 */
class Terminal {
public:
    /**
     * @brief Execute a shell command and capture its stdout.
     * @param cmd The command to execute
     * @return std::string The output (stdout) of the command
     */
    static std::string execute(const std::string& cmd) {
        char buffer[128] = {};
        std::string result;

        FILE* pipe = popen(cmd.c_str(), "r");
        if (!pipe) {
            throw std::runtime_error("popen() failed!");
        }
        
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            result += buffer;
        }
        
        pclose(pipe);
        return result;
    }

    /**
     * @brief Execute a command and get both output and exit code.
     * @param cmd The command to execute
     * @param exitCode Output parameter for the return code
     * @return std::string The output (stdout)
     */
    static std::string execute(const std::string& cmd, int& exitCode) {
        char buffer[128] = {};
        std::string result;
        
        FILE* pipe = popen(cmd.c_str(), "r");
        if (!pipe) {
            exitCode = -1;
            return "Error: popen() failed";
        }

        while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            result += buffer;
        }

        int status = pclose(pipe);
        if (WIFEXITED(status)) {
            exitCode = WEXITSTATUS(status);
        } else {
            exitCode = status;
        }

        return result;
    }

    /**
     * @brief Check if a command exists in the system.
     * @param cmd Command name (e.g., "git", "python3")
     * @return true if available
     */
    static bool exists(const std::string& cmd) {
        std::string checkCmd = "command -v " + cmd + " > /dev/null 2>&1";
        return (system(checkCmd.c_str()) == 0);
    }

    /**
     * @brief Execute a command without capturing output.
     * @param cmd The command to run
     * @return int Exit code
     */
    static int run(const std::string& cmd) {
        return system(cmd.c_str());
    }
};

} // namespace iot
