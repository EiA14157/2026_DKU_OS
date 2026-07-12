# Lab 2 Report Draft

## 1. 과제 개요

이번 과제의 목표는 Hash Table에 대해 서로 다른 lock 적용 범위를 사용하여 동시성 제어를 구현하고, 그에 따른 correctness와 성능 차이를 비교하는 것이다. 구현 대상은 Without Lock, Coarse-grained Lock, Fine-grained Lock의 세 가지 버전이다. 세 버전은 모두 같은 Hash Table 자료구조를 사용하지만, lock의 범위가 다르기 때문에 thread 수와 workload에 따라 실행 시간이 달라진다.

## 2. 구현한 소스코드 설명

### 2-1. 기본 자료구조

Hash Table은 bucket 배열과 각 bucket에 연결된 singly linked list로 구성된다. key가 들어오면 `hash_func(key)`로 bucket index를 계산하고, 같은 bucket에 여러 key가 들어오면 chaining 방식으로 연결 리스트에 저장한다.

각 node는 다음 정보를 가진다.

- `key`: node가 저장하는 key
- `value`: 해당 key에 대한 누적 value
- `upd_cnt`: 동일 key에 대해 다시 insert가 호출된 횟수
- `next`: 같은 bucket 체인의 다음 node

### 2-2. 공통 연산

`insert` 연산은 먼저 key가 속한 bucket 체인을 순회하여 같은 key가 존재하는지 확인한다. 같은 key가 존재하면 기존 value에 새 value를 더하고 `upd_cnt`를 1 증가시킨다. 같은 key가 없으면 새 node를 생성하여 bucket chain의 head에 삽입한다.

`lookup` 연산은 key가 속한 bucket만 순회하여 node를 찾고, 존재하면 value를 반환하며 존재하지 않으면 0을 반환한다.

`remove` 연산은 bucket 체인을 순회하면서 target key를 찾고, 이전 node와의 연결을 다시 이어 주어 unlink한 뒤 node를 삭제한다.

`traversal` 연산은 전체 bucket을 순회하여 `(key, value, upd_cnt)`를 임시 배열에 모은 뒤 key 오름차순으로 정렬하여 반환한다. 이는 테스트 코드의 `std::map` 기반 정답 비교와 맞추기 위한 처리이다.

### 2-3. Without Lock

`HashTable`은 lock이 없는 기본 버전이다. single-thread 환경에서는 가장 단순하며 lock 오버헤드가 없다. 그러나 여러 thread가 동시에 접근하면 race condition이 발생할 수 있으므로 multi-thread correctness를 보장할 수 없다.

### 2-4. Coarse-grained Lock

`CoarseHashTable`은 Hash Table 전체를 하나의 mutex로 보호한다. `insert`, `lookup`, `remove`, `traversal`을 수행할 때마다 전체 table이 하나의 critical section이 된다. 구현이 단순하고 correctness를 보장하기 쉽지만, thread 수가 증가하더라도 병렬성이 제한된다.

### 2-5. Fine-grained Lock

`FineHashTable`은 bucket마다 별도의 mutex를 두고, 연산이 접근하는 bucket에 대해서만 lock을 획득한다. 따라서 서로 다른 bucket을 접근하는 thread끼리는 동시에 실행될 수 있다. 이 방식은 coarse-grained보다 더 높은 병렬성을 기대할 수 있지만, lock 배열 관리 비용과 구현 복잡도가 증가한다.

`traversal`은 전체 상태를 일관되게 읽어야 하므로 모든 bucket lock을 고정된 순서로 획득한 뒤 수행하였다.

## 3. 문제 풀이

### 3-1. 멀티 스레드 환경에서 Hash Table 접근 순서를 일관되게 보장하는 방법

멀티 스레드 환경에서 요청 수행 순서를 일관되게 보장하려면 공유 데이터에 접근하는 구간을 critical section으로 정의하고, mutex를 통해 상호배제를 보장해야 한다. 여러 thread가 같은 자료구조를 동시에 수정하면 race condition이 발생할 수 있고, 실행 순서에 따라 최종 결과가 달라질 수 있기 때문이다.

가장 단순한 방법은 coarse-grained lock 방식이다. 이 방식에서는 hash table 전체에 대해 하나의 mutex를 두고, 어떤 thread가 `insert`, `lookup`, `remove` 중 하나를 수행하는 동안 다른 thread는 기다리게 된다. 따라서 요청은 mutex 획득 순서에 따라 직렬화되며, single-thread에서 순차적으로 수행한 것과 같은 일관성을 유지할 수 있다.

반면 fine-grained lock 방식에서는 전체 table의 단일 순서를 강하게 보장하는 대신, 같은 bucket에 대한 순서를 보장한다. 서로 다른 bucket을 접근하는 요청은 동시에 수행될 수 있고, 같은 bucket을 접근하는 요청만 mutex를 통해 직렬화된다.

### 3-2. Fine-grained Hash Table에서 동일 bucket에 대한 Insert와 Remove가 동시에 발생할 때 결과가 달라지는 이유

