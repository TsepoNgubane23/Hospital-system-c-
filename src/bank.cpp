#include "bank.hpp"
#include "utils.hpp"

#include <algorithm>

Bank::Bank() {
    std::filesystem::create_directories(tx_dir_);
}

bool Bank::create_account(const std::string& username, const std::string& password, double initial_balance, std::string& err) {
    // Validation
    if (username.size() < 3) { err = "Username must be at least 3 characters."; return false; }
    if (password.size() < 4) { err = "Password must be at least 4 characters."; return false; }
    if (initial_balance < 0.0) { err = "Initial balance cannot be negative."; return false; }
    if (accounts_.count(username)) { err = "Username already exists."; return false; }

    std::string hash = hash_password(password);
    Account acc(username, hash, initial_balance);

    // Log initial deposit if any
    if (initial_balance > 0.0) {
        Transaction tx{current_timestamp_iso(), "DEPOSIT", initial_balance, initial_balance, "Initial deposit"};
        acc.add_transaction(tx);
        log_transaction(username, tx);
    }

    accounts_.emplace(username, acc);
    save();
    return true;
}

std::optional<Account> Bank::authenticate(const std::string& username, const std::string& password) {
    auto it = accounts_.find(username);
    if (it == accounts_.end()) return std::nullopt;
    if (it->second.password_hash() != hash_password(password)) return std::nullopt;

    // Return a copy with history loaded from file
    Account acc = it->second;
    load_transactions_for(username, acc);
    return acc;
}

bool Bank::deposit(const std::string& username, double amount, std::string& err) {
    if (amount <= 0.0) { err = "Amount must be positive."; return false; }
    auto it = accounts_.find(username);
    if (it == accounts_.end()) { err = "Account not found."; return false; }

    double new_bal = it->second.balance() + amount;
    it->second.set_balance(new_bal);
    Transaction tx{current_timestamp_iso(), "DEPOSIT", amount, new_bal, "Cash deposit"};
    log_transaction(username, tx);
    save();
    return true;
}

bool Bank::withdraw(const std::string& username, double amount, std::string& err) {
    if (amount <= 0.0) { err = "Amount must be positive."; return false; }
    auto it = accounts_.find(username);
    if (it == accounts_.end()) { err = "Account not found."; return false; }
    if (it->second.balance() < amount) { err = "Insufficient funds."; return false; }

    double new_bal = it->second.balance() - amount;
    it->second.set_balance(new_bal);
    Transaction tx{current_timestamp_iso(), "WITHDRAW", amount, new_bal, "Cash withdrawal"};
    log_transaction(username, tx);
    save();
    return true;
}

bool Bank::transfer(const std::string& from_user, const std::string& to_user, double amount, std::string& err) {
    if (from_user == to_user) { err = "Cannot transfer to the same account."; return false; }
    if (amount <= 0.0) { err = "Amount must be positive."; return false; }

    auto from_it = accounts_.find(from_user);
    auto to_it = accounts_.find(to_user);
    if (from_it == accounts_.end()) { err = "Source account not found."; return false; }
    if (to_it == accounts_.end()) { err = "Destination account not found."; return false; }
    if (from_it->second.balance() < amount) { err = "Insufficient funds."; return false; }

    // Perform transfer atomically in-memory
    double from_new = from_it->second.balance() - amount;
    double to_new = to_it->second.balance() + amount;
    from_it->second.set_balance(from_new);
    to_it->second.set_balance(to_new);

    Transaction out_tx{current_timestamp_iso(), "TRANSFER_OUT", amount, from_new, std::string("To ") + to_user};
    Transaction in_tx{current_timestamp_iso(), "TRANSFER_IN", amount, to_new, std::string("From ") + from_user};
    log_transaction(from_user, out_tx);
    log_transaction(to_user, in_tx);

    save();
    return true;
}

std::optional<double> Bank::balance_of(const std::string& username) const {
    auto it = accounts_.find(username);
    if (it == accounts_.end()) return std::nullopt;
    return it->second.balance();
}

bool Bank::has_user(const std::string& username) const {
    return accounts_.count(username) != 0;
}

std::vector<std::pair<std::string,double>> Bank::all_accounts() const {
    std::vector<std::pair<std::string,double>> v;
    v.reserve(accounts_.size());
    for (const auto& kv : accounts_) {
        v.emplace_back(kv.first, kv.second.balance());
    }
    std::sort(v.begin(), v.end(), [](const auto& a, const auto& b){ return a.first < b.first; });
    return v;
}

void Bank::load() {
    accounts_.clear();
    std::filesystem::create_directories(db_path_.parent_path());
    std::ifstream in(db_path_);
    if (!in) {
        // create default admin if file missing
        accounts_.emplace("admin", Account("admin", hash_password("admin"), 0.0));
        save();
        return;
    }
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        std::istringstream iss(line);
        std::string username, hash, bal_str;
        if (std::getline(iss, username, '|') && std::getline(iss, hash, '|') && std::getline(iss, bal_str)) {
            try {
                double bal = std::stod(bal_str);
                accounts_.emplace(username, Account(username, hash, bal));
            } catch (...) {
                // skip malformed
            }
        }
    }
    // Ensure admin exists
    if (!has_user("admin")) {
        accounts_.emplace("admin", Account("admin", hash_password("admin"), 0.0));
        save();
    }
}

void Bank::save() const {
    std::filesystem::create_directories(db_path_.parent_path());
    std::ofstream out(db_path_, std::ios::trunc);
    for (const auto& kv : accounts_) {
        out << kv.first << '|' << kv.second.password_hash() << '|' << kv.second.balance() << '\n';
    }
}

void Bank::log_transaction(const std::string& username, const Transaction& tx) const {
    std::filesystem::create_directories(tx_dir_);
    std::ofstream out(tx_dir_ / (username + ".log"), std::ios::app);
    out << tx.timestamp << '|' << tx.type << '|' << tx.amount << '|' << tx.balance_after << '|' << tx.details << '\n';
}

void Bank::load_transactions_for(const std::string& username, Account& acc) {
    acc = accounts_.at(username); // ensure balance/hash from map
    std::ifstream in(tx_dir_ / (username + ".log"));
    if (!in) return;
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        std::istringstream iss(line);
        Transaction tx; std::string amount_str, bal_str;
        if (std::getline(iss, tx.timestamp, '|') && std::getline(iss, tx.type, '|') &&
            std::getline(iss, amount_str, '|') && std::getline(iss, bal_str, '|') && std::getline(iss, tx.details)) {
            try {
                tx.amount = std::stod(amount_str);
                tx.balance_after = std::stod(bal_str);
                acc.add_transaction(tx);
            } catch (...) { /* skip */ }
        }
    }
}
