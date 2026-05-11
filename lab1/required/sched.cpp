/*
 *	DKU Operating System Lab (2026)
 *	    Lab1 (Scheduler Algorithm Simulator)
 *	    Student id : 
 *	    Student name :
 */

#include <string>
#include <stdio.h>
#include <iostream>
#include <queue>
#include <algorithm>
#include <deque>
#include <vector>
#include "sched.h"

namespace {
bool is_finished(const Job& job) {
    return job.name == 0 || job.remain_time <= 0;
}

bool is_first_run(const Job& job) {
    return job.first_run_time == 0.0 && job.arrival_time != 0;
}

void save_finished_job(std::vector<Job>& end_jobs, Job& job, double current_time) {
    job.completion_time = current_time;
    end_jobs.push_back(job);
    job = Job();
}
}

class FCFS : public Scheduler
{
private:
    /*
    * 구현 (멤버 변수/함수 추가 및 삭제 가능)
    */
    std::queue<Job> ready_queue_;

    void load_arrived_jobs()
    {
        while (!job_queue_.empty() && job_queue_.front().arrival_time <= current_time_) {
            ready_queue_.push(job_queue_.front());
            job_queue_.pop();
        }
    }

public:
    FCFS(std::queue<Job> jobs, double switch_overhead) : Scheduler(jobs, switch_overhead)
    {
        name = "FCFS";
    }

    int run() override
    {
        /*
        * 구현
        */
        load_arrived_jobs();

        if (is_finished(current_job_)) {
            current_time_ += switch_time_;
            load_arrived_jobs();
            if (ready_queue_.empty()) {
                return -1;
            }

            current_job_ = ready_queue_.front();
            ready_queue_.pop();
            if (is_first_run(current_job_)) {
                current_job_.first_run_time = current_time_;
            }
        }

        int running_job = current_job_.name;
        current_job_.remain_time--;
        current_time_ += 1;

        if (current_job_.remain_time == 0) {
            save_finished_job(end_jobs_, current_job_, current_time_);
        }

        return running_job;
    }
};

class SPN : public Scheduler
{
private:
    /*
    * 구현 (멤버 변수/함수 추가 및 삭제 가능)
    */
    std::vector<Job> ready_jobs_;

    void load_arrived_jobs()
    {
        while (!job_queue_.empty() && job_queue_.front().arrival_time <= current_time_) {
            ready_jobs_.push_back(job_queue_.front());
            job_queue_.pop();
        }
    }

public:
    SPN(std::queue<Job> jobs, double switch_overhead) : Scheduler(jobs, switch_overhead)
    {
        /* 
        * 구현 (멤버 변수/함수 추가 및 삭제 가능)
        */
        name = "SPN";
    }

    int run() override
    {
        /*
        * 구현
        */
        load_arrived_jobs();

        if (is_finished(current_job_)) {
            current_time_ += switch_time_;
            load_arrived_jobs();
            if (ready_jobs_.empty()) {
                return -1;
            }

            auto next_job = std::min_element(
                ready_jobs_.begin(),
                ready_jobs_.end(),
                [](const Job& lhs, const Job& rhs) {
                    if (lhs.service_time != rhs.service_time) {
                        return lhs.service_time < rhs.service_time;
                    }
                    if (lhs.arrival_time != rhs.arrival_time) {
                        return lhs.arrival_time < rhs.arrival_time;
                    }
                    return lhs.name < rhs.name;
                }
            );

            current_job_ = *next_job;
            ready_jobs_.erase(next_job);
            if (is_first_run(current_job_)) {
                current_job_.first_run_time = current_time_;
            }
        }

        int running_job = current_job_.name;
        current_job_.remain_time--;
        current_time_ += 1;

        if (current_job_.remain_time == 0) {
            save_finished_job(end_jobs_, current_job_, current_time_);
        }

        return running_job;
    }
};

class RR : public Scheduler
{
private:
    int time_slice_;
    int left_slice_;
    std::queue<Job> waiting_queue;
    /* 
    * 구현 (멤버 변수/함수 추가 및 삭제 가능)
    */

    void load_arrived_jobs()
    {
        while (!job_queue_.empty() && job_queue_.front().arrival_time <= current_time_) {
            waiting_queue.push(job_queue_.front());
            job_queue_.pop();
        }
    }

