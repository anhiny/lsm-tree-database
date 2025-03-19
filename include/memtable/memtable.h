#pragma once

#include <cstddef>
#include <list>
#include <memory>
#include <optional>
#include <shared_mutex>
#include <string>
#include <utility>
#include <vector>
#include "../skiplist/skiplist.h"

class Memtable {
    public:
    Memtable();
    ~Memtable() = default;
    void put(const std::string& key, const std::string& value);
    void put_batch(const std::vector<std::string>& keys, const std::vector<std::string>& values);
    void remove(const std::string& key);
    void remove_batch(const std::vector<std::string>& keys);
    std::optional<std::string> get(const std::string& key);
    std::vector<std::optional<std::string>> get_batch(const std::vector<std::string>& keys);

    void clear();
    private:
    std::shared_ptr<SkipList> current_table;
    std::list<std::shared_ptr<SkipList>> frozen_tables;
    size_t frozen_bytes;
    std::shared_mutex cur_mtx;
    std::shared_mutex frozen_mtx;

    // 后缀_表示不加锁的版本
    void put_(const std::string& key, const std::string& value);
    void remove_(const std::string& key);
    std::optional<std::string> cur_get_(const std::string& key);
    std::optional<std::string> frozen_get_(const std::string& key);
};