#pragma once

#include <cstddef>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <memory>
#include <vector>
#include <optional>
#include <random>
#include <utility>
#include <functional>


// 跳表的节点
struct SkipListNode {
    std::string key;        // 节点存储的键
    std::string value;      // 节点存储的值
    std::vector<std::shared_ptr<SkipListNode>>
        forward;            // 指向不同层级的下一个节点的指针数组
    std::vector<std::weak_ptr<SkipListNode>>
        backward;           // 指向不同层级的上一个节点的指针数组

    SkipListNode(const std::string& k, const std::string& v, int level) 
        : key(k), value(v), forward(level, nullptr),
          backward(level, std::weak_ptr<SkipListNode>()) {}

    void set_backward(int level, std::shared_ptr<SkipListNode> node) {
        backward[level] = std::weak_ptr<SkipListNode>(node);
    }
};

// 跳表迭代器
class SkipListIterator {
    private:
    std::shared_ptr<SkipListNode> current;
    std::shared_ptr<std::shared_lock<std::shared_mutex>> lock;

public:
    SkipListIterator(std::shared_ptr<SkipListNode> node = nullptr);
    SkipListIterator& operator++();
    SkipListIterator operator++(int);
    bool operator==(const SkipListIterator& other) const;
    bool operator!=(const SkipListIterator& other) const;

    std::string get_key() const;
    std::string get_value() const;

    bool is_valid () const;
    bool is_end() const;
};

// 跳表类
class SkipList {
    private:
    std::shared_ptr<SkipListNode> head; // 跳表的头节点
    int max_level;                      // 跳表的最大层数
    int current_level;                  // 当前跳表的层数
    size_t size_bytes;                  // 占用的字节数
    int random_level();                 // 随机生成节点的层级

    public:
    SkipList(int max_level = 16);       // 默认最大层级为16

    void put(const std::string& key, const std::string& value); // 插入操作

    void remove(const std::string& key);                        // 删除操作

    std::optional<std::string> get(const std::string& key);     // 查询操作

    void clear();

    SkipListIterator begin();

    SkipListIterator end();

    std::optional<std::pair<SkipListIterator, SkipListIterator>> iters_monotony_predicate(std::function<int(const std::string&)> predicate);

};
