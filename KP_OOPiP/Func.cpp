// func.cpp - реализации функций
#include <cctype>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include "prop.h"
using namespace std;

// Память под статические поля выделяется здесь, один раз.
// Порог годового дохода и ставка доплаты - примерные значения, уточнить у преподавателя.
int Employee::nextId_ = 1;
double Employee::baseRate_ = 0.13;
double Employee::highThreshold_ = 1200000.0;
double Employee::highRate_ = 0.05;

// ---------------- Ввод ----------------

// Если ввод закончился, бросаем InputEnded: иначе циклы повторного ввода зациклились бы навсегда
string ReadLine(const string& prompt) {
    cout << prompt;
    string s;
    if (!getline(cin, s)) throw InputEnded();
    return s;
}

// ---------------- Вывод ----------------

// В кодировке 1251 один символ = один байт, поэтому setw ровно выравнивает и русский текст
void PrintMenu(const string& title, const vector<string>& items) {
    cout << string(40, '-') << "\n";
    cout << "| " << left << setw(37) << title << "|\n";
    cout << string(40, '-') << "\n";
    for (size_t i = 0; i < items.size(); ++i) cout << "| " << left << setw(37) << items[i] << "|\n";
    cout << string(40, '-') << "\n";
}

string RoleName(int role) { return role == 1 ? "администратор" : "пользователь"; }

double Round2(double v) { return round(v * 100.0) / 100.0; }

string Money(double v) {
    ostringstream os;
    os << fixed << setprecision(2) << v;
    return os.str();
}

ostream& operator<<(ostream& os, const User& u) {
    os << left << setw(22) << u.login_ << RoleName(u.role_);
    return os;
}

// ---------------- Сотрудники ----------------

double Employee::calcTax(double monthlyGross) {
    double tax = monthlyGross * baseRate_;
    double annual = monthlyGross * 12;
    if (annual > highThreshold_) tax += (annual - highThreshold_) / 12 * highRate_;
    return Round2(tax);
}

double Employee::tax() const { return calcTax(gross()); }
double Employee::net() const { return Round2(gross() - tax()); }

string SalariedEmployee::details() const {
    return "оклад " + Money(getSalary()) + ", премия " + Money(getBonus());
}

string Manager::details() const {
    return SalariedEmployee::details() + ", надбавка " + Money(allowance_);
}

unique_ptr<Employee> EmployeeFactory::create(int type) {
    switch (type) {
    case Salaried: return unique_ptr<Employee>(new SalariedEmployee());
    case ManagerType: return unique_ptr<Employee>(new Manager());
    default: throw ValidationException("неизвестный тип сотрудника: " + to_string(type));
    }
}

string EmployeeFactory::typeTitle(int type) { return create(type)->typeName(); }

// ---------------- Хеширование ----------------

// djb2: hash = hash * 33 + код символа. Логин подмешивается к паролю как соль,
// чтобы одинаковые пароли у разных пользователей давали разные хеши.
unsigned long long HashPassword(const string& password, const string& login) {
    unsigned long long hash = 6180;
    const string data = login + password;
    for (size_t i = 0; i < data.size(); ++i)
        hash = ((hash << 5) + hash) + static_cast<unsigned char>(data[i]);
    return hash;
}

// ---------------- Пользователи ----------------

bool IsValidLogin(const string& login) {
    if (login.empty() || login.size() > 20) return false;
    for (size_t i = 0; i < login.size(); ++i) {
        unsigned char c = static_cast<unsigned char>(login[i]);
        if (!isalnum(c) && c != '_') return false;
    }
    return true;
}

int FindUser(const vector<User>& users, const string& login) {
    for (size_t i = 0; i < users.size(); ++i)
        if (users[i].getLogin() == login) return static_cast<int>(i);
    return -1;
}

