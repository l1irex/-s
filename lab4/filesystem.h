#pragma once
#include "disk.h"
#include <string>
#include <vector>
#include <map>
#include <set>

struct Inode {
    int id;
    bool is_dir;
    int size;
    std::vector<int> blocks;
    std::map<std::string, int> children;
};

class FileSystem {
private:
    Disk disk;
    std::map<int, Inode> inodes;
    int next_id;

    int get_inode_id(const std::string& path);
    std::pair<std::string, std::string> split_path(const std::string& path);
    void print_tree(int curr, int depth, std::set<int>& visited) const;

public:
    FileSystem();
    void create_file(const std::string& path);
    void create_dir(const std::string& path);
    void write_file(const std::string& path, const std::string& content);
    std::string read_file(const std::string& path);
    void remove(const std::string& path);
    void move(const std::string& src, const std::string& dst);
    void copy(const std::string& src, const std::string& dst);
    void dump() const;
    void load_test();
};