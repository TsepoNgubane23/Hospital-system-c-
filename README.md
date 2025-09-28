# C++ Banking System (C++17)

A simple, secure, file-backed banking system written in modern C++17.

Features:
- Object-oriented design: `Bank`, `Account`, `Transaction`
- Create accounts (username, password, initial balance)
- Secure login (passwords hashed using SHA-256)
- Deposit, withdraw, check balance, transfer between accounts
- Per-account transaction history persisted to disk
- Input validation and meaningful error messages
- Text-based menu interface
- Windows password masking (no echo); POSIX fallback
- Optional admin user: `admin` can view all accounts and balances

## Build Instructions

Requires a C++17 compiler.

- MinGW (g++):
  ```bash
  g++ -std=c++17 -O2 -Iinclude src/*.cpp -o banking_app
  ```

- MSVC (Developer Command Prompt):
  ```bat
  cl /std:c++17 /EHsc /I include src\*.cpp /Fe:banking_app.exe
  ```

Run:
```bash
./banking_app
```

Data files are stored under `data/`:
- `data/accounts.db` — `username|passwordHash|balance`
- `data/transactions/<username>.log` — one transaction per line

## Notes
- Passwords are hashed (SHA-256) using an embedded header-only implementation (picosha2).
- The program creates the `data/` and `data/transactions/` directories on startup if missing.
- Admin functionality is simple: when logged in as `admin`, you can view all accounts.
