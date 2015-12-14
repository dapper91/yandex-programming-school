#include <algorithm>
#include <atomic>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <mutex>
#include <vector>
#include <thread>
#include <condition_variable>


unsigned debug_flag;

typedef std::chrono::milliseconds Milliseconds;
typedef std::chrono::seconds Seconds;
typedef std::chrono::high_resolution_clock Clock;
typedef Clock::time_point Time;



class Fork {
private:
	std::mutex m;

public:
	Fork(): m() {
	}

	void take()
	{
		m.lock();
	}

	bool try_take()
	{
		return m.try_lock();
	}

	void put()
	{
		m.unlock();
	}
};

class Philosopher;

class Waiter {
public:
	Waiter(unsigned N) : N(N), eatings(0) {
	}

	void ask_for_permission()
	{
		std::unique_lock<std::mutex> lock(mx);
		if (eatings >= N-1)
			cond.wait(lock,[this]{return eatings < N-1;});
		eatings++;
	}

	void finish()
	{
		eatings--;
		cond.notify_one();
	}

private:
	std::mutex mx;
	std::condition_variable cond;
	std::atomic<unsigned> eatings;
	unsigned N;
};


class Philosopher {
public:
	Philosopher(unsigned id, Fork* fork_left, Fork* fork_right, Waiter& waiter,
		unsigned think_delay, unsigned eat_delay) :
		id(id), fork_left(fork_left), fork_right(fork_right), think_delay(think_delay),
		eat_delay(eat_delay), waiter(waiter), stop_flag(false) {
	}

	void run() {
		while (!stop_flag) {
			think();

			waiter.ask_for_permission();

			unsigned current_delay = std::rand() % (min_wait_delay + 1);
			while (true) {
				fork_left->take();
				if (debug_flag) std::printf("[%d] took left fork\n", id);

				if (!fork_right->try_take()) {
					fork_left->put();
					if (debug_flag) std::printf("[%d] put left fork\n", id);
					exponential_backoff(current_delay);
					continue;
				}
				if (debug_flag) std::printf("[%d] took right fork\n", id);
				break;
			}

			eat();

			fork_right->put();
			if (debug_flag) std::printf("[%d] put right fork\n", id);

			fork_left->put();
			if (debug_flag) std::printf("[%d] put left fork\n", id);

			waiter.finish();
		}
	}

	void stop() {
		stop_flag = true;
	}

	void print_stats() {
		std::printf("[%d] %d %d\n", id, eat_count, wait_time);
	}

private:
	unsigned id;
	unsigned think_delay;
	unsigned eat_delay;
	unsigned eat_count = 0;
	unsigned wait_time = 0;

	unsigned min_wait_delay = 4;
	unsigned max_wait_delay = 30;

	Fork* fork_left;
	Fork* fork_right;

	Waiter& waiter;

	Time wait_start;
	std::atomic<bool> stop_flag;


	void think()
	{
		if (debug_flag) std::printf("[%d] thinking\n", id);

		sleep(think_delay);

		if (debug_flag) std::printf("[%d] hungry\n", id);

		wait_start = Clock::now();
	}

	void eat()
	{
		wait_time += std::chrono::duration_cast<Milliseconds>(Clock::now() - wait_start).count();
		if (debug_flag) std::printf("[%d] eating\n", id);
		sleep(eat_delay);
		eat_count++;
	}

	void sleep(int max_delay)
	{
		std::this_thread::sleep_for(Milliseconds( std::rand() % (max_delay + 1) ));
	}

	void exponential_backoff(unsigned& current_delay) {
		sleep(current_delay);

		current_delay *= 2;
		if (current_delay >= max_wait_delay) {
			current_delay = max_wait_delay;
		}
	}
};



int main(int argc, char* argv[]) {
	unsigned N, duration, think_delay, eat_delay;
	std::srand((unsigned)std::time(0));

	try {
		if (argc != 6) {
			throw std::logic_error("args error");
		}
		N = atoi(argv[1]);
		duration = atoi(argv[2]);
		think_delay = atoi(argv[3]);
		eat_delay = atoi(argv[4]);
		debug_flag = atoi(argv[5]);
	}
	catch(std::logic_error& err) {
		std::printf("Usage: %s phil_count duration think_delay eat_delay debug_flag\n", argv[0]);
		return 1;
	}

	Waiter waiter(N);

	Fork* forks[N];
	for (unsigned i=0; i<N; i++) {
		forks[i] = new Fork();
	}

	Philosopher* phils[N];
	for (unsigned i=0; i<N; i++) {
		phils[i] = new Philosopher(i + 1, forks[(i + 1) % N], forks[i], waiter,
			think_delay, eat_delay);
	}

	std::thread threads[N];
	for (unsigned i=0; i<N; i++) {
		threads[i] = std::thread(&Philosopher::run, phils[i]);
	}

	std::this_thread::sleep_for(Seconds(duration));

	std::for_each(phils, phils + N, std::mem_fn(&Philosopher::stop));
	std::for_each(threads, threads + N, std::mem_fn(&std::thread::join));
	std::for_each(phils, phils + N, std::mem_fn(&Philosopher::print_stats));

	for (size_t i=0; i<N; i++) {
		delete forks[i];
		delete phils[i];
	}

	return 0;
}