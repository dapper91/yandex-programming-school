#include <vector>
#include <algorithm>
#include <functional>
#include <memory>
#include <thread>
#include <future>

#include "concurrent_structs.h"


class ThreadPool {
public:
	ThreadPool(size_t threads = std::thread::hardware_concurrency()) :
		stop(false)
	{
		threads = (threads == 0) ? 1 : threads;

		for(size_t i = 0; i < threads; i++) {
			workers.emplace_back(&ThreadPool::loop, this);
		}
	}

	~ThreadPool()
	{
		stop = true;
		loop_cond.notify_all();

		std::for_each(workers.begin(), workers.end(),
		std::mem_fn(&std::thread::join));
	}

	ThreadPool(const ThreadPool &) = delete;
	ThreadPool(ThreadPool &&) = delete;

	ThreadPool& operator=(const ThreadPool &) = delete;
	ThreadPool& operator=(ThreadPool &&) = delete;

	void loop()
	{
		std::shared_ptr<std::function<void()>> task_ptr;
		std::function<void()> task;

		while (true) {
			{
				std::unique_lock<std::mutex> lk(loop_mx);
				loop_cond.wait(lk, [this]{return !tasks.empty() || stop;});
				if (tasks.empty() && stop)
					return;
				task_ptr = tasks.try_pop();
			}

			task = *task_ptr;
			task();
		}
	}

	template<class Func, class... Args>
	auto add_task(Func func, Args... args) ->
		std::future<typename std::result_of<Func(Args&&...)>::type>
	{
		using ReturnType = typename std::result_of<Func(Args&&...)>::type;

		auto task_ptr = std::make_shared<std::packaged_task<ReturnType()>>(std::bind(func, args...));
		std::future<ReturnType> future = task_ptr->get_future();

		tasks.push([task_ptr]() {
			(*task_ptr)();
		});
		loop_cond.notify_one();

		return future;
	}

private:
	std::vector<std::thread> workers;
	concurrent_queue<std::function<void()>> tasks;

	std::condition_variable loop_cond;
	mutable std::mutex loop_mx;
	std::atomic<bool> stop;
};