Fine-grained Hash Table에서는 같은 bucket에 대해서는 하나의 bucket lock으로 보호하지만, 실제 최종 결과는 어떤 thread가 lock을 먼저 획득하느냐에 따라 달라질 수 있다.

예를 들어 key `K`가 아직 존재하지 않는 상태에서 thread A가 `insert(K, 10)`을 수행하고 thread B가 `remove(K)`를 동시에 수행한다고 가정하자.

- A가 먼저 lock을 획득하면 A는 새 node를 삽입한 뒤 lock을 해제하고, 이후 B가 그 node를 삭제할 수 있다. 최종 결과는 key `K`가 존재하지 않는 상태가 된다.
- B가 먼저 lock을 획득하면 아직 key `K`가 없으므로 remove는 아무 일도 하지 않고 종료된다. 이후 A가 insert를 수행하면 최종 결과는 key `K`가 존재하는 상태가 된다.

즉 fine-grained lock은 자료구조 무결성은 보장하지만, 연산의 논리적 선후관계 자체를 고정해 주지는 않는다.

## 4. 실험 환경 및 측정 방법

실험은 VirtualBox의 Ubuntu VM(`linuxUbuntu`)에서 수행하였다. 측정은 다음 순서로 진행했다.

1. `make clean`
2. `make`
3. `./test`

실행 시간은 테스트 코드가 각 케이스마다 출력한 `Execution time` 값을 사용하였다. 비교 대상은 Coarse-grained Lock과 Fine-grained Lock이며, workload는 다음 세 가지이다.

- Insert Only
- Insert + Lookup
- Insert + Lookup + Delete

thread 수는 1, 2, 4, 8개로 구성하였다.

## 5. 벤치마킹 결과

### 5-1. 실행 시간 표

| Workload | Threads | Coarse-grained (ms) | Fine-grained (ms) |
|---|---:|---:|---:|
| Insert Only | 1 | 496 | 526 |
| Insert Only | 2 | 496 | 480 |
| Insert Only | 4 | 497 | 492 |
| Insert Only | 8 | 555 | 555 |
| Insert + Lookup | 1 | 469 | 487 |
| Insert + Lookup | 2 | 488 | 458 |
| Insert + Lookup | 4 | 542 | 490 |
| Insert + Lookup | 8 | 577 | 627 |
| Insert + Lookup + Delete | 1 | 455 | 451 |
| Insert + Lookup + Delete | 2 | 461 | 465 |
| Insert + Lookup + Delete | 4 | 758 | 461 |
| Insert + Lookup + Delete | 8 | 653 | 461 |

### 5-2. 실행 시간 그래프 정리용 수치

그래프는 workload별로 x축을 thread 수, y축을 실행 시간(ms)으로 두고 Coarse-grained와 Fine-grained를 두 개의 선으로 그리면 된다.

- Insert Only
  - Coarse: 496, 496, 497, 555
  - Fine: 526, 480, 492, 555
- Insert + Lookup
  - Coarse: 469, 488, 542, 577
  - Fine: 487, 458, 490, 627
- Insert + Lookup + Delete
  - Coarse: 455, 461, 758, 653
  - Fine: 451, 465, 461, 461

## 6. Discussion

### 6-1. Thread 수에 따른 실행 시간 비교

single-thread 환경에서는 coarse-grained와 fine-grained의 차이가 크지 않았고, 오히려 coarse가 약간 더 빠른 경우도 있었다. 이는 fine-grained 방식이 bucket별 lock 배열을 관리하는 추가 비용을 가지기 때문이다. 즉 thread가 1개일 때는 병렬성의 이점보다 lock 관리 오버헤드가 먼저 드러난다.

multi-thread 환경에서는 workload 종류에 따라 fine-grained lock의 장점이 뚜렷하게 나타났다. 특히 `Insert + Lookup + Delete` workload에서 4-thread와 8-thread 결과를 보면 fine-grained가 coarse-grained보다 훨씬 빠르게 측정되었다. 이는 coarse-grained가 table 전체를 하나의 critical section으로 묶는 반면, fine-grained는 경쟁 범위를 bucket 수준으로 줄이기 때문이다.

반면 `Insert + Lookup` workload의 8-thread 결과에서는 fine-grained가 coarse-grained보다 느리게 나왔다. 이 결과는 fine-grained lock이 항상 무조건 빠른 것이 아니라는 점을 보여 준다. key가 특정 bucket에 몰리거나 lock 획득과 해제가 매우 자주 발생하면 bucket 단위 lock 관리 비용이 오히려 더 커질 수 있다.

### 6-2. Fine-grained가 확실히 유리했던 workload

이번 측정에서 fine-grained가 가장 확실하게 유리했던 경우는 `Insert + Lookup + Delete` workload였다.

- 4-thread: coarse 758ms, fine 461ms
- 8-thread: coarse 653ms, fine 461ms

