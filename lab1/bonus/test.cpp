#include "sched.cpp"
#include "test_util.h"
#include "gtest/gtest.h"

// 각 TEST_P는 SchedulerTest::SetUp() 이후 실행됨
// 각 TEST_P 실행 이후, SchedulerTest::TearDow()이 실행됨
TEST_P(SchedulerTest, Stride) {
  sched_ = new Stride(job_list_, switch_time_);
}

TEST_P(SchedulerTest, Lottery) {
  sched_ = new Lottery(job_list_, switch_time_);
}

// 각 2가지 TEST_P(스케줄링 기법)는 각각 4가지 경우(workload 2가지 x switch_time 2가지)를 TEST 함 
INSTANTIATE_TEST_CASE_P(Default, SchedulerTest,
  ::testing::Values(
    // std::make_tuple("워크로드 파일", "context_switch 시간"),
    std::make_tuple("A", 0.01),
    std::make_tuple("A", 0.1),
    
    std::make_tuple("B", 0.05),
    std::make_tuple("B", 0.2)
  )
);

int main() {
    ::testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}
