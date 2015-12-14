#include <string>
#include <algorithm>
#include <regex>



class MyTinyHTMLParser {
public:
	std::vector<std::string> get_urls(const std::string& page, const std::string& base_url,
		const std::vector<std::string> protos)
	{
		std::string base = normolize_url(base_url);
		std::string proto = get_proto(base);

		std::vector<std::string> refs = get_refs(page);
		std::vector<std::string> urls;

		for (auto ref : refs) {
			// complite relative links to absolute
			if (start_with(ref, "//"))
				ref = proto + ":" + ref;
			if (start_with(ref, "/")) 
				ref = base + ref;
			// only protos are acceptable
			if (is_protocol(ref, protos))
				urls.push_back(normolize_url(ref));
		}
		return urls;
	}

	std::string normolize_url(std::string url)
	{
		// delete anchors
		url = url.substr(0, url.find_first_of("#"));
		// delete parameters
		url = url.substr(0, url.find_first_of("?"));

		// delete trailing slash
		while (end_with(url, "/"))
			url.pop_back();

		return url;
	}

	std::string get_proto(const std::string& url) const
	{
		size_t pos = url.find("://");
		if (pos != std::string::npos)
			return url.substr(0, pos);
		return "";
	}

private:
	std::vector<std::string> get_refs(const std::string& page) const
	{
		std::vector<std::string> refs;
		std::regex reg("<a\\s+href\\s*=\\s*\"([^\"]*)\"[^>]*>");
		std::smatch match;

		auto begin = page.cbegin();
		auto end = page.cend();

		while (std::regex_search(begin, end, match, reg)) {
			if (match.size() == 2)
				refs.push_back(match[1]);
			begin = match.suffix().first;
			end = match.suffix().second;
		}

		return refs;
	}

	bool is_protocol(const std::string& str, const std::vector<std::string>& protos) const
	{
		for (auto proto : protos) {
			if (start_with(str, proto + "://"))
				return true;
		}
		return false;
	}

	bool start_with(const std::string& str, const std::string& pattern) const
	{
		if (str.length() >= pattern.length() &&
			str.compare(0, pattern.length(), pattern) == 0)
			return true;
		return false;
	}

	bool end_with(const std::string& str, const std::string& pattern) const
	{
		if (str.length() >= pattern.length() &&
			str.compare(str.length()-pattern.length(), std::string::npos, pattern) == 0)
			return true;
		return false;
	}
};