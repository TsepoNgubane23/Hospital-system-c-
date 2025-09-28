#pragma once
#include <string>
#include <vector>
#include "transaction.hpp"

class Account {
public:
    Account() = default;
    Account(std::string uname, std::string pass_hash, double bal = 0.0)
        : username_(std::move(uname)), password_hash_(std::move(pass_hash)), balance_(bal) {}

    const std::string& username() const { return username_; }
    const std::string& password_hash() const { return password_hash_; }
    double balance() const { return balance_; }

    void set_password_hash(const std::string& h) { password_hash_ = h; }
    void set_balance(double b) { balance_ = b; }

    void add_transaction(const Transaction& tx) { history_.push_back(tx); }
    const std::vector<Transaction>& history() const { return history_; }

private:
    std::string username_;
    std::string password_hash_;
    double balance_{0.0};
    std::vector<Transaction> history_;
};
