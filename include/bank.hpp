#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <optional>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <iostream>

#include "account.hpp"
#include "transaction.hpp"
#include "sha256.hpp"

class Bank {
public:
    Bank();

    bool create_account(const std::string& username, const std::string& password, double initial_balance, std::string& err);
    std::optional<Account> authenticate(const std::string& username, const std::string& password);

    bool deposit(const std::string& username, double amount, std::string& err);
    bool withdraw(const std::string& username, double amount, std::string& err);
    bool transfer(const std::string& from_user, const std::string& to_user, double amount, std::string& err);

    std::optional<double> balance_of(const std::string& username) const;
    bool has_user(const std::string& username) const;
    bool is_admin(const std::string& username) const { return username == "admin"; }

    std::vector<std::pair<std::string,double>> all_accounts() const; // for admin view

    void load();
    void save() const;

    static std::string hash_password(const std::string& password) {
        return picosha2::hash256_hex_string(password);
    }

private:
    void log_transaction(const std::string& username, const Transaction& tx) const;
    void load_transactions_for(const std::string& username, Account& acc);

    std::unordered_map<std::string, Account> accounts_;
    std::filesystem::path db_path_ = "data/accounts.db";
    std::filesystem::path tx_dir_ = "data/transactions";
};
