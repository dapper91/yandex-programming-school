#ifndef CONCURRENT_STRUCTS_HPP
#define CONCURRENT_STRUCTS_HPP

#include <queue>
#include <set>
#include <mutex>
#include <condition_variable>



template<typename T>
class concurrent_queue {
public:
	concurrent_queue() {}

	concurrent_queue(concurrent_queue const& that)
	{
		std::lock_guard<std::mutex> lk(that.mx);
		data_queue = that.data_queue;
	}

	void push(const T& value)
	{
		std::lock_guard<std::mutex> lk(mx);
		data_queue.push(value);
		data_cond.notify_one();
	}

	void push(T&& value)
	{
		std::lock_guard<std::mutex> lk(mx);
		data_queue.push(std::move(value));
		data_cond.notify_one();
	}

	std::shared_ptr<T> wait_and_pop()
	{
		std::unique_lock<std::mutex> lk(mx);
		data_cond.wait(lk,[this]{return !data_queue.empty();});
		std::shared_ptr<T> res(std::make_shared<T>(std::move(data_queue.front())));
		data_queue.pop();
		return res;
	}

	std::shared_ptr<T> try_pop()
	{
		std::lock_guard<std::mutex> lk(mx);
		if(data_queue.empty())
			return std::shared_ptr<T>();
		std::shared_ptr<T> res(std::make_shared<T>(std::move(data_queue.front())));
		data_queue.pop();
		return res;
	}

	bool empty() const
	{
		std::lock_guard<std::mutex> lk(mx);
		return data_queue.empty();
	}

	size_t size() const
	{
		return data_queue.size();
	}

private:
	mutable std::mutex mx;
	std::queue<T> data_queue;
	std::condition_variable data_cond;
};


template<typename T>
class concurrent_set {
public:
	concurrent_set() {}

	concurrent_set(concurrent_set const& that)
	{
		std::lock_guard<std::mutex> lk(that.mx);
		data_set = that.data_set;
	}

	void insert(const T& value)
	{
		std::lock_guard<std::mutex> lk(mx);
		data_set.insert(value);
	}

	void erase(const T& value)
	{
		std::lock_guard<std::mutex> lk(mx);
		data_set.erase(value);
	}

	bool contains(const T& value) const
	{
		std::lock_guard<std::mutex> lk(mx);
		return data_set.find(value) != data_set.end();
	}

	bool empty() const
	{
		std::lock_guard<std::mutex> lk(mx);
		return data_set.empty();
	}

private:
	mutable std::mutex mx;
	std::set<T> data_set;
};


#endif