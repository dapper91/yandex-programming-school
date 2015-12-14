#include <string>
#include <algorithm>
#include <regex>



class Url {
public:
	std::string scheme;
	std::string host;
	std::string path;
	std::string query;
	std::string anchor;

	enum Component {
		SCHEME	= (1u << 0),
		HOST	= (1u << 1),
		PATH	= (1u << 2),
		QUERY	= (1u << 3),
		ANCHOR	= (1u << 4)
	};


	Url(const std::string& url)
	{
		parse_url(url);
	}

	std::string to_string(unsigned mask = HOST | PATH | SCHEME) const
	{
		std::string url;
		std::vector<bool> mask_vector = {
			(mask & SCHEME)	&& scheme.length(),
			(mask & HOST)	&& host.length(),
			(mask & PATH)	&& path.length(),
			(mask & QUERY)	&& query.length(),
			(mask & ANCHOR)	&& anchor.length()
		};

		std::vector<std::string> parts = {
			scheme + "://",
			host,
			path,
			"?" + query,
			"#" + anchor
		};

		for (size_t i = 0; i < mask_vector.size(); i++) {
			if (mask_vector[i]) url += parts[i];
		}
		return url;
	}

	bool operator==(Url& that) const
	{
		return  (scheme == that.scheme) &&
				(host == that.host) &&
				(path == that.path);
	}

	operator const std::string() const
	{
		return to_string();
	}

	bool is_scheme(const std::vector<std::string>& schemas) const
	{
		return std::any_of(schemas.begin(), schemas.end(),
		[this](const std::string& val){return scheme==val;});
	}

	Url& to_absolute(const Url& base)
	{
		if (is_relative()) {
			host = base.host;
			scheme = base.scheme;
		}
		return *this;
	}

private:
	const static std::string default_scheme;

	void parse_url(const std::string& url)
	{
		const static
		std::regex reg( "((([\\w-]+):)?//)?"	// scheme
						"([^\\s:/?#]+)?"		// host
						"(:(\\d+))?"			// port
						"([^\\s?#]*)"			// path
						"(\\?([^\\s#]*))?"		// query
						"(#([\\w]*))?");		// anchor

		std::smatch smatch;
		std::regex_search(url, smatch, reg);
		scheme	= smatch[3];
		host	= smatch[4];
		path	= smatch[7];
		query	= smatch[9];
		anchor	= smatch[11];

		scheme = scheme.size()==0 ? default_scheme : scheme;
	}

	bool is_relative() const
	{
		return host.empty() ? true : false;
	}
};

const std::string Url::default_scheme = "http";


class MyTinyHTMLParser {
public:
	std::vector<std::string> get_urls(const std::string& page, const Url& url,
		const std::vector<std::string> schemas) const
	{
		std::vector<Url> refs = get_refs(page);
		std::vector<std::string> urls;

		for (auto ref : refs) {
			ref.to_absolute(url);
			if (ref.is_scheme(schemas))
				urls.push_back(ref.to_string());
		}
		return urls;
	}

	std::vector<Url> get_refs(const std::string& page) const
	{
		std::vector<Url> refs;
		std::regex reg("<a.+?href\\s*=\\s*\"([^\"]*)\"[^>]*>");
		std::smatch match;

		auto begin = page.cbegin();
		auto end = page.cend();

		while (std::regex_search(begin, end, match, reg)) {
			if (match.size() == 2)
				refs.push_back(Url(match[1]));
			begin = match.suffix().first;
			end = match.suffix().second;
		}

		return refs;
	}
};