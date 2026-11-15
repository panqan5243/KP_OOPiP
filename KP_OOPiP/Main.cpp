// main.cpp - точка входа и меню авторизации
#define NOMINMAX             // windows.h иначе определяет макросы min/max
#define WIN32_LEAN_AND_MEAN  // и конфликтует с std::byte
#include <windows.h>

#include <iostream>
#include <vector>
#include "prop.h"
using namespace std;

int main() {
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);

    try {
        vector<User> users;

        // Первый запуск (файла нет): администратор по умолчанию admin / admin123
        if (!LoadUsers(users, USERS_FILE)) {
            users.push_back(User("admin", HashPassword("admin123", "admin"), 1));
            SaveUsers(users, USERS_FILE);
            cout << "Файл пользователей не найден. Создан администратор: admin / admin123\n\n";
        }

        int choice = 1;
        while (choice != 0) {
            PrintMenu("СИСТЕМА РАСЧЁТА ЗАРПЛАТЫ",
                { "1. Вход в систему", "2. Регистрация", "0. Выход из системы" });
            choice = GetInt<int>("Выберите пункт: ", 0, 2);
            cout << "\n";

            try {
                if (choice == 1) {
                    int i = LoginUser(users);
                    if (i != -1) {
                        cout << "\nДобро пожаловать, " << users[i].getLogin() << " ("
                            << RoleName(users[i].getRole()) << ")!\n\n";
                        if (users[i].getRole() == 1) AdminMenu(users, i);
                        else UserMenu(users, i);
                    }
                }
                else if (choice == 2) {
                    if (RegisterUser(users)) SaveUsers(users, USERS_FILE);
                }
            }
            catch (const InputEnded&) {
                throw;
            }
            catch (const exception& e) {
                cout << "\nОшибка: " << e.what() << "\n";  // например, не удалось записать файл
            }
            cout << "\n";
        }
        cout << "До свидания!\n";
    }
    catch (const InputEnded&) {
        cout << "\nВвод завершён. Выход.\n";
    }
    catch (const exception& e) {  // например, повреждён файл пользователей при запуске
        cout << "Ошибка: " << e.what() << "\n";
        return 1;
    }
    return 0;
}