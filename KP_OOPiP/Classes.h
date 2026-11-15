// classes.h - классы проекта
#pragma once
#include <memory>
#include <ostream>
#include <stdexcept>
#include <string>
using namespace std;

// Ввод закончился: отличает "пора завершить программу" от обычных ошибок
class InputEnded : public runtime_error {
public:
    InputEnded() : runtime_error("Ввод завершён") {}
};

// Некорректные данные (пустое поле, число вне диапазона и т.п.)
class ValidationException : public runtime_error {
public:
    explicit ValidationException(const string& m) : runtime_error("Некорректные данные: " + m) {}
};

class User {
    string login_;
    unsigned long long hash_;  // хеш пароля, сам пароль не хранится
    int role_;                 // 0 - пользователь, 1 - администратор
public:
    User(const string& login, unsigned long long hash, int role) : login_(login), hash_(hash), role_(role) {}

    const string& getLogin() const { return login_; }
    unsigned long long getHash() const { return hash_; }
    int getRole() const { return role_; }

    friend ostream& operator<<(ostream& os, const User& u);  // логин и роль, хеш не выводится
};

// Абстрактный базовый класс всех сотрудников
class Employee {
    int id_;
    string name_;
    string department_;
    string position_;

    // Общие для всех сотрудников статические поля (определения в func.cpp)
    static int nextId_;
    static double baseRate_;       // базовая ставка налога, доля (0.13 = 13%)
    static double highThreshold_;  // порог годового дохода, свыше которого действует доплата
    static double highRate_;       // ставка доплаты с части годового дохода сверх порога

protected:
    static void checkField(const string& value, const string& what, size_t maxLen) {
        if (value.empty()) throw ValidationException(what + " не может быть пустым");
        if (value.size() > maxLen) throw ValidationException(what + " длиннее " + to_string(maxLen) + " символов");
        if (value.find(';') != string::npos) throw ValidationException(what + ": символ ';' недопустим");  // разделитель в файле
    }
    static void checkMoney(double v, const string& what) {
        if (!(v >= 0.0 && v <= 1000000.0)) throw ValidationException("поле «" + what + "» должно быть от 0 до 1000000");  // !(...) отсекает и NaN
    }

public:
    Employee() : id_(0) {}
    Employee(const string& name, const string& department, const string& position) : id_(0) {
        setName(name);
        setDepartment(department);
        setPosition(position);
    }

    virtual ~Employee() {}  // нужен для delete через указатель на базовый класс

    virtual double gross() const = 0;     // начислено до налога (за месяц)
    virtual string typeName() const = 0;  // название типа для таблицы
    virtual string details() const = 0;   // параметры расчёта текстом

    double tax() const;  // налог по правилам calcTax
    double net() const;  // к выдаче: начислено минус налог

    int getId() const { return id_; }
    const string& getName() const { return name_; }
    const string& getDepartment() const { return department_; }
    const string& getPosition() const { return position_; }

    void setId(int id) { id_ = id; }
    void setName(const string& n) { checkField(n, "ФИО", 24); name_ = n; }
    void setDepartment(const string& d) { checkField(d, "Отдел", 12); department_ = d; }
    void setPosition(const string& p) { checkField(p, "Должность", 20); position_ = p; }

    static int generateId() { return nextId_++; }
    static void syncId(int maxId) { if (maxId >= nextId_) nextId_ = maxId + 1; }  // после чтения из файла

    // Налог с месячного начисления: базовая ставка со всей суммы, плюс доплата
    // с той части годового дохода (начисление * 12), которая превышает порог
    static double calcTax(double monthlyGross);
    static double getBaseRate() { return baseRate_; }
    static double getHighThreshold() { return highThreshold_; }
    static double getHighRate() { return highRate_; }
    static void setBaseRate(double r) {
        if (!(r >= 0.0 && r <= 1.0)) throw ValidationException("ставка налога должна быть от 0 до 100%");
        baseRate_ = r;
    }
};

// Сотрудник на окладе: оклад + премия
class SalariedEmployee : public Employee {
    double salary_;
    double bonus_;
public:
    SalariedEmployee() : salary_(0), bonus_(0) {}
    SalariedEmployee(const string& name, const string& department, const string& position, double salary, double bonus)
        : Employee(name, department, position), salary_(0), bonus_(0) {
        setSalary(salary);
        setBonus(bonus);
    }

    double gross() const override { return salary_ + bonus_; }
    string typeName() const override { return "Оклад"; }
    string details() const override;

    double getSalary() const { return salary_; }
    double getBonus() const { return bonus_; }
    void setSalary(double s) { checkMoney(s, "Оклад"); salary_ = s; }
    void setBonus(double b) { checkMoney(b, "Премия"); bonus_ = b; }
};

// Руководитель: оклад + премия + надбавка за руководство (3-й уровень наследования)
class Manager : public SalariedEmployee {
    double allowance_;
public:
    Manager() : allowance_(0) {}
    Manager(const string& name, const string& department, const string& position, double salary, double bonus,
        double allowance)
        : SalariedEmployee(name, department, position, salary, bonus), allowance_(0) {
        setAllowance(allowance);
    }

    double gross() const override { return SalariedEmployee::gross() + allowance_; }
    string typeName() const override { return "Руководитель"; }
    string details() const override;

    double getAllowance() const { return allowance_; }
    void setAllowance(double a) { checkMoney(a, "Надбавка"); allowance_ = a; }
};

// Фабрика (паттерн "Фабричный метод"): создаёт пустого сотрудника нужного типа,
// чтобы остальной код не зависел от конкретных классов
class EmployeeFactory {
public:
    enum Type { Salaried = 1, ManagerType = 2 };

    static unique_ptr<Employee> create(int type);  // неизвестный тип - ValidationException
    static string typeTitle(int type);             // название типа для меню и файла
};