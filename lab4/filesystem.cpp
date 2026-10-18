#include "filesystem.h"
#include <iostream>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <set>

using namespace std;
using namespace std::chrono;

const int BLOCK_SIZE = 64;

FileSystem::FileSystem() : next_id(0) {
    inodes[0] = {0, true, 0, {}, {}};
    next_id = 1;  // Корень занял id=0, следующий будет 1
}

int FileSystem::get_inode_id(const string& path) {
    if (path.empty() || path[0] != '/') return -1;
    if (path == "/") return 0;
    
    int curr = 0;
    stringstream ss(path.substr(1));
    string part;
    while (getline(ss, part, '/')) {
        if (part.empty()) continue;
        if (inodes[curr].children.count(part) == 0) return -1;
        curr = inodes[curr].children[part];
    }
    return curr;
}

pair<string, string> FileSystem::split_path(const string& path) {
    if (path.empty()) return {"/", ""};
    size_t pos = path.find_last_of('/');
    if (pos == string::npos) return {"/", path};
    if (pos == 0) return {"/", path.substr(1)};
    return {path.substr(0, pos), path.substr(pos + 1)};
}

void FileSystem::create_file(const string& path) {
    auto [dir_path, name] = split_path(path);
    if (name.empty()) {
        cout << "[!] Ошибка: пустое имя\n";
        return;
    }
    int parent_id = get_inode_id(dir_path);
    if (parent_id == -1 || inodes[parent_id].children.count(name)) {
        cout << "[!] Ошибка: нельзя создать " << path << "\n";
        return;
    }
    inodes[next_id] = {next_id, false, 0, {}, {}};
    inodes[parent_id].children[name] = next_id++;
    cout << "[OK] Файл создан: " << path << "\n";
}

void FileSystem::create_dir(const string& path) {
    auto [dir_path, name] = split_path(path);
    if (name.empty()) {
        cout << "[!] Ошибка: пустое имя\n";
        return;
    }
    int parent_id = get_inode_id(dir_path);
    if (parent_id == -1 || inodes[parent_id].children.count(name)) {
        cout << "[!] Ошибка: нельзя создать " << path << "\n";
        return;
    }
    inodes[next_id] = {next_id, true, 0, {}, {}};
    inodes[parent_id].children[name] = next_id++;
    cout << "[OK] Каталог создан: " << path << "\n";
}

void FileSystem::write_file(const string& path, const string& content) {
    int id = get_inode_id(path);
    if (id == -1 || inodes[id].is_dir) {
        cout << "[!] Ошибка: файл не найден " << path << "\n";
        return;
    }

    for (int b : inodes[id].blocks) disk.free_block(b);
    inodes[id].blocks.clear();

    int blocks_needed = (content.length() + BLOCK_SIZE - 1) / BLOCK_SIZE;
    if (blocks_needed == 0) blocks_needed = 1;

    for (int i = 0; i < blocks_needed; ++i) {
        int b = disk.allocate_block();
        if (b == -1) {
            cout << "[!] Нет места на диске!\n";
            return;
        }
        inodes[id].blocks.push_back(b);
        disk.write_block(b, content.substr(i * BLOCK_SIZE, BLOCK_SIZE));
    }
    inodes[id].size = content.length();
    cout << "[OK] Записано " << inodes[id].size << " байт. Блоков: " 
         << inodes[id].blocks.size() << "\n";
}

string FileSystem::read_file(const string& path) {
    int id = get_inode_id(path);
    if (id == -1 || inodes[id].is_dir) return "[!] Ошибка чтения";

    string res = "";
    for (int b : inodes[id].blocks) res += disk.read_block(b);
    return res.substr(0, inodes[id].size);
}

void FileSystem::remove(const string& path) {
    auto [dir_path, name] = split_path(path);
    int parent_id = get_inode_id(dir_path);
    int target_id = get_inode_id(path);
    
    if (target_id == -1) {
        cout << "[!] Не найдено: " << path << "\n";
        return;
    }
    if (inodes[target_id].is_dir && !inodes[target_id].children.empty()) {
        cout << "[!] Каталог не пуст: " << path << "\n";
        return;
    }

    for (int b : inodes[target_id].blocks) disk.free_block(b);
    inodes[parent_id].children.erase(name);
    inodes.erase(target_id);
    cout << "[OK] Удалено: " << path << "\n";
}

void FileSystem::move(const string& src, const string& dst) {
    auto [src_dir, src_name] = split_path(src);
    auto [dst_dir, dst_name] = split_path(dst);
    int src_parent = get_inode_id(src_dir);
    int dst_parent = get_inode_id(dst_dir);
    int target_id = get_inode_id(src);

    if (target_id == -1 || dst_parent == -1 || 
        inodes[dst_parent].children.count(dst_name)) {
        cout << "[!] Ошибка перемещения\n";
        return;
    }

    inodes[src_parent].children.erase(src_name);
    inodes[dst_parent].children[dst_name] = target_id;
    cout << "[OK] Перемещено: " << src << " -> " << dst << "\n";
}

void FileSystem::copy(const string& src, const string& dst) {
    string content = read_file(src);
    if (content.substr(0, 3) == "[!]") {
        cout << "[!] Ошибка копирования\n";
        return;
    }
    create_file(dst);
    write_file(dst, content);
}

void FileSystem::print_tree(int curr, int depth, set<int>& visited) const {
    if (depth > 10) return;
    if (inodes.count(curr) == 0) return;
    
    if (visited.count(curr)) {
        cout << string(depth * 2, ' ') << "[!] Цикл на Inode:" << curr << "\n";
        return;
    }
    visited.insert(curr);
    
    string indent(depth * 2, ' ');
    const Inode& node = inodes.at(curr);

    if (node.is_dir) {
        cout << indent << "[DIR]  Inode:" << node.id << "\n";
        for (const auto& child : node.children) {
            print_tree(child.second, depth + 1, visited);
        }
    } else {
        string blocks_str = "[";
        for (size_t i = 0; i < node.blocks.size(); ++i) {
            blocks_str += to_string(node.blocks[i]);
            if (i + 1 < node.blocks.size()) blocks_str += ", ";
        }
        blocks_str += "]";
        cout << indent << "[FILE] Inode:" << node.id 
             << " | Size:" << node.size << "B"
             << " | Blocks:" << blocks_str << "\n";
    }
    
    visited.erase(curr);
}

void FileSystem::dump() const {
    cout << "\n=== ДАМП ФС (Вариант 8: Перечень блоков) ===\n";
    set<int> visited;
    print_tree(0, 0, visited);
}

void FileSystem::load_test() {
    cout << "\n=== НАГРУЗОЧНОЕ ТЕСТИРОВАНИЕ ===\n";
    cout << left << setw(12) << "Размер (КБ)" 
         << setw(12) << "Блоков" 
         << "Время (мкс)\n";
    cout << string(36, '-') << "\n";
    
    create_file("/test.bin");
    
    int sizes[] = {1, 2, 4, 8, 16};
    for (int i = 0; i < 5; ++i) {
        int kb = sizes[i];
        string data(kb * 1024, 'X');
        auto start = high_resolution_clock::now();
        write_file("/test.bin", data);
        auto end = high_resolution_clock::now();
        
        int id = get_inode_id("/test.bin");
        int blks = inodes[id].blocks.size();
        cout << left << setw(12) << kb 
             << setw(12) << blks 
             << duration_cast<microseconds>(end - start).count() << "\n";
    }
    remove("/test.bin");
    cout << "\n* Вывод: Время записи линейно зависит от количества блоков.\n";
}