    void dispatch_next_job()
    {
        current_time_ += switch_time_;
        load_arrived_jobs();
        if (waiting_queue.empty()) {
            current_job_ = Job();
            return;
        }

        current_job_ = waiting_queue.front();
        waiting_queue.pop();
        left_slice_ = time_slice_;
        if (is_first_run(current_job_)) {
            current_job_.first_run_time = current_time_;
        }
    }

public:
    RR(std::queue<Job> jobs, double switch_overhead, int time_slice) : Scheduler(jobs, switch_overhead)
    {
        name = "RR_" + std::to_string(time_slice);
        /*
         * 위 생성자 선언 및 이름 초기화 코드 수정하지 말것.
         * 나머지는 자유롭게 수정 및 작성 가능 (아래 코드 수정 및 삭제 가능)
         */
        time_slice_ = time_slice;
        left_slice_ = time_slice;
    }

    int run() override
    {
        /* 
        * 구현
        */
        load_arrived_jobs();

        if (!is_finished(current_job_) && left_slice_ == 0) {
            if (waiting_queue.empty()) {
                left_slice_ = time_slice_;
            }
            else {
                waiting_queue.push(current_job_);
                current_job_ = Job();
            }
        }

        if (is_finished(current_job_)) {
            if (waiting_queue.empty()) {
                return -1;
            }
            dispatch_next_job();
        }

        int running_job = current_job_.name;
        current_job_.remain_time--;
        left_slice_--;
        current_time_ += 1;

        if (current_job_.remain_time == 0) {
            save_finished_job(end_jobs_, current_job_, current_time_);
            left_slice_ = time_slice_;
        }

        return running_job;
    }
};


// FeedBack 스케줄러 (queue 개수 : 4 / boosting 없음)
class FeedBack : public Scheduler
{
private:
    /*
    * 구현 (멤버 변수/함수 추가 및 삭제 가능)
    */
    std::deque<Job> ready_queue_[4];
    int time_quantum_[4];
    int current_level_ = 0;
    int left_slice_ = 0;

    void load_arrived_jobs()
    {
        while (!job_queue_.empty() && job_queue_.front().arrival_time <= current_time_) {
            ready_queue_[0].push_back(job_queue_.front());
            job_queue_.pop();
        }
    }

    bool has_ready_job() const
    {
        for (int i = 0; i < 4; i++) {
            if (!ready_queue_[i].empty()) {
                return true;
            }
        }
        return false;
    }

    int find_next_level() const
    {
        for (int i = 0; i < 4; i++) {
            if (!ready_queue_[i].empty()) {
                return i;
            }
        }
        return -1;
    }

    void dispatch_next_job()
    {
        current_time_ += switch_time_;
        load_arrived_jobs();

        int next_level = find_next_level();
        if (next_level == -1) {
            current_job_ = Job();
            left_slice_ = 0;
            return;
        }

        current_level_ = next_level;
        current_job_ = ready_queue_[current_level_].front();
        ready_queue_[current_level_].pop_front();
        left_slice_ = time_quantum_[current_level_];
        if (is_first_run(current_job_)) {
            current_job_.first_run_time = current_time_;
        }
    }

public:
    FeedBack(std::queue<Job> jobs, double switch_overhead, bool is_2i) : Scheduler(jobs, switch_overhead) {
        if (is_2i) {
            name = "FeedBack_2i";
            time_quantum_[0] = 1;
            time_quantum_[1] = 2;
            time_quantum_[2] = 4;
            time_quantum_[3] = 8;
        }
        else {
            name = "FeedBack_1";
            time_quantum_[0] = 1;
            time_quantum_[1] = 1;
            time_quantum_[2] = 1;
            time_quantum_[3] = 1;
        }
        /*
         * 위 생성자 선언 및 이름 초기화 코드 수정하지 말것.
         * 나머지는 자유롭게 수정 및 작성 가능
         */
    }

    int run() override {
        /*
        * 구현
        */
        load_arrived_jobs();

        if (!is_finished(current_job_) && has_ready_job() && find_next_level() < current_level_) {
            ready_queue_[current_level_].push_front(current_job_);
            current_job_ = Job();
            left_slice_ = 0;
        }

        if (!is_finished(current_job_) && left_slice_ == 0) {
            int next_level = std::min(current_level_ + 1, 3);
            if (has_ready_job()) {
                ready_queue_[next_level].push_back(current_job_);
                current_job_ = Job();
            }
            else {
                current_level_ = next_level;
                left_slice_ = time_quantum_[current_level_];
            }
        }

        if (is_finished(current_job_)) {
            if (!has_ready_job()) {
                return -1;
            }
            dispatch_next_job();
        }

        int running_job = current_job_.name;
        current_job_.remain_time--;
        left_slice_--;
        current_time_ += 1;

        if (current_job_.remain_time == 0) {
            save_finished_job(end_jobs_, current_job_, current_time_);
            left_slice_ = 0;
        }

        return running_job;
    }
};
