#include "hashtable.h"

#ifndef HASHTABLE_IMPL_H
#define HASHTABLE_IMPL_H

/**
 * @brief HashTable without lock
 * DefaultHashTable의 함수를 오버라이드하여, 클래스를 완성한다.
 * 구현에 필요한 멤버 변수/함수를 추가하여 구현한다.
 */
class HashTable : public DefaultHashTable {
    private:
        // 멤버 변수 추가 선언 가능

    public:
        // 생성자
        HashTable(int num_buckets = 1024);
        // 소멸자
        ~HashTable();

        // 멤버 함수 추가 선언 가능
        void insert(int key, int value) override;
        int lookup(int key) override;
        void remove(int key) override;
        void traversal(KVC* arr);
};

/**
 * @brief HashTable with coarse-grained lock
 * HashTable 전체를 critical section으로 가정하여, 하나의 lock으로 이를 관리한다.
 * DefaultHashTable의 함수를 오버라이드하여, 클래스를 완성한다.
 * 구현에 필요한 멤버 변수/함수를 추가하여 구현한다.
 */
class CoarseHashTable : public DefaultHashTable {
    private:
        // 테이블 전체를 보호하는 단일 뮤텍스
        pthread_mutex_t mutex_lock;

    public:
        // 생성자
        CoarseHashTable(int num_buckets = 1024);
        // 소멸자
        ~CoarseHashTable();

        // 멤버 함수 추가 선언 가능
        void insert(int key, int value) override;
        int lookup(int key) override;
        void remove(int key) override;
        void traversal(KVC* arr);
};

/**
 * @brief HashTable with fine-grained lock
 * 각 버킷(bucket)마다 별도의 mutex를 사용하여 lock 경합을 줄인다.
 * 서로 다른 버킷에 접근하는 스레드들은 병렬로 수행될 수 있다.
 * DefaultHashTable의 함수를 오버라이드하여, 클래스를 완성한다.
 * 구현에 필요한 멤버 변수/함수를 추가하여 구현한다.
 */
class FineHashTable : public DefaultHashTable {
    private:
        // 버킷별 뮤텍스 배열 (bucket i 는 bucket_locks[i]로 보호)
        pthread_mutex_t* bucket_locks;

    public:
        // 생성자
        FineHashTable(int num_buckets = 1024);
        // 소멸자
        ~FineHashTable();

        // 멤버 함수 추가 선언 가능
        void insert(int key, int value) override;
        int lookup(int key) override;
        void remove(int key) override;
        void traversal(KVC* arr);
};
#endif // HASHTABLE_IMPL_H
