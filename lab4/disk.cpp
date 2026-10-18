#include "disk.h"

Disk::Disk(int num_blocks, int block_size) 
    : block_size(block_size) {
    blocks.resize(num_blocks, "");
    for (int i = 0; i < num_blocks; ++i) {
        free_blocks.push_back(i);
    }
}

int Disk::allocate_block() {
    if (free_blocks.empty()) return -1;
    int id = free_blocks.back();
    free_blocks.pop_back();
    return id;
}

void Disk::free_block(int id) {
    if (id >= 0 && id < (int)blocks.size()) {
        blocks[id] = "";
        free_blocks.push_back(id);
    }
}

void Disk::write_block(int id, const std::string& data) {
    if (id >= 0 && id < (int)blocks.size()) {
        blocks[id] = data;
    }
}

std::string Disk::read_block(int id) const {
    if (id >= 0 && id < (int)blocks.size()) {
        return blocks[id];
    }
    return "";
}

int Disk::get_free_count() const {
    return free_blocks.size();
}

int Disk::get_block_size() const {
    return block_size;
}