bool RegisterUser(vector<User>& users) {
    string login = ReadLine("Логин (до 20 символов): ");
    if (!IsValidLogin(login)) {
        cout << "Ошибка: логин 1-20 символов, допустимы латинские буквы, цифры и '_'.\n";
        return false;
    }
    if (FindUser(users, login) != -1) {
        cout << "Ошибка: такой логин уже занят.\n";
        return false;
    }

    string password = ReadLine("Пароль (от 4 до 20 символов): ");
    if (password.size() < 4 || password.size() > 20) {
        cout << "Ошибка: пароль должен быть длиной от 4 до 20 символов.\n";
        return false;
    }

    // Код 112 даёт права администратора
    int code = GetInt<int>("Введите код (если не знаете - введите 0): ", 0, 999999);
    int role = (code == 112) ? 1 : 0;

    users.push_back(User(login, HashPassword(password, login), role));
    cout << "Пользователь создан, роль: " << RoleName(role) << ".\n";
    return true;
}

int LoginUser(const vector<User>& users) {
    for (int attempt = 1; attempt <= 3; ++attempt) {
        string login = ReadLine("Логин: ");
        string password = ReadLine("Пароль: ");
        int i = FindUser(users, login);
        // одно и то же сообщение для неверного логина и неверного пароля: нельзя подбирать существующие логины
        if (i != -1 && HashPassword(password, login) == users[i].getHash()) return i;
        cout << "Неверный логин или пароль (попытка " << attempt << " из 3).\n";
    }
    return -1;
}

// ---------------- Файл пользователей ----------------

// Формат строки: логин;хеш;роль. Нет файла - false (первый запуск).
// Файл есть, но повреждён - исключение, чтобы следующее сохранение не затёрло данные.
bool LoadUsers(vector<User>& users, const string& filename) {
    ifstream in(filename);
    if (!in.is_open()) return false;

    vector<User> loaded;  // при ошибке исходный вектор не меняется
    string line;
    int lineNo = 0;
    while (getline(in, line)) {
        ++lineNo;
        if (line.empty()) continue;
        const string where = filename + ", строка " + to_string(lineNo) + ": ";

        stringstream ss(line);
        string login, hashStr, roleStr, extra;
        if (!getline(ss, login, ';') || !getline(ss, hashStr, ';') || !getline(ss, roleStr, ';') || getline(ss, extra, ';'))
            throw runtime_error(where + "ожидалось 3 поля (логин;хеш;роль)");
        if (!IsValidLogin(login)) throw runtime_error(where + "недопустимый логин");
        if (roleStr != "0" && roleStr != "1") throw runtime_error(where + "роль должна быть 0 или 1");

        unsigned long long hash = 0;
        try {
            size_t pos = 0;
            hash = stoull(hashStr, &pos);
            if (pos != hashStr.size()) throw invalid_argument("лишние символы");
        }
        catch (const logic_error&) {  // invalid_argument и out_of_range
            throw runtime_error(where + "неверный хеш");
        }

        if (FindUser(loaded, login) != -1) throw runtime_error(where + "повторяющийся логин");
        loaded.push_back(User(login, hash, roleStr == "1" ? 1 : 0));
    }
    if (in.bad()) throw runtime_error("Ошибка чтения файла «" + filename + "»");
    if (loaded.empty()) throw runtime_error("Файл «" + filename + "» не содержит пользователей");

    users = loaded;
    return true;
}

void SaveUsers(const vector<User>& users, const string& filename) {
    ofstream out(filename, ios::trunc);
    if (!out.is_open()) throw runtime_error("Не удалось открыть файл «" + filename + "» для записи");
    for (size_t i = 0; i < users.size(); ++i)
        out << users[i].getLogin() << ';' << users[i].getHash() << ';' << users[i].getRole() << '\n';
    out.flush();
    if (!out.good()) throw runtime_error("Ошибка записи в файл «" + filename + "»");
}

