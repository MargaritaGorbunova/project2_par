#include <iostream>
#include <fstream>
#include <random>
#include <iomanip>
#include <vector>
#include <string>
#include <omp.h>
#include <sstream>
#include <limits>
#include <chrono>

using namespace std;
using namespace chrono;

/*
Функция для создания уникального имени файла с указанием строк и столбцов
@param numRows количество строк
@param numCols количество столбцов
@return индивидуальное имя файла
*/
inline string generateFilename(int numRows, int numCols) {
    int fileNumber = 1;
    string filename;
    while (true) {
        filename = "NumFile" + to_string(fileNumber) + "(" + to_string(numRows) + "x" + to_string(numCols) + ").txt";
        ifstream file(filename);
        if (!file) {
            break;
        }
        fileNumber++;
    }
    return filename;
}

/*
Функция для генерации случайного числа
@param gen генератор случайных чисел
@param dis распределение случайных чисел
@return случайное число
*/
inline double generateRandomNumber(mt19937& gen, uniform_real_distribution<>& dis) {
    return dis(gen);
}

/*
Функция для заполнения файла случайными числами
@param filename имя файла
@param numRows количество строк
@param numCols количество столбцов
*/
void fillFileWithRandomNumbers(const string& filename, int numRows, int numCols) {
    ofstream file(filename, ios::trunc); // Открываем для перезаписи
    if (!file) {
        cerr << "Ошибка открытия файла!" << endl;
        return;
    }

    // Генератор чисел
    random_device rd;
    mt19937 gen(rd());
    double a = -100.0;
    double b = 100.0;
    uniform_real_distribution<> dis(a, b);

    // Точность - 3 знака после запятой
    file << fixed << setprecision(3);

    vector<string> lines(numRows);

#pragma omp parallel for
    for (int i = 0; i < numRows; i++) {
        stringstream line;
        for (int j = 0; j < numCols; j++) {
            line << generateRandomNumber(gen, dis) << " ";
        }
        lines[i] = line.str();
    }

    for (const auto& line : lines) {
        file << line << "\n";
    }

    file.close();

    cout << "Файл " << filename << " заполнен случайными числами." << endl;
}

/* Функция для расчета среднего и максимального значения
@param filename имя файла
@param average среднее арифметическое
@param maxValue максимальная величина
*/
void calculateAverageAndMax(const string& filename, double& average, double& maxValue) {
    ifstream file(filename);
    if (!file) {
        cerr << "Ошибка открытия файла!" << endl;
        return;
    }

    vector<double> numbers;
    double value;
    while (file >> value) {
        numbers.push_back(value);
    }

    file.close();

    double sum = 0.0;
    maxValue = -numeric_limits<double>::infinity(); // Изначально определяем максимум как минимально возможное число
    int n = numbers.size();

#pragma omp parallel for reduction(+:sum) reduction(max:maxValue)
    for (int i = 0; i < n; i++) {
        sum += numbers[i];
        if (numbers[i] > maxValue) {
            maxValue = numbers[i];
        }
    }

    average = sum / n;
}

int main() {
    setlocale(LC_ALL, "Russian");

    vector<int> numRowsList = { 10, 50, 100, 500, 1000, 50000, 100000 };
    int numCols = 10;  // Количество чисел в строке

    for (int numRows : numRowsList) {
        // Генерация уникального имени txt-файла в формате NumFileX(STRxCOL)
        string filename = generateFilename(numRows, numCols);

        // Замер времени заполнения файла случайными числами
        auto start = high_resolution_clock::now();
        fillFileWithRandomNumbers(filename, numRows, numCols);
        auto end = high_resolution_clock::now();
        auto duration_fill = duration_cast<milliseconds>(end - start);
        cout << "Для numRows = " << numRows << ", время заполнения файла: " << duration_fill.count() << " мс" << endl;

        double average, maxValue; // среднее арифметическое, максимум

        // Замер времени вычисления среднего и максимального значений
        start = high_resolution_clock::now();
        calculateAverageAndMax(filename, average, maxValue);
        end = high_resolution_clock::now();
        auto duration_calc = duration_cast<milliseconds>(end - start);
        cout << "Для numRows = " << numRows << ", время вычисления среднего и максимального значений: " << duration_calc.count() << " мс" << endl;

        cout << "Для numRows = " << numRows << ", среднее значение: " << average << ", максимальное значение: " << maxValue << endl;
        cout << endl;
    }
    cout << "0         2045     4023  2272      1186";
    return 0;
}
