#include <iostream>
#include <fstream>
#include <string>
#include <stdexcept>
#include <utility>

#include "concurrent_structs.h"
#include "thread_pool.h"
#include "html_parser.h"
#include "curl_read.h"


const size_t DEFAULT_THREADS = 2;
const long DEFAULT_TIMEOUT = 10;



class Crawler {
public:
	struct Result
	{
		size_t code;
		Url url;
		std::string message;

		Result(size_t code, Url url, std::string message) :
			code(code), url(url), message(message) {}

		friend std::ostream& operator<<(std::ostream& output, const Result& res)
		{
			output << res.url.to_string() + "\t" + res.message;
			return output;
		}
	};


	Crawler(size_t threads = 1, long timeout = 30) :
		threads(threads), timeout(timeout) {}

	void start(std::string url, size_t depth = 1, size_t count = 1, std::string dir = "./")
	{
		directory = (dir.back() == '/') ? dir : dir + "/";
		max_depth = depth;
		max_count = count;
		cur_count = 0;

		add_task(Url(url), 1);

		while (!results.empty()) {
			Result res = results.wait_and_pop()->get();
			if (res.code == 0)
				std::cout << res << std::endl;
			else
				std::cerr << res << std::endl;
		}
	}

	void add_task(const Url& url, size_t depth)
	{
		{
			std::lock_guard<std::mutex> lk(add_mx);
			if (visited.contains(url) || cur_count >= max_count)
				return;
			visited.insert(url);
			cur_count++;
		}

		auto future = threads.add_task(&Crawler::download, this, url, depth);
		results.push(std::move(future));
	}

	Result download(Url url, size_t depth)
	{
		std::vector<std::string> protos = {"http", "https"};
		std::vector<std::string> links;
		std::string page;

		CURLcode res =  curl_read(url, page, timeout);
		if (res != CURLE_OK) {
			return Result(res, url, curl_easy_strerror(res));
		}

		if (!save_page(page, filter_filename(url.to_string(Url::HOST | Url::PATH)))) {
			return Result(1, url, "file writing error");
		}

		if (depth < max_depth) {
			links = parser.get_urls(page, url, protos);
			for (auto link : links) {
				add_task(link, depth+1);
			}
		}

		return Result(0, url, "saved");
	}

	std::string filter_filename(std::string url) const
	{
		std::replace(url.begin(), url.end(), '/', '_');
		return url;
	}

	bool save_page(const std::string& page, const std::string& filename) const
	{
		std::ofstream file(directory + filename);
		file << page;
		file.close();

		return file.good();
	}

private:
	concurrent_set<std::string>visited;
	concurrent_queue<std::future<Result>> results;

	ThreadPool threads;
	mutable std::mutex add_mx;

	std::string directory;
	size_t max_depth;
	size_t max_count;
	int cur_count;
	long timeout;

	MyTinyHTMLParser parser;
};





int main(int argc, const char* argv[])
{
	std::string message = "Usage: " + std::string(argv[0]) +
			" url max_depth max_count dir [threads] [timeout]";

	std::string url, dir;
	size_t depth, count;
	size_t threads = DEFAULT_THREADS, timeout = DEFAULT_TIMEOUT;

	try {
		if (argc < 5 || argc > 7)
			throw std::logic_error("args error");
		url = argv[1];
		dir = argv[4];
		depth = std::stoul(argv[2]);
		count = std::stoul(argv[3]);
		if (argc > 5) threads = std::stoul(argv[5]);
		if (argc > 6) timeout = std::stoul(argv[6]);
	}
	catch (std::logic_error) {
		std::cerr << message << std::endl;
		return 1;
	}

	Crawler crawler(threads, timeout);
	crawler.start(url, depth, count, dir);

	return 0;
}