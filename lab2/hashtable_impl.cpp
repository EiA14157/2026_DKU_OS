/*
*  DKU Operating System Lab (2026)
*      Lab2 (Concurrency Data Structure: Hash Table)
*      Student id : 
*      Student name : 
*      Date:
*/


#include "hashtable_impl.h"
#include <cstdlib>
#include <vector>
#include <algorithm>

namespace {
HTNode* find_node(HTNode* head, int key) {
    HTNode* node = head;
    while (node != nullptr) {
        if (node->key == key) {
            return node;
        }
        node = node->next;
    }
    return nullptr;
}
}

// DefaultHashTable
// DefaultHashTable 생성자
DefaultHashTable::DefaultHashTable(int num_buckets) : num_buckets_(num_buckets) {
    // 버킷 배열 할당 및 초기화 (모두 nullptr)
    buckets_ = new HTNode*[num_buckets_];
    for (int i = 0; i < num_buckets_; i++) {
        buckets_[i] = nullptr;
    }
}

// DefaultHashTable 소멸자
DefaultHashTable::~DefaultHashTable() {
    // 각 버킷의 체인 노드 전부 해제
    for (int i = 0; i < num_buckets_; i++) {
        HTNode* node = buckets_[i];
        while (node) {
            HTNode* tmp = node->next;
            delete node;
            node = tmp;
        }
    }
    delete[] buckets_;
}

// 해시 함수 : key를 버킷 수로 나눈 나머지
int DefaultHashTable::hash_func(int key) {
    return key % num_buckets_;
}

// 순회 함수 : 전체 버킷을 순회하여 key 오름차순으로 정렬 후 arr에 저장
// Hash Table은 삽입 순서를 보장하지 않으므로, std::sort로 정렬한다.
void DefaultHashTable::traversal(KVC* arr) {
    std::vector<KVC> tmp;
    for (int i = 0; i < num_buckets_; i++) {
        HTNode* node = buckets_[i];
        while (node) {
            KVC kvc;
            kvc.key     = node->key;
            kvc.value   = node->value;
            kvc.upd_cnt = node->upd_cnt;
            tmp.push_back(kvc);
            node = node->next;
        }
    }
    // key 기준 오름차순 정렬 (std::map 순회와 동일한 순서 보장)
    std::sort(tmp.begin(), tmp.end(), [](const KVC& a, const KVC& b) {
        return a.key < b.key;
    });
    for (int i = 0; i < (int)tmp.size(); i++) {
        arr[i] = tmp[i];
    }
}

// HashTable (without lock)
// HashTable 생성자
HashTable::HashTable(int num_buckets) : DefaultHashTable(num_buckets) {}

// HashTable 소멸자
HashTable::~HashTable() {}

void HashTable::insert(int key, int value) {
    // 구현
    int bucket_idx = hash_func(key);
    HTNode* node = find_node(buckets_[bucket_idx], key);

    if (node != nullptr) {
        node->value += value;
        node->upd_cnt += 1;
        return;
    }

    HTNode* new_node = new HTNode{key, value, 0, buckets_[bucket_idx]};
    buckets_[bucket_idx] = new_node;
}

int HashTable::lookup(int key) {
    // 구현
    int bucket_idx = hash_func(key);
    HTNode* node = find_node(buckets_[bucket_idx], key);
    if (node == nullptr) {
        return 0;
    }
    return node->value;
}

void HashTable::remove(int key) {
    // 구현
    int bucket_idx = hash_func(key);
    HTNode* prev = nullptr;
    HTNode* node = buckets_[bucket_idx];

    while (node != nullptr) {
        if (node->key == key) {
            if (prev == nullptr) {
                buckets_[bucket_idx] = node->next;
            }
            else {
                prev->next = node->next;
            }
            delete node;
            return;
        }
        prev = node;
        node = node->next;
    }
}

void HashTable::traversal(KVC* arr) {
    // 구현
    DefaultHashTable::traversal(arr);
}

// CoarseHashTable (coarse-grained lock)
// CoarseHashTable 생성자
CoarseHashTable::CoarseHashTable(int num_buckets) : DefaultHashTable(num_buckets) {
    pthread_mutex_init(&mutex_lock, nullptr);
}

// CoarseHashTable 소멸자
CoarseHashTable::~CoarseHashTable() {
    pthread_mutex_destroy(&mutex_lock);
}

