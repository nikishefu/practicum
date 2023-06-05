#include <iostream>
#include <vector>
#include <iomanip>
#include <algorithm>
#include <cmath>

using namespace std;

double func(double x) {return sin(x) - x * x / 2;}

// Класс, позволяющий создать функцию сравнения двух пар (точек) по их удаленности от
// точки x0, при этом без передачи x0 как параметра функции сравнения
// Это нужно чтобы функция сравнения соответствовала формату для std::sort()
// то есть мы можем передать ровно 2 параметра - 2 объекта, которые сравниваются
class CloserTo {
public:
    CloserTo(double x0) {
        this->x0 = x0;
    }
    bool operator() (pair<double, double> &p1, pair<double, double> &p2) {return abs(p1.first - x0) < abs(p2.first - x0);}
private:
    double x0;
};

/// @brief Генерация таблицы со значениями с фиксированным шагом
vector<pair<double, double>> generateTable(double a, double b, uint16_t nodeCount) {
    
    vector<pair<double, double>> table;
    double step = (b - a) / (nodeCount - 1);
    double x, y;

    cout << fixed << setprecision(10);
    cout << "Table" << endl;

    for (uint8_t i = 0; i < nodeCount - 1; ++i) {
        x = a + i * step;
        y = func(x);
        table.push_back(make_pair(x, y));
        cout << x << " " << y << endl;
    }

    x = b;
    y = func(x);
    table.push_back(make_pair(x, y));
    cout << x << " " << y << endl;

    return table;
}

void calculateValues(vector<pair<double, double>> &table) {
    uint16_t n;
    double x0;

    cout << "Enter x where Pn(x) will be calculated: ";
    cin >> x0;

    cout << "Enter power n of Pn(x): ";
    cin >> n;
    while (n >= table.size()) {
        cout << "Power must be less than number of nodes, try again: ";
        cin >> n;
    }

    sort(table.begin(), table.end(), CloserTo(x0));
    cout << "\nTable sorted by distance to " << x0 << endl;
    for (auto point : table) {cout << point.first << " " << point.second << endl;}


    // В форме Лагранжа
    double sum = 0, summand;
    cout << "\n=====Lagrange Polynomial=====\nL(x) = ";
    
    for (uint16_t k = 0; k < n; ++k) {
        summand = table.at(k).second;  // f(Xk)
        cout << table.at(k).second;
        for (uint16_t i = 0; i < n; ++i) {
            if (i == k) continue;
            summand *= (x0 - table.at(i).first) / (table.at(k).first - table.at(i).first);
            cout << " * (x - " << table.at(i).first << ") / " << table.at(k).first - table.at(i).first;
        }
        if (k < n - 1) cout << " +\n+ ";
        else cout << "\n=================\n\n";
        sum += summand;
    }
    cout << "Lagrange form: Pn(x0) = " << sum << endl;
    cout << "Error: " << abs(sum - func(x0)) << endl;

    
    // В форме Ньютона
    // Таблица разделенных разностей
    vector<vector<double>> diffTable(table.size() - 1);

    // Разности 1-го порядка
    diffTable.at(0) = vector<double>(diffTable.size());
    for (uint16_t i = 0; i < diffTable.at(0).size(); ++i) {
        diffTable.at(0).at(i) = (table.at(i + 1).second - table.at(i).second)/(table.at(i + 1).first - table.at(i).first);
    }

    // Разности порядка > 1
    for (uint16_t i = 1; i < diffTable.size(); ++i) {
        diffTable.at(i) = vector<double>(diffTable.size() - i);
        for (uint16_t j = 0; j < diffTable.at(i).size(); ++j) {
            diffTable.at(i).at(j) = (diffTable.at(i - 1).at(j + 1) - diffTable.at(i - 1).at(j)) / (table.at(j + 1 + i).first - table.at(j).first);
        }
    }
    
    // Вычисление по схеме Горнера
    sum = diffTable.at(n - 1).at(0), summand;
    for (uint16_t k = n - 1; k > 0; --k) {
        sum *= x0 - table.at(k).first;
        sum += diffTable.at(k - 1).at(0);
    }
    sum *= x0 - table.at(0).first;
    sum += table.at(0).second;

    cout << "Newton form: Pn(x0) = " << sum << endl;
    cout << "Error: " << abs(sum - func(x0)) << endl;
}

int main() {
    uint16_t nodeCount;
    double a, b;

    cout << "=== Task 2: The problem of algebraic interpolation ===\n";
    cout << "=== Variant 1 ===\n\n";
    cout << "Enter number of nodes (m + 1): ";
    cin >> nodeCount;  // m + 1

    cout << "Enter boundaries (a and b): ";
    cin >> a >> b;
    cout << a << " " << b << endl;
    while (a >= b) {
        cout << "b must be greater than a, try again: ";
        cin >> a >> b;
    }


    auto table = generateTable(a, b, nodeCount);

    string ans;
    do {
        calculateValues(table);
        cout << "Do you want to calculate with other x0 and n? (yes/No): ";
        cin >> ans;
    } while (ans == "yes");

    return 0;
}