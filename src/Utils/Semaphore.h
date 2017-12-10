#pragma once
#include <iostream>
#include <thread>
#include <mutex>

class Semaphore {
public:
	Semaphore(long count = 0)
		: count_(count) {
	}

	void Signal() {
		std::unique_lock<std::mutex> lock(mutex_);
		count_=1;
		cv_.notify_one();
	}

	void Wait() {
		std::unique_lock<std::mutex> lock(mutex_);
		cv_.wait(lock, [=] { return count_ > 0; });
		--count_;
	}

private:
	std::mutex mutex_;
	std::condition_variable cv_;
	long count_;
};

