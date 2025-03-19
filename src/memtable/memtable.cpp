#include "../../include/memtable/memtable.h"
#include <cstddef>
#include <memory>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <string>
#include <vector>

Memtable::Memtable() : current_table(std::make_shared<SkipList>()), frozen_bytes(0) {}

void Memtable::put_(const std::string& key, const std::string& value) {
    current_table->put(key, value);
}

void Memtable::remove_(const std::string& key) {
    current_table->put(key, "");    
}


void Memtable::put(const std::string& key, const std::string& value) {
    // 并发控制
    std::unique_lock<std::shared_mutex> lock(cur_mtx);
    put_(key, value);
}


void Memtable::put_batch(const std::vector<std::string>& keys, const std::vector<std::string>& values) {
    std::unique_lock<std::shared_mutex> lock(cur_mtx);
    for (size_t i = 0; i < keys.size(); ++i) {
        put_(keys[i], values[i]);
    }
}

void Memtable::remove(const std::string& key) {
    // 并发控制
    std::unique_lock<std::shared_mutex> lock(cur_mtx);
    remove_(key);    
}


void Memtable::remove_batch(const std::vector<std::string>& keys) {
    std::unique_lock<std::shared_mutex> lock(cur_mtx);
    for (size_t i = 0; i < keys.size(); ++i) {
        remove_(keys[i]);
    }  
}

std::optional<std::string> Memtable::cur_get_(const std::string& key) {
    auto result = current_table->get(key);
    
    if (result.has_value()) {
        auto data = result.value();
        return data;
    }

    return std::nullopt;
}



std::optional<std::string> Memtable::frozen_get_(const std::string& key) {
    // 从冻结的表中进行查找
    for (auto& table : frozen_tables) {
        auto result = table->get(key);
        if (result.has_value()) {
            return result;
        }
    }
    return std::nullopt;
}



std::optional<std::string> Memtable::get(const std::string& key) {
    std::shared_lock<std::shared_mutex> lock1(cur_mtx);
    auto result1 = cur_get_(key);
    
    if (result1.has_value()) {
        return result1;
    }

    // 从冻结的表中进行查找
    lock1.unlock();
    std::shared_lock<std::shared_mutex> lock2(frozen_mtx);    
    return frozen_get_(key);
}

std::vector<std::optional<std::string>> Memtable::get_batch(const std::vector<std::string>& keys) {
    std::shared_lock<std::shared_mutex> lock1(cur_mtx);
    std::shared_lock<std::shared_mutex> lock2(frozen_mtx);   
    std::vector<std::optional<std::string>> result;
    for (auto& key : keys) {
        auto result1 = cur_get_(key);    
        if (result1.has_value()) {
            result.push_back(result1);
            continue;
        }
        auto result2 = frozen_get_(key);
        if (result2.has_value()) {
            result.push_back(result2);
        }
    }
    return result;

}

void Memtable::clear() {
    // 并发控制
    // 为了避免死锁，按顺序加锁。规定先获取cur锁，再获取frozen锁
    std::unique_lock<std::shared_mutex> lock1(cur_mtx);
    std::unique_lock<std::shared_mutex> lock2(frozen_mtx);
    frozen_tables.clear();
    current_table->clear();    
}