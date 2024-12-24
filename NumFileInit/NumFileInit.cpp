#include <iostream>
#include <fstream>
#include <random>
#include <iomanip>
#include <vector>
#include <string>
#include <omp.h>
#include <sstream>
#include <limits>

using namespace std;

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

#pragma omp parallel for
    for (int i = 0; i < numRows; i++) {
        stringstream line;
        for (int j = 0; j < numCols; j++) {
            line << generateRandomNumber(gen, dis) << " ";
        }
#pragma omp critical
        {
            file << line.str() << "\n";
        }
    }

    file.close();

    cout << "Файл " << filename << " заполнен случайными числами." << endl;
}

/* Функция для расчета среднего и максимального значения
@param filename имя файла
@param average среднее арифметическое
@param maxValue масимальная величина
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
    int numRows = 100000; // Количество строк
    int numCols = 10;  // Количество чисел в строке

    // Генерация уникального имени txt-файла в формате NumFileX(STRxCOL)
    string filename = generateFilename(numRows, numCols);

    // Заполнение файла заданного размера случайными числами с плавающей точкой
    fillFileWithRandomNumbers(filename, numRows, numCols);

    double average, maxValue; // среднее арифметическое, максимум

    // Вычисление среднего и максимального значений
    calculateAverageAndMax(filename, average, maxValue);

    cout << "Среднее значение: " << average << endl;
    cout << "Максимальное значение: " << maxValue << endl;

    return 0;
}
