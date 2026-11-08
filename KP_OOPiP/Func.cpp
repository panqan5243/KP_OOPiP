#include <cctype>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include "prop.h"
using namespace std;

string ReadLine(const string& prompt) {
    cout << prompt;
    string s;
    if (!getline(cin, s)) throw runtime_error("Ввод завершён");
    return s;
}


int GetInt(const string& prompt, int lo, int hi) {
    while (true) {
        string s = ReadLine(prompt);
        try {
            size_t pos = 0;
            int v = stoi(s, &pos);  
            if (pos == s.size() && v >= lo && v <= hi) return v; 
        }
        catch (const exception&) {
            
        }
        cout << "Ошибка. Введите число от " << lo << " до " << hi << ".\n";
    }
}


unsigned long long HashPassword(const string& password, const string& login) {
    unsigned long long hash = 6180;
    const string data = login + password;
    for (size_t i = 0; i < data.size(); ++i)
        hash = ((hash << 5) + hash) + static_cast<unsigned char>(data[i]);
    return hash;
}

int FindUser(const vector<User>& users, const string& login) {
    for (size_t i = 0; i < users.size(); ++i)
        if (users[i].getLogin() == login) return static_cast<int>(i);
    return -1;
}

void RegisterUser(vector<User>& users) {
    string login = ReadLine("Логин (до 20 символов): ");
    if (login.empty() || login.size() > 20) {
        cout << "Ошибка: логин должен быть длиной от 1 до 20 символов.\n";
        return;
    }
    for (size_t i = 0; i < login.size(); ++i) {
        unsigned char c = static_cast<unsigned char>(login[i]);
        if (!isalnum(c) && c != '_') {
            cout << "Ошибка: в логине допустимы латинские буквы, цифры и '_'.\n";
            return;
        }
    }
    if (FindUser(users, login) != -1) {
        cout << "Ошибка: такой логин уже занят.\n";
        return;
    }

    string password = ReadLine("Пароль (от 4 до 20 символов): ");
    if (password.size() < 4 || password.size() > 20) {
        cout << "Ошибка: пароль должен быть длиной от 4 до 20 символов.\n";
        return;
    }

    // Код 112 даёт права администратора
    int code = GetInt("Введите код (если не знаете - введите 0): ", 0, 999999);
    int role = (code == 112) ? 1 : 0;

    unsigned long long hash = HashPassword(password, login);
    users.push_back(User(login, hash, role));
    cout << "Пользователь создан, роль: " << (role == 1 ? "администратор" : "пользователь") << ".\n";
    cout << "[отладка] хеш пароля: " << hash << "\n";  // временно, чтобы видеть результат хеширования
}

// Три попытки ввода. Возвращает индекс пользователя в векторе или -1.
int LoginUser(const vector<User>& users) {
    for (int attempt = 1; attempt <= 3; ++attempt) {
        string login = ReadLine("Логин: ");
        string password = ReadLine("Пароль: ");
        int i = FindUser(users, login);
        if (i != -1 && HashPassword(password, login) == users[i].getHash()) return i;
        cout << "Неверный логин или пароль (попытка " << attempt << " из 3).\n";
    }
    return -1;
}