#include <iostream>
#include <unistd.h>
#include <random>
#include <chrono>
#include <cstring>
#include "gtest/gtest.h"
#include "test_util.h"

/// @brief TEST_P 실행 전, 실행 됨
void HashTableTest::SetUp() {
    load_workload();
}

/// @brief TEST_P 실행 후, 실행 됨
void HashTableTest::TearDown() {
    run_workload();
    check_answer();
}

/**
 * @brief 워크로드를 불러오는 함수
 * Workload의 type과 num을 바탕으로, request를 생성한다.
 * Request의 key와 value는 1 ~ 100,000 사이의 랜덤한 값을 가진다.
 */
void HashTableTest::load_workload() {
    // 난수 생성기 초기화
    std::random_device rd;
    std::mt19937 gen(rd());
    // uniform random: [1, 100000]
    std::uniform_int_distribution<> dis(1, 100000);

    // workload 배열 메모리 할당
    workload_ = (Req*)malloc(sizeof(Req) * workload_num_);

    // 첫 번째 request는 insert로 고정 (빈 테이블 보장)
    workload_[0] = {1, 1, INSERT};

    for (int i = 1; i < workload_num_; ++i) {
        workload_[i].key   = dis(gen);
        workload_[i].value = dis(gen);

        switch (workload_type_) {
        case INSERT_ONLY:
            workload_[i].req_type = INSERT;
            break;

        case INSERT_LOOKUP:
            // INSERT 50%, LOOKUP 50%
            workload_[i].req_type = static_cast<REQ_TYPE>(dis(gen) % 2);
            break;

        case INSERT_LOOKUP_DELETE: {
            int num = dis(gen) % 5;
            // INSERT 60%, LOOKUP 20%, DELETE 20%
            if (num < 3)     workload_[i].req_type = INSERT;
            else if (num == 3) workload_[i].req_type = LOOKUP;
            else               workload_[i].req_type = DELETE;
            break;
        }

        default:
            exit(0);
        }
    }
}

/**
 * @brief 워크로드를 실행하는 함수
 * Workload를 스레드 수 만큼 분할하고, 각 스레드에 할당한다.
 */
void HashTableTest::run_workload() {
    pthread_t* threads = (pthread_t*)malloc(sizeof(pthread_t) * thread_num_);
    memset(threads, 0x0, sizeof(pthread_t) * thread_num_);

    ThreadArg* th_args = (ThreadArg*)malloc(sizeof(ThreadArg) * thread_num_);

    for (int i = 0; i < thread_num_; i++) {
        th_args[i].hashtable    = hashtable;
        th_args[i].workload_num = workload_num_ / thread_num_;
        th_args[i].workload     = workload_ + i * (workload_num_ / thread_num_);
    }

    auto start = std::chrono::steady_clock::now();

    for (int i = 0; i < thread_num_; i++) {
        pthread_create(&threads[i], NULL, thread_job, (void*)(th_args + i));
    }
    for (int i = 0; i < thread_num_; i++) {
        pthread_join(threads[i], NULL);
    }

    auto end = std::chrono::steady_clock::now();
    auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Execution time: " << duration_ms.count() << " ms" << std::endl;

    free(threads);
    free(th_args);
}

/**
 * @brief 테스트 케이스의 수행결과를 확인하는 함수
 * std::map을 통해 single thread로 workload를 수행한 결과와 비교한다.
 * Hash Table은 삽입 순서를 보장하지 않으므로, traversal이 key 기준 정렬된 결과를 반환한다.
 */
void HashTableTest::check_answer() {
    // INSERT_LOOKUP_DELETE + Multi-threading 조합은 비교 생략
    // (삭제 순서에 따라 최종 상태가 달라질 수 있음)
    if ((workload_type_ == INSERT_LOOKUP_DELETE) && (thread_num_ != 1)) {
        return;
    }

    // std::map 으로 동일 workload 를 싱글 스레드 수행 (정답 생성)
    for (int i = 0; i < workload_num_; i++) {
        switch (workload_[i].req_type) {
        case INSERT: {
            auto itr = map_.find(workload_[i].key);
            if (itr == map_.end()) {
                map_.insert({workload_[i].key, {workload_[i].value, 0}});
            } else {
                itr->second.first  += workload_[i].value;
                itr->second.second += 1;
            }
            break;
        }
        case LOOKUP:
            // 정답 처리용이므로 Lookup은 수행하지 않음
            break;
        case DELETE:
            map_.erase(workload_[i].key);
            break;
        default:
            break;
        }
    }

    // HashTable 순회 결과 (key 오름차순 정렬됨)
    KVC* arr = new KVC[map_.size()];
    hashtable->traversal(arr);

    auto iter = map_.begin();
    for (int i = 0; i < (int)map_.size(); i++, iter++) {
        ASSERT_EQ(iter->first,        arr[i].key);
        ASSERT_EQ(iter->second.first, arr[i].value);
        ASSERT_EQ(iter->second.second, arr[i].upd_cnt);
    }

    delete[] arr;
    delete hashtable;
}

/**
 * @brief 각 스레드가 자신에게 할당된 workload를 수행
 * 
 * @param arg : 스레드 인자
 * @return void* 
 */
void* thread_job(void* arg) {
    ThreadArg* th_arg = (ThreadArg*)arg;

    for (int i = 0; i < th_arg->workload_num; i++) {
        switch (th_arg->workload[i].req_type) {
        case INSERT:
            th_arg->hashtable->insert(th_arg->workload[i].key,
                                      th_arg->workload[i].value);
            break;
        case LOOKUP:
            th_arg->hashtable->lookup(th_arg->workload[i].key);
            break;
        case DELETE:
            th_arg->hashtable->remove(th_arg->workload[i].key);
            break;
        default:
            exit(0);
        }
    }
    return nullptr;
}
