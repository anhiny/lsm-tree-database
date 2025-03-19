#include "../../include/skiplist/skiplist.h"
#include <memory>
#include <optional>
#include <random>
#include <stdexcept>
#include <vector>

// **************************** SkipListIterator ***************************

SkipListIterator::SkipListIterator(std::shared_ptr<SkipListNode> node)
    : current(node) {}

SkipListIterator& SkipListIterator::operator++() {
    if (current) {
        current = current->forward[0];
    }
    return *this;
}

SkipListIterator SkipListIterator::operator++(int) {
    SkipListIterator temp = current;
    ++(*this);
    return temp;
}

bool SkipListIterator::operator==(const SkipListIterator& other) const {
    return current == other.current;
}
bool SkipListIterator::operator!=(const SkipListIterator& other) const {
    return !(*this == other);
}

std::string SkipListIterator::get_key() const { return current->key; }
std::string SkipListIterator::get_value() const { return current->value; }

bool SkipListIterator::is_valid () const { return !current->value.empty(); }
bool SkipListIterator::is_end() const { return current == nullptr;}

// **************************** SkipList ***********************************
SkipList::SkipList(int max_level) {
    this->max_level = max_level;
    this->head = std::make_shared<SkipListNode>("", "", max_level);
    current_level = 1;
}

int SkipList::random_level() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0, 1);
    int level = 1;
    // 通过抛硬币的方式生成随机数
    // 每一次都有百分之五十的概率增加一层
    while (dis(gen) && level < max_level) {
        level++;
    }
    return level;
}

void SkipList::put(const std::string& key, const std::string& value) {

    // 标记当前的节点
    auto current = head;

    // 记录修改位置
    std::vector<std::shared_ptr<SkipListNode>> update(max_level, nullptr);
    for (int i = current_level - 1; i >= 0; --i) {
        while (current->forward[i] && current->forward[i]->key < key) {
            current = current->forward[i];
        }
        update[i] = current;
    }

    // 移动到最底层
    current = current->forward[0];
    // 判断key是否存在
    if (current && current->key == key) {
        size_bytes += (value.size() - current->value.size());
        current->value = value;
        return;
    }
    // 不存在即创建节点
    int new_level = random_level();
    if (new_level > current_level) {
        for (int i = current_level; i < new_level; ++i) {
            update[i] = head;
        }
        current_level = new_level;
    }
    auto new_node = std::make_shared<SkipListNode>(key, value, new_level);
    size_bytes += new_node->key.size() + new_node->value.size();

    // 更新各层的指针
    for (int i = 0; i < new_level; ++i) {
        new_node->forward[i] = update[i]->forward[i];
        update[i]->forward[i] = new_node;
    }

    // 更新backward指针
    for (int i = 0; i < new_level; ++i) {
        if (new_node->forward[i]) {
            new_node->forward[i]->set_backward(i, new_node);
        }
        new_node->set_backward(i, update[i]);
    }
}


void SkipList::remove(const std::string& key) {
    std::vector<std::shared_ptr<SkipListNode>> update(max_level, nullptr);

    // 标记当前的节点
    auto current = head;
    for (int i = current->forward.size() - 1; i >= 0; --i) {
        while (current->forward[i] && current->forward[i]->key < key) {
            current = current->forward[i];
        }
        update[i] = current;
    }
    // 移动到最底层
    current = current->forward[0];

    // 如果找到目标节点，执行删除操作
    if (current && current->key == key) {
        // 记录要更新的节点
        for (int i = 0; i < current_level; ++i) {
            if (update[i]->forward[i] != current) {
                break;
            }
            update[i]->forward[i] = current->forward[i];
        }
    }

    // 更新backward指针
    for (int i = 0; i < current->backward.size() && i < current_level; ++i) {
        if (current->forward[i]) {
            current->forward[i]->set_backward(i, update[i]);
        }
    }

    // 更新内存大小
    if (current) {
        size_bytes -= key.size() + current->value.size();
    }

    // 如果删除节点是最高层的节点，需要更新跳表层级
    while (current_level > 1 && head->forward[current_level - 1] == nullptr) {
        current_level--;
    }
}

std::optional<std::string> SkipList::get(const std::string& key) {
    auto current = head;
    for (int i = current_level - 1; i >= 0; --i) {
        while (current->forward[i] && current->forward[i]->key < key) {
            current = current->forward[i];
        }
    }
    
    // 移动到最底层
    current = current->forward[0];
    if (current && current->key == key) {
        return current->value;
    } else {
        return std::nullopt;
    }
    return std::nullopt;
}

void SkipList::clear() {
    head = std::make_shared<SkipListNode>("", "", max_level);
    size_bytes = 0;
}

SkipListIterator SkipList::begin() {
    return SkipListIterator(head->forward[0]);
}

SkipListIterator SkipList::end() {
    return SkipListIterator(nullptr);
}