#include <pthread.h>
#include <iostream>
#ifndef HASHTABLE_H
#define HASHTABLE_H

/// @brief Key, Value, Update Count 구조체
struct KVC {
    int key;
    int value;
    int upd_cnt;
};

/// @brief Hash Table 노드 구조체 (Separate Chaining)
struct HTNode {
    // 키
    int key;
    // 밸류
    int value;
    // 업데이트 횟수
    int upd_cnt;
    // 체이닝: 같은 버킷 내 다음 노드 포인터
    HTNode* next;
};

/// @brief HashTable 부모 클래스.
class DefaultHashTable {
    protected:
        // 버킷 개수
        int num_buckets_;
        // 버킷 배열 (각 버킷은 HTNode 체인의 헤드)
        HTNode** buckets_;
        
    public:
        DefaultHashTable(int num_buckets = 1024);
        virtual ~DefaultHashTable();
        
        /**
         * @brief 삽입 함수 : 인자로 받은 key와 value를 저장한다.
         * 이미 key가 존재하는 경우, 기존 value에 새로운 value를 더하고 update count를 1 증가시킨다.
         * 
         * @param key 
         * @param value 
         */
        virtual void insert(int key, int value) = 0;

        /**
         * @brief 탐색 함수 : 인자로 받은 key의 value를 반환한다.
         * HashTable에 key가 존재하지 않을 경우, 0을 반환한다.
         * 
         * @param key 
         * @return int 
         */
        virtual int lookup(int key) = 0;

        /**
         * @brief 삭제 함수 : 인자로 받은 key가 저장된 노드를 삭제한다.
         * 
         * @param key 
         */
        virtual void remove(int key) = 0;

        /**
         * @brief 모든 노드를 key 오름차순으로 정렬하여 배열에 저장한다.
         * Hash Table은 정렬된 순서를 보장하지 않으므로, 순회 후 정렬을 수행한다.
         * 
         * @param arr 
         */
        virtual void traversal(KVC* arr);

    protected:
        /**
         * @brief 키에 대한 버킷 인덱스를 계산하는 해시 함수
         * 
         * @param key 
         * @return int 
         */
        int hash_func(int key);
};
#endif // HASHTABLE_H
