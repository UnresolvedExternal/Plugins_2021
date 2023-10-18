#pragma once

#include <mutex>
#include <future>
#include <vector>
#include <condition_variable>
#include <queue>

class ThreadPool
{
private:
	std::vector<std::thread> threads;
	std::queue<std::packaged_task<void()>> tasks;
	std::mutex mutex;
	std::condition_variable condVar;
	std::condition_variable joinVar;
	bool exit;

	void Run()
	{
		while (true)
		{
			std::unique_lock lock{ mutex };

			if (tasks.empty())
				joinVar.notify_one();

			condVar.wait(lock, [this]() { return exit || !tasks.empty(); });

			if (exit)
				return;

			auto task{ std::move(tasks.front()) };
			tasks.pop();
			lock.unlock();

			task();
		}
	}

public:
	ThreadPool(size_t concurrency) :
		exit{ false }
	{
		threads.reserve(concurrency);

		for (size_t i = 0; i < concurrency; i++)
			threads.emplace_back(&ThreadPool::Run, this);
	}

	ThreadPool(const ThreadPool&) = delete;
	ThreadPool& operator=(const ThreadPool&) = delete;

	template <class F, class... Args>
	auto Execute(F&& function, Args&&... args)
	{
		auto f = std::bind(std::forward<F>(function), std::forward<Args>(args)...);

		std::unique_lock lock{ mutex };
		const bool wasEmpty = tasks.empty();
		auto future = tasks.emplace(f).get_future();
		lock.unlock();

		if (wasEmpty)
			condVar.notify_one();

		return future;
	}

	void Join()
	{
		std::unique_lock lock{ mutex };
		joinVar.wait(lock, [this]() { return tasks.empty(); });
	}

	~ThreadPool()
	{
		{
			std::lock_guard lock{ mutex };
			exit = true;
		}

		condVar.notify_all();
		
		for (std::thread& thread : threads)
			thread.join();
	}
};