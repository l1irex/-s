#include "filesystem.h"
#include <iostream>

using namespace std;

int main() {
    setlocale(LC_ALL, "ru_RU.UTF-8");
    FileSystem fs;

    cout << "=== ДЕМОНСТРАЦИЯ РАБОТЫ (Вариант 8) ===\n";
    
    fs.create_dir("/docs");
    fs.create_file("/docs/report.txt");
    fs.create_file("/notes.txt");
    
    fs.write_file("/docs/report.txt", 
        "Отчет по лабораторной работе №4. Вариант 8.");
    fs.write_file("/notes.txt", "Краткие заметки.");

    cout << "\n--- Чтение /docs/report.txt ---\n";
    cout << fs.read_file("/docs/report.txt") << "\n";

    fs.copy("/docs/report.txt", "/copy.txt");
    fs.move("/notes.txt", "/docs/notes_moved.txt");
    fs.remove("/copy.txt");

    fs.dump();
    fs.load_test();

    cout << "\n=== ПРОГРАММА ЗАВЕРШЕНА ===\n";
    return 0;
}