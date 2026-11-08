#pragma once
#include <string>
using namespace std;

class User {
    string login_;
    unsigned long long hash_;  // хеш пароля
    int role_;                 // 0 - пользователь, 1 - администратор
public:
    User(const string& login, unsigned long long hash, int role)
        : login_(login), hash_(hash), role_(role) {
    }

    // Методы доступа (геттеры)
    const string& getLogin() const { return login_; }
    unsigned long long getHash() const { return hash_; }
    int getRole() const { return role_; }
};
