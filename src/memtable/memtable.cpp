#include "../../include/memtable/memtable.h"
#include <memory>
#include <mutex>
#include <optional>
#include <shared_mutex>

Memtable::Memtable() : current_table(std::make_shared<SkipList>()), frozen_bytes(0) {}


void Memtable::put(const std::string& key, const std::string& value) {
    // 并发控制
    std::unique_lock<std::shared_mutex> lock(rx_mtx);
    current_table->put(key, value);
}
void Memtable::remove(const std::string& key) {
    // 并发控制
    std::unique_lock<std::shared_mutex> lock(rx_mtx);
    current_table->put(key, "");    
}
std::optional<std::string> Memtable::get(const std::string& key) {
    std::shared_lock<std::shared_mutex> lock(rx_mtx);
    auto result = current_table->get(key);
    
    if (result.has_value()) {
        auto data = result.value();
        return data;
    }

    // 从冻结的表中进行查找
    for (auto& table : frozen_tables) {
        auto result = table->get(key);
        if (result.has_value()) {
            return result;
        }
    }
    return std::nullopt;
}

void Memtable::clear() {
    // 并发控制
    std::unique_lock<std::shared_mutex> lock(rx_mtx);
    frozen_tables.clear();
    current_table->clear();    
}