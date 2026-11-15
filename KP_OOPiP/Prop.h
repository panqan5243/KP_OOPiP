// prop.h - прототипы функций; тела лежат в func.cpp
#pragma once
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>
#include "classes.h"
using namespace std;

const string USERS_FILE = "users.txt";

// Ввод
string ReadLine(const string& prompt);

// Ввод числа любого типа (int, double, ...) в диапазоне [lo, hi]; повторяет запрос до верного ввода.
// Шаблон должен лежать в заголовке: компилятор создаёт нужный вариант в месте вызова.
template <typename T>
T GetInt(const string& prompt, T lo = numeric_limits<T>::lowest(), T hi = numeric_limits<T>::max()) {
    while (true) {
        string s = ReadLine(prompt);
        replace(s.begin(), s.end(), ',', '.');  // 7,5 -> 7.5
        istringstream iss(s);
        T v = T();
        if ((iss >> v) && iss.peek() == char_traits<char>::eof() && v >= lo && v <= hi) return v;  // eof: после числа нет лишних символов
        if (lo == numeric_limits<T>::lowest() && hi == numeric_limits<T>::max()) {
            cout << "Ошибка. Введите число.\n";
        }
        else {
            ostringstream range;  // fixed: чтобы 1000000.0 не печаталось как 1e+06
            range << fixed << setprecision(2) << lo << " до " << hi;
            cout << "Ошибка. Введите число от " << range.str() << ".\n";
        }
    }
}

// Вывод
void PrintMenu(const string& title, const vector<string>& items);
string RoleName(int role);
string Money(double v);
double Round2(double v);

// Хеширование
unsigned long long HashPassword(const string& password, const string& login);

// Пользователи
bool IsValidLogin(const string& login);
int FindUser(const vector<User>& users, const string& login);  // индекс или -1
bool RegisterUser(vector<User>& users);                        // true, если пользователь создан
int LoginUser(const vector<User>& users);                      // индекс или -1

// Файл пользователей
bool LoadUsers(vector<User>& users, const string& filename);   // false, если файла ещё нет
void SaveUsers(const vector<User>& users, const string& filename);

// Меню после входа
void AdminMenu(vector<User>& users, int current);
void UserMenu(vector<User>& users, int current);