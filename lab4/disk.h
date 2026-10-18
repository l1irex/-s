#pragma once
#include <vector>
#include <string>

class Disk {
private:
    std::vector<std::string> blocks;
    std::vector<int> free_blocks;
    int block_size;  // Сохраняем размер блока

public:
    Disk(int num_blocks = 500, int block_size = 64);
    
    int allocate_block();
    void free_block(int id);
    void write_block(int id, const std::string& data);
    std::string read_block(int id) const;
    int get_free_count() const;
    int get_block_size() const;
};