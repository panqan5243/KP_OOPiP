#define NOMINMAX             // windows.h не должен определять макросы min/max
#define WIN32_LEAN_AND_MEAN  
#include <windows.h>

#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <vector>
#include "Func.cpp"
#include "Prop.h"
using namespace std;

int main() {
    SetConsoleCP(1251);       
    SetConsoleOutputCP(1251); 

    try {
        vector<User> users;  // пока пользователи живут только в памяти (файлы добавим позже)
        // Начальный администратор, чтобы было с чего войти: admin / admin123
        users.push_back(User("admin", HashPassword("admin123", "admin"), 1));

        int choice = 1;
        while (choice != 0) {
            cout << "----------------------------------------\n";
            cout << "| " << left << setw(37) << "СИСТЕМА РАСЧЁТА ЗАРПЛАТЫ" << "|\n";
            cout << "----------------------------------------\n";
            cout << "| " << left << setw(37) << "1. Вход в систему" << "|\n";
            cout << "| " << left << setw(37) << "2. Регистрация" << "|\n";
            cout << "| " << left << setw(37) << "0. Выход из системы" << "|\n";
            cout << "----------------------------------------\n";
            choice = GetInt("Выберите пункт: ", 0, 2);
            cout << "\n";

            if (choice == 1) {
                int i = LoginUser(users);
                if (i != -1) {
                    cout << "\nДобро пожаловать, " << users[i].getLogin() << " ("
                        << (users[i].getRole() == 1 ? "администратор" : "пользователь") << ")!\n";
                    // Здесь позже появится меню администратора \ пользователя
                }
            }
            else if (choice == 2) {
                RegisterUser(users);
            }
            cout << "\n";
        }
        cout << "До свидания!\n";
    }
    catch (const runtime_error& e) {  
        cout << "\n" << e.what() << ". Выход.\n";
    }
    return 0;
}