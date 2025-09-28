#pragma once
#include <string>
#include <filesystem>
#include <iostream>
#include <limits>
#ifdef _WIN32
#include <conio.h>
#endif
#ifndef _WIN32
#include <termios.h>
#include <unistd.h>
#endif

namespace utils {
    inline void ensure_data_dirs() {
        std::filesystem::create_directories("data/transactions");
    }

    inline std::string get_password_masked(const std::string& prompt = "Password: ") {
        std::cout << prompt;
        std::string password;
#ifdef _WIN32
        char ch;
        while ((ch = _getch()) != '\r') {
            if (ch == '\b') {
                if (!password.empty()) { password.pop_back(); std::cout << "\b \b"; }
            } else if (ch == 3) { // Ctrl+C
                throw std::runtime_error("Interrupted");
            } else if (isprint(static_cast<unsigned char>(ch))) {
                password.push_back(ch);
                std::cout << '*';
            }
        }
        std::cout << std::endl;
#else
        termios oldt, newt;
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt; newt.c_lflag &= ~ECHO;
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
        std::getline(std::cin, password);
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        std::cout << std::endl;
#endif
        return password;
    }

    inline void clear_input() {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
}