// ---------------- Меню администратора ----------------
void AdminMenu(vector<User>& users, int current) {
    (void)users;  // пока не используются
    (void)current;

    int choice = -1;
    while (choice != 0) {
        PrintMenu("МЕНЮ АДМИНИСТРАТОРА",
            { "1. Просмотр сотрудников", "2. Добавить сотрудника", "3. Редактировать сотрудника",
             "4. Удалить сотрудника", "5. Поиск", "6. Сортировка", "7. Фильтрация",
             "8. Ведомость (отчёт)", "9. Ставка налога", "0. Разлогиниться" });
        choice = GetInt<int>("Выберите пункт: ", 0, 9);
        cout << "\n";

        try {
            switch (choice) {
            case 1:
                // ПРОСМОТР: взять контейнер сотрудников, отсортировать по ID и вывести
                // таблицей: ID, ФИО, отдел, тип, начислено, налог, к выдаче.
                // Если контейнер пуст - написать "Записей нет".
                cout << "[в разработке]\n";
                break;
            case 2:
                // ДОБАВЛЕНИЕ: выбрать тип сотрудника (оклад / руководитель), создать объект
                // через фабрику, запросить поля у пользователя (ФИО, отдел, должность и поля
                // типа), присвоить ID, положить в контейнер, сохранить файл.
                cout << "[в разработке]\n";
                break;
            case 3:
                // РЕДАКТИРОВАНИЕ: спросить ID, найти сотрудника (нет - сообщение об ошибке),
                // показать карточку, запросить новые значения полей, сохранить файл.
                cout << "[в разработке]\n";
                break;
            case 4:
                // УДАЛЕНИЕ: спросить ID, показать карточку, переспросить "Удалить? 1/0",
                // удалить из контейнера, сохранить файл.
                cout << "[в разработке]\n";
                break;
            case 5:
                // ПОИСК: подменю "по ID" (точное совпадение) и "по части ФИО"
                // (без учёта регистра). Результат - карточка или таблица.
                cout << "[в разработке]\n";
                break;
            case 6:
                // СОРТИРОВКА: выбрать поле (ФИО, ID, отдел, начислено, к выдаче) и порядок
                // (по возрастанию / по убыванию), отсортировать копию списка сортировкой
                // Шелла и вывести таблицей.
                cout << "[в разработке]\n";
                break;
            case 7:
                // ФИЛЬТРАЦИЯ: подменю "по типу", "по отделу", "по диапазону начислений".
                // Вывести только подходящие записи.
                cout << "[в разработке]\n";
                break;
            case 8:
                // ВЕДОМОСТЬ (реализация "заказа"): отчёт по всем сотрудникам с итогами
                // (начислено, налог, к выдаче). Вывести на экран и записать в report.txt.
                cout << "[в разработке]\n";
                break;
            case 9:
                // НАЛОГ: показать правила (базовая ставка, порог годового дохода, ставка
                // доплаты), запросить новую базовую ставку (0-100%), применить через
                // Employee::setBaseRate, сохранить в файл сотрудников.
                cout << "[в разработке]\n";
                break;
            default:  // 0 - выход, цикл завершится сам
                break;
            }
        }
        catch (const InputEnded&) {
            throw;  // программа завершается
        }
        catch (const exception& e) {
            cout << "\nОшибка: " << e.what() << "\n";  // остальные ошибки: сообщаем и продолжаем
        }
    }
}

// ---------------- Меню пользователя ----------------
void UserMenu(vector<User>& users, int current) {
    (void)users;
    (void)current;

    int choice = -1;
    while (choice != 0) {
        PrintMenu("МЕНЮ ПОЛЬЗОВАТЕЛЯ",
            { "1. Просмотр сотрудников", "2. Поиск", "3. Сортировка", "4. Фильтрация",
             "5. Ведомость (отчёт)", "6. Сменить пароль", "0. Разлогиниться" });
        choice = GetInt<int>("Выберите пункт: ", 0, 6);
        cout << "\n";

        try {
            switch (choice) {
            case 1:
            case 2:
            case 3:
            case 4:
            case 5:
                // Те же действия, что у администратора (пункты 1, 5, 6, 7, 8), только чтение:
                // добавлять, изменять и удалять записи пользователь не может.
                cout << "[в разработке]\n";
                break;
            case 6:
                // СМЕНА ПАРОЛЯ: запросить старый пароль и проверить его хешем,
                // запросить новый (4-20 символов), записать новый хеш,
                // вызвать SaveUsers(users, USERS_FILE).
                cout << "[в разработке]\n";
                break;
            default:  // 0 - выход
                break;
            }
        }
        catch (const InputEnded&) {
            throw;
        }
        catch (const exception& e) {
            cout << "\nОшибка: " << e.what() << "\n";
        }
    }
}