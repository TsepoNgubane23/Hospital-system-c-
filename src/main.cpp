#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <filesystem>

#include "bank.hpp"
#include "utils.hpp"

static void print_user_menu(bool is_admin) {
    std::cout << "\n==== User Menu ====\n";
    std::cout << "1. Check Balance\n";
    std::cout << "2. Deposit\n";
    std::cout << "3. Withdraw\n";
    std::cout << "4. Transfer\n";
    std::cout << "5. View Transaction History\n";
    if (is_admin) {
        std::cout << "6. [Admin] View All Accounts\n";
        std::cout << "7. Logout\n";
    } else {
        std::cout << "6. Logout\n";
    }
    std::cout << "Select option: ";
}

static void view_history(const std::string& username) {
    std::filesystem::path file = std::filesystem::path("data/transactions") / (username + ".log");
    std::ifstream in(file);
    if (!in) {
        std::cout << "No history available." << std::endl;
        return;
    }
    std::cout << "\n-- Transaction History for '" << username << "' --\n";
    std::string line;
    int count = 0;
    while (std::getline(in, line)) {
        std::istringstream iss(line);
        std::string ts, type, amount_str, bal_str, details;
        if (std::getline(iss, ts, '|') && std::getline(iss, type, '|') &&
            std::getline(iss, amount_str, '|') && std::getline(iss, bal_str, '|') && std::getline(iss, details)) {
            std::cout << ts << " | " << type << " | Amount: " << amount_str << " | Balance: " << bal_str << " | " << details << "\n";
            ++count;
        }
    }
    if (count == 0) std::cout << "(empty)\n";
}

int main() {
    try {
        utils::ensure_data_dirs();
        Bank bank;
        bank.load();

        while (true) {
            std::cout << "\n====== Banking System ======\n";
            std::cout << "1. Create Account\n";
            std::cout << "2. Login\n";
            std::cout << "3. Exit\n";
            std::cout << "Select option: ";
            int choice = 0;
            if (!(std::cin >> choice)) { utils::clear_input(); continue; }
            utils::clear_input();

            if (choice == 1) {
                std::string username; double initial_balance = 0.0; std::string err;
                std::cout << "Enter username: "; std::getline(std::cin, username);
                std::string password = utils::get_password_masked("Enter password: ");
                std::string confirm = utils::get_password_masked("Confirm password: ");
                if (password != confirm) { std::cout << "Passwords do not match.\n"; continue; }
                std::cout << "Initial balance (>=0): ";
                if (!(std::cin >> initial_balance)) { std::cout << "Invalid amount.\n"; utils::clear_input(); continue; }
                utils::clear_input();
                if (!bank.create_account(username, password, initial_balance, err)) {
                    std::cout << "Failed: " << err << "\n";
                } else {
                    std::cout << "Account created successfully.\n";
                }
            } else if (choice == 2) {
                std::string username; std::cout << "Username: "; std::getline(std::cin, username);
                std::string password = utils::get_password_masked("Password: ");
                auto acc_opt = bank.authenticate(username, password);
                if (!acc_opt) { std::cout << "Invalid credentials.\n"; continue; }
                bool admin = bank.is_admin(username);
                std::cout << "Welcome, " << username << (admin ? " [admin]" : "") << "!\n";
                // user session
                bool session = true;
                while (session) {
                    print_user_menu(admin);
                    int op = 0; if (!(std::cin >> op)) { utils::clear_input(); continue; }
                    utils::clear_input();
                    if (!admin) {
                        switch (op) {
                            case 1: {
                                auto bal = bank.balance_of(username);
                                if (bal) std::cout << "Balance: " << *bal << "\n"; else std::cout << "Error reading balance.\n";
                                break;
                            }
                            case 2: {
                                std::cout << "Amount to deposit: "; double amt; if (!(std::cin >> amt)) { utils::clear_input(); std::cout << "Invalid amount.\n"; break; }
                                utils::clear_input(); std::string err; if (bank.deposit(username, amt, err)) std::cout << "Deposit successful.\n"; else std::cout << "Failed: " << err << "\n";
                                break;
                            }
                            case 3: {
                                std::cout << "Amount to withdraw: "; double amt; if (!(std::cin >> amt)) { utils::clear_input(); std::cout << "Invalid amount.\n"; break; }
                                utils::clear_input(); std::string err; if (bank.withdraw(username, amt, err)) std::cout << "Withdrawal successful.\n"; else std::cout << "Failed: " << err << "\n";
                                break;
                            }
                            case 4: {
                                std::string to; std::cout << "Transfer to (username): "; std::getline(std::cin, to);
                                std::cout << "Amount: "; double amt; if (!(std::cin >> amt)) { utils::clear_input(); std::cout << "Invalid amount.\n"; break; }
                                utils::clear_input(); std::string err; if (bank.transfer(username, to, amt, err)) std::cout << "Transfer successful.\n"; else std::cout << "Failed: " << err << "\n";
                                break;
                            }
                            case 5: {
                                view_history(username);
                                break;
                            }
                            case 6: session = false; break;
                            default: std::cout << "Invalid option.\n"; break;
                        }
                    } else { // admin menu
                        switch (op) {
                            case 1: {
                                auto bal = bank.balance_of(username);
                                if (bal) std::cout << "Balance: " << *bal << "\n"; else std::cout << "Error reading balance.\n";
                                break;
                            }
                            case 2: {
                                std::cout << "Amount to deposit: "; double amt; if (!(std::cin >> amt)) { utils::clear_input(); std::cout << "Invalid amount.\n"; break; }
                                utils::clear_input(); std::string err; if (bank.deposit(username, amt, err)) std::cout << "Deposit successful.\n"; else std::cout << "Failed: " << err << "\n";
                                break;
                            }
                            case 3: {
                                std::cout << "Amount to withdraw: "; double amt; if (!(std::cin >> amt)) { utils::clear_input(); std::cout << "Invalid amount.\n"; break; }
                                utils::clear_input(); std::string err; if (bank.withdraw(username, amt, err)) std::cout << "Withdrawal successful.\n"; else std::cout << "Failed: " << err << "\n";
                                break;
                            }
                            case 4: {
                                std::string to; std::cout << "Transfer to (username): "; std::getline(std::cin, to);
                                std::cout << "Amount: "; double amt; if (!(std::cin >> amt)) { utils::clear_input(); std::cout << "Invalid amount.\n"; break; }
                                utils::clear_input(); std::string err; if (bank.transfer(username, to, amt, err)) std::cout << "Transfer successful.\n"; else std::cout << "Failed: " << err << "\n";
                                break;
                            }
                            case 5: {
                                view_history(username);
                                break;
                            }
                            case 6: {
                                std::cout << "\n-- All Accounts --\n";
                                for (const auto& p : bank.all_accounts()) {
                                    std::cout << p.first << " | Balance: " << p.second << "\n";
                                }
                                break;
                            }
                            case 7: session = false; break;
                            default: std::cout << "Invalid option.\n"; break;
                        }
                    }
                }
            } else if (choice == 3) {
                std::cout << "Goodbye!\n"; break;
            } else {
                std::cout << "Invalid option.\n";
            }
        }
    } catch (const std::exception& ex) {
        std::cerr << "Fatal error: " << ex.what() << "\n";
        return 1;
    }
    return 0;
}