void CoarseHashTable::insert(int key, int value) {
    // 구현
    pthread_mutex_lock(&mutex_lock);

    int bucket_idx = hash_func(key);
    HTNode* node = find_node(buckets_[bucket_idx], key);
    if (node != nullptr) {
        node->value += value;
        node->upd_cnt += 1;
        pthread_mutex_unlock(&mutex_lock);
        return;
    }

    HTNode* new_node = new HTNode{key, value, 0, buckets_[bucket_idx]};
    buckets_[bucket_idx] = new_node;

    pthread_mutex_unlock(&mutex_lock);
}

int CoarseHashTable::lookup(int key) {
    // 구현
    pthread_mutex_lock(&mutex_lock);

    int bucket_idx = hash_func(key);
    HTNode* node = find_node(buckets_[bucket_idx], key);
    int value = (node == nullptr) ? 0 : node->value;

    pthread_mutex_unlock(&mutex_lock);
    return value;
}

void CoarseHashTable::remove(int key) {
    // 구현
    pthread_mutex_lock(&mutex_lock);

    int bucket_idx = hash_func(key);
    HTNode* prev = nullptr;
    HTNode* node = buckets_[bucket_idx];

    while (node != nullptr) {
        if (node->key == key) {
            if (prev == nullptr) {
                buckets_[bucket_idx] = node->next;
            }
            else {
                prev->next = node->next;
            }
            delete node;
            break;
        }
        prev = node;
        node = node->next;
    }

    pthread_mutex_unlock(&mutex_lock);
}

void CoarseHashTable::traversal(KVC* arr) {
    // 구현
    pthread_mutex_lock(&mutex_lock);
    DefaultHashTable::traversal(arr);
    pthread_mutex_unlock(&mutex_lock);
}

// FineHashTable (fine-grained lock)
// FineHashTable 생성자
FineHashTable::FineHashTable(int num_buckets) : DefaultHashTable(num_buckets) {
    // 버킷 수만큼 뮤텍스 배열 할당 및 초기화
    bucket_locks = new pthread_mutex_t[num_buckets_];
    for (int i = 0; i < num_buckets_; i++) {
        pthread_mutex_init(&bucket_locks[i], nullptr);
    }
}

// FineHashTable 소멸자
FineHashTable::~FineHashTable() {
    for (int i = 0; i < num_buckets_; i++) {
        pthread_mutex_destroy(&bucket_locks[i]);
    }
    delete[] bucket_locks;
}

void FineHashTable::insert(int key, int value) {
    // 구현
    int bucket_idx = hash_func(key);
    pthread_mutex_lock(&bucket_locks[bucket_idx]);

    HTNode* node = find_node(buckets_[bucket_idx], key);
    if (node != nullptr) {
        node->value += value;
        node->upd_cnt += 1;
        pthread_mutex_unlock(&bucket_locks[bucket_idx]);
        return;
    }

    HTNode* new_node = new HTNode{key, value, 0, buckets_[bucket_idx]};
    buckets_[bucket_idx] = new_node;

    pthread_mutex_unlock(&bucket_locks[bucket_idx]);
}

int FineHashTable::lookup(int key) {
    // 구현
    int bucket_idx = hash_func(key);
    pthread_mutex_lock(&bucket_locks[bucket_idx]);

    HTNode* node = find_node(buckets_[bucket_idx], key);
    int value = (node == nullptr) ? 0 : node->value;

    pthread_mutex_unlock(&bucket_locks[bucket_idx]);
    return value;
}

void FineHashTable::remove(int key) {
    // 구현
    int bucket_idx = hash_func(key);
    pthread_mutex_lock(&bucket_locks[bucket_idx]);

    HTNode* prev = nullptr;
    HTNode* node = buckets_[bucket_idx];

    while (node != nullptr) {
        if (node->key == key) {
            if (prev == nullptr) {
                buckets_[bucket_idx] = node->next;
            }
            else {
                prev->next = node->next;
            }
            delete node;
            break;
        }
        prev = node;
        node = node->next;
    }

    pthread_mutex_unlock(&bucket_locks[bucket_idx]);
}

void FineHashTable::traversal(KVC* arr) {
    // 구현
    for (int i = 0; i < num_buckets_; i++) {
        pthread_mutex_lock(&bucket_locks[i]);
    }

    DefaultHashTable::traversal(arr);

    for (int i = 0; i < num_buckets_; i++) {
        pthread_mutex_unlock(&bucket_locks[i]);
    }
}
