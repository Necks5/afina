#include "SimpleLRU.h"

namespace Afina {
namespace Backend {

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Put(const std::string &key, const std::string &value) {
        std::size_t node_size = key.size() + value.size();
        if(node_size > _max_size) {
            return false;
        }
        auto node = _lru_index.find(key);
        if(node != _lru_index.end()) {
            std::size_t new_size = value.size() - (node->second).get().value.size();
            node_to_head(&node->second.get());
            while(current_size + new_size > _max_size) {
                delete_last();
            }
            node->second.get().value = value;
            current_size += new_size;
        } else {
            while(current_size + node_size > _max_size) {
                delete_last();
            }
            lru_node* new_node = new lru_node{key, value, nullptr, nullptr};
            // new_node->key = key;
            // new_node->value = value;
            _lru_index.insert({std::reference_wrapper<const std::string>(new_node->key), std::reference_wrapper<lru_node>(*new_node)});
            insert_node(new_node);
            current_size += node_size;
        }
        return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::PutIfAbsent(const std::string &key, const std::string &value) {
        std::size_t node_size = key.size() + value.size();
        if(node_size > _max_size) {
            return false;
        }
        auto node = _lru_index.find(key);
        if(node != _lru_index.end()) {
            return false;
        }
        while(current_size + node_size > _max_size) {
            delete_last();
        }
        lru_node* new_node = new lru_node{key, value, nullptr, nullptr};
        // new_node->key = key;
        // new_node->value = value;
        _lru_index.insert({std::reference_wrapper<const std::string>(new_node->key), std::reference_wrapper<lru_node>(*new_node)});
        insert_node(new_node);
        current_size += node_size;

        return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Set(const std::string &key, const std::string &value) {
        auto node = _lru_index.find(key);
        if(node == _lru_index.end()) {
            return false;
        }
        std::size_t new_size = value.size() - (node->second).get().value.size();
        node_to_head(&node->second.get());
        while(current_size + new_size > _max_size) {
            delete_last();
        }
        node->second.get().value = value;
        current_size += new_size;

        return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Delete(const std::string &key) {
        auto it_node = _lru_index.find(key);
        if(it_node == _lru_index.end()) {
            return false;
        }
        lru_node* node = &(it_node->second.get());
        _lru_index.erase(node->key);
        std::size_t node_size = node->key.size() + node->value.size();
        if(node == _lru_head.get()) { //delete first
            if(node->next.get()) {   // not last
                node->next.get()->prev = nullptr;
                _lru_head = std::move(node->next);
            } else {  //last (only one element in LRU)
                _lru_head.reset(nullptr);
                top = nullptr;
            }
        } else if(!node->next.get()) { //last element
            top = node->prev;
            node->prev->next = std::move(nullptr);
        } else { //poseredine
            node->next.get()->prev = node->prev;
            node->prev->next = std::move(node->next);
        }
        current_size -= node_size;
        return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Get(const std::string &key, std::string &value) {
        auto node = _lru_index.find(key);
        if(node == _lru_index.end()) {
            return false;
        }
        value = node->second.get().value;
        node_to_head(&node->second.get());
        return true;
}


void SimpleLRU::delete_last() {
    lru_node* del_node = top;
    std::size_t del_size = del_node->key.size() + del_node->value.size();
    _lru_index.erase(del_node->key);
    if(_lru_head.get() == top) {
        _lru_head.reset(nullptr);
    } else {
        top = top->prev;
        top->next.reset(nullptr);
    }
    current_size -= del_size;
}

void SimpleLRU::insert_node(lru_node *node) {
    if(_lru_head.get()) {
        _lru_head.get()->prev = node;
    } else {
        top = node;
    }
    node->next = std::move(_lru_head);
    _lru_head.reset(node);
}

void SimpleLRU::node_to_head(lru_node* node) {
    if(_lru_head.get() == node) {
        return;
    }
    if(!node->next.get()) { //last
        top = node->prev;
        // std::cout << "last\n";
        _lru_head.get()->prev = node;
        node->next = std::move(_lru_head);
        _lru_head = std::move(node->prev->next);

    } else {
        // std::cout << "ne last\n";
        auto temp_head = std::move(_lru_head);
        _lru_head = std::move(node->prev->next);
        node->prev->next = std::move(node->next);
        temp_head.get()->prev = node;
        node->next = std::move(temp_head);
        node->prev->next->prev = node->prev;

    }


}


} // namespace Backend
} // namespace Afina
