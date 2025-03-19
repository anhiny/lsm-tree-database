#pragma once

#include <cstddef>
#include <list>
#include <memory>
#include <optional>
#include <shared_mutex>
#include <string>
#include "../skiplist/skiplist.h"

class Memtable {
    public:
    Memtable();
    ~Memtable() = default;
    void put(const std::string& key, const std::string& value);
    void remove(const std::string& key);
    std::optional<std::string> get(const std::string& key);

    void clear();
    private:
    std::shared_ptr<SkipList> current_table;
    std::list<std::shared_ptr<SkipList>> frozen_tables;
    size_t frozen_bytes;
    std::shared_mutex rx_mtx;
};