이 workload는 insert와 lookup뿐 아니라 delete까지 포함하므로 연결 리스트 포인터 수정이 자주 일어난다. 이때 coarse-grained 방식은 table 전체를 잠그기 때문에 병목이 커진다. 반면 fine-grained 방식은 같은 bucket에 대해서만 직렬화하고, 서로 다른 bucket은 동시에 처리할 수 있으므로 경쟁 범위를 크게 줄일 수 있었다.

따라서 “수정 연산이 많고 경쟁이 심한 workload에서는 fine-grained가 coarse-grained보다 확실히 유리할 것이다”라는 가설을 세울 수 있었고, 이번 결과는 이 가설을 강하게 지지했다.

### 6-3. Fine-grained가 오히려 느렸던 경우와 가설

이번 결과에서 fine-grained가 오히려 느렸던 대표적인 경우는 `Insert + Lookup` workload의 8-thread였다.

- 8-thread: coarse 577ms, fine 627ms

이 결과에 대해 세울 수 있는 가설은 다음과 같다.

1. key가 일부 bucket에 집중되어 bucket lock 경쟁이 예상보다 심해졌을 수 있다.
2. lookup이 섞이면서 lock 획득과 해제가 더 자주 발생해 bucket별 lock 관리 비용이 커졌을 수 있다.
3. 8-thread 수준에서는 thread scheduling 오버헤드와 lock contention이 함께 증가해 fine-grained의 장점이 상쇄되었을 수 있다.

이번 결과만 보면 이 가설은 “부분적으로 맞았다”고 보는 것이 가장 적절하다. 이유는 2-thread와 4-thread에서는 같은 `Insert + Lookup` workload에서 fine-grained가 coarse-grained보다 빨랐기 때문이다.

- 2-thread: coarse 488ms, fine 458ms
- 4-thread: coarse 542ms, fine 490ms

즉 fine-grained 방식 자체가 이 workload에 불리한 것은 아니고, thread 수가 더 커졌을 때 특정 조건에서 오히려 손해를 볼 수 있다는 의미다. 따라서 “fine-grained는 평균적으로 병렬성에 유리하지만, 항상 무조건 빠르지는 않다”는 가설이 이번 실험과 가장 잘 맞는 해석이다. 다만 이 가설을 더 강하게 검증하려면 동일 workload를 여러 번 반복 실행해 평균값과 분산까지 비교할 필요가 있다.

### 6-4. 그래프를 통해 해석할 포인트

그래프를 보면 `Insert + Lookup + Delete`에서는 thread 수가 증가할수록 coarse-grained의 실행 시간이 크게 증가하거나 높은 수준을 유지하는 반면, fine-grained는 비교적 안정적으로 유지되는 모습을 보일 것이다. 반대로 `Insert + Lookup`에서는 8-thread에서 fine-grained 선이 coarse-grained보다 위로 올라가는 지점이 생기는데, 이 지점을 근거로 “lock granularity를 줄이는 것만으로 항상 성능이 좋아지는 것은 아니다”라고 해석할 수 있다.

## 7. 새롭게 배운 점 / 어려웠던 점

이번 과제를 통해 같은 자료구조라도 lock을 어디에 적용하느냐에 따라 correctness와 성능이 크게 달라진다는 점을 배울 수 있었다. 보호 범위가 너무 넓으면 병렬성이 사라지고, 너무 좁으면 자료구조 무결성을 유지하기 어려워진다는 점이 인상적이었다.

특히 fine-grained lock 구현에서는 동시성 제어와 자료구조 조작을 함께 생각해야 해서 더 어려웠다. remove 연산은 연결 리스트 포인터를 수정하므로 lock 없이 수행하면 구조가 쉽게 깨질 수 있고, traversal처럼 전체 상태를 읽는 연산은 bucket별 lock만으로는 부족하다는 점도 중요했다.

또 하나 배운 점은 멀티 스레드 프로그램에서는 같은 코드라도 실행할 때마다 스레드 스케줄링 순서가 달라질 수 있다는 것이다. 따라서 interleaving에 따라 결과가 달라질 수 있는 연산과, 항상 deterministic한 연산을 구분해서 이해해야 했다.

## 8. 결론

이번 과제에서는 Hash Table을 기반으로 lock이 없는 버전, coarse-grained lock 버전, fine-grained lock 버전을 구현하고 비교하였다. 실험 결과를 종합하면 fine-grained lock은 대부분의 multi-thread workload에서 더 좋은 확장성을 보였고, 특히 delete가 포함된 경쟁적인 workload에서 강점을 보였다.

반면 fine-grained lock이 항상 무조건 빠른 것은 아니며, workload 특성과 key 분포, thread 수에 따라 coarse-grained보다 느릴 수도 있음을 확인했다. 따라서 동시성 자료구조 설계에서는 correctness를 보장하는 것뿐 아니라, 어떤 범위에 lock을 적용해야 병렬성과 구현 복잡도 사이의 균형을 맞출 수 있는지도 함께 고려해야 한다.
