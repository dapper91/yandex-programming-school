#include <iostream>
#include <vector>


std::vector<size_t> PrefixFunction(const std::string &str) {
	size_t plen;
	size_t slen = str.size();
	std::vector<size_t> pi(slen);

	for (size_t i = 1; i < slen; i++) {
		plen = pi[i-1];

		while (plen > 0 && str[i] != str[plen])
			plen = pi[plen-1];
		if (str[i] == str[plen])
			plen++;
		pi[i] = plen;
	}

	return pi;
}

int main()
{
	std::string in_string;

	std::cin >> in_string;
	std::vector<size_t> pi = PrefixFunction(in_string);

	for (size_t item : pi)
		std::cout << item << " ";
	std::cout << std::endl;

	return 0;
}