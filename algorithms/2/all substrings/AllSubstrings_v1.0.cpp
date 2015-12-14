#include <iostream>
#include <iterator>
#include <string>
#include <vector>
#include <array>
#include <limits>
#include <algorithm>
#include <stdexcept>



std::vector<size_t> inverse_permutation(std::vector<size_t>::const_iterator begin, std::vector<size_t>::const_iterator end)
{
	std::vector<size_t> result(*std::max_element(begin, end)+1);

	for (auto it = begin; it < end; it++) {
		result[*it] = it - begin;
	}

	return result;
}


class SuffixArray {
public:
	SuffixArray() {}

	template <typename InputIterator>
	SuffixArray(InputIterator begin, InputIterator end)
	{
		array = build_suffix_array(begin, end);
	}

	SuffixArray(const SuffixArray& that)
	{
		*this = that;
	}

	SuffixArray& operator=(const SuffixArray& that)
	{
		array = that.array;

		return *this;
	}

	size_t suffix_length(size_t n) const
	{
		return array.size() - array[n];
	}

	size_t operator[](size_t n) const
	{
		return array[n];
	}

	size_t size() const
	{
		return array.size();
	}

	std::vector<size_t>::const_iterator cbegin() const
	{
		return array.cbegin();
	}

	std::vector<size_t>::const_iterator cend() const
	{
		return array.cend();
	}

private:
	static const size_t TRAILING_SENTINELS_COUNT;
	static const size_t SENTINEL;

	std::vector<size_t> array;

	// Sanders algorithms
	template <typename InputIterator>
	std::vector<size_t> build_suffix_array(InputIterator begin, InputIterator end) const
	{
		std::vector<size_t> in_string = prepare_input_string(begin, end);
		if (in_string.size() <= 4 + TRAILING_SENTINELS_COUNT) {
			// add yet another sentinel to compare substrings of length 4
			in_string.push_back(SENTINEL);
			return substrings_radix_sort(in_string.cbegin(), in_string.cend(), 4);
		}

		std::vector<size_t> s12 = compute_s12(in_string);
		std::vector<size_t> s0  = compute_s0(in_string, s12);

		return merge(in_string, s0, s12);
	}

	template <typename InputIterator>
	std::vector<size_t> prepare_input_string(InputIterator begin, InputIterator end) const
	{
		std::vector<size_t> prepared;

		prepared.reserve(end - begin + TRAILING_SENTINELS_COUNT);
		// cast each element to size_t and free a zero size_t value for the sentinel by adding 1 to each one
		std::transform(begin, end, std::back_inserter(prepared), [](decltype(*begin) value) {
			size_t stvalue = static_cast<size_t>(value);
			if (stvalue == std::numeric_limits<size_t>::max())
				throw std::logic_error("can't allocate zero value for sentinel");
			return stvalue+1;
		});

		prepared.insert(prepared.end(), TRAILING_SENTINELS_COUNT, SENTINEL);

		return prepared;
	}

	// returns sorted substrings indexes
	template <typename RandomAccessIterator>
	std::vector<size_t> substrings_radix_sort(RandomAccessIterator begin, RandomAccessIterator end, size_t length) const
	{
		if (begin + length > end)
			return {};

		size_t max = static_cast<size_t>(*std::max_element(begin, end));
		size_t min = static_cast<size_t>(*std::min_element(begin, end));

		std::vector<std::vector<size_t>> buckets(max-min+1);
		std::vector<size_t> sorted_substrings;

		sorted_substrings.reserve(end - begin - length + 1);
		for (auto it = begin; it < end - length + 1; it++) {
			sorted_substrings.push_back(it-begin);
		}

		for (size_t i = length-1; i != std::numeric_limits<size_t>::max(); i--) {
			for (size_t j = 0; j < sorted_substrings.size(); j++) {
				buckets[static_cast<size_t>(*(begin + sorted_substrings[j] + i))-min].push_back(sorted_substrings[j]);
			}
			buckets_to_array(buckets, sorted_substrings);
		}

		return sorted_substrings;
	}

	void buckets_to_array(std::vector<std::vector<size_t>>& buckets, std::vector<size_t>& array) const
	{
		array.clear();

		for (size_t i = 0; i < buckets.size(); i++) {
			for (size_t j = 0; j < buckets[i].size(); j++) {
				array.push_back(buckets[i][j]);
			}
			buckets[i].clear();
		}
	}

	std::vector<size_t> compute_s0(const std::vector<size_t>& in_string, const std::vector<size_t>& s12) const
	{
		std::vector<size_t> indexes;
		std::vector<size_t> symbols;

		indexes.reserve(in_string.size()*1/3);
		if ((in_string.size()-TRAILING_SENTINELS_COUNT-1)%3 == 0)
			indexes.push_back(in_string.size()-TRAILING_SENTINELS_COUNT-1);

		for (size_t i = 0; i < s12.size(); i++) {
			if (s12[i]%3 == 1) {
				indexes.push_back(s12[i]-1);
			}
		}

		symbols.reserve(in_string.size()*1/3);
		for (size_t i = 0; i < indexes.size(); i++) {
			symbols.push_back(in_string[indexes[i]]);
		}

		std::vector<size_t> sorted = substrings_radix_sort(symbols.begin(), symbols.end(), 1);

		std::vector<size_t> s0(in_string.size()*1/3);
		for (size_t i = 0; i < sorted.size(); i++) {
			s0[i] = indexes[sorted[i]];
		}

		return s0;
	}

	std::vector<size_t> compute_s12(const std::vector<size_t>& in_string) const
	{
		std::vector<size_t> na_string = new_alphabet_string(in_string);
		std::vector<size_t> s12 = build_suffix_array(na_string.cbegin(), na_string.cend());

		// delete previously added separating sentinel (first element) and recount indexes
		std::for_each(s12.begin(), s12.end(), [s12](size_t& value) {
			if (value > *s12.begin()) value--;
		});
		return recount_indexes(s12.cbegin()+1, s12.cend());
	}

	std::vector<size_t> new_alphabet_string(const std::vector<size_t>& old_string) const
	{
		std::vector<size_t> na_string;
		std::vector<size_t> triple_symbol_map(old_string.size()-TRAILING_SENTINELS_COUNT);

		std::vector<size_t> sorted_triples = substrings_radix_sort(old_string.cbegin(), old_string.cend(), 3);

		// find equal triples and assign them the same symbol
		for (size_t i = 0, symbol = 1; i < sorted_triples.size(); i++) {
			triple_symbol_map[sorted_triples[i]] = symbol;
			if (i != sorted_triples.size()-1 &&
				!std::equal(old_string.cbegin() + sorted_triples[i], old_string.cbegin() + sorted_triples[i] + 3,
							old_string.cbegin() + sorted_triples[i+1]) ) {
				symbol++;
			}
		}

		na_string.reserve(triple_symbol_map.size()*2/3);
		for (auto it = triple_symbol_map.cbegin()+1; it < triple_symbol_map.cend(); it+=3) {
			na_string.push_back(*it);
		}
		na_string.push_back(SENTINEL);
		for (auto it = triple_symbol_map.cbegin()+2; it < triple_symbol_map.cend(); it+=3) {
			na_string.push_back(*it);
		}

		return na_string;
	}

	std::vector<size_t> recount_indexes(std::vector<size_t>::const_iterator begin, std::vector<size_t>::const_iterator end) const
	{
		std::vector<size_t> recounted;
		size_t size = end - begin;
		size_t pivot = size/2 + size%2;

		recounted.reserve(size);
		std::transform(begin, end, std::back_inserter(recounted), [pivot](size_t value) {
			return (value < pivot) ? (3*value + 1) : (3*(value - pivot) + 2);
		});

		return recounted;
	}

	std::vector<size_t> merge(std::vector<size_t>& in_string, std::vector<size_t>& s0, std::vector<size_t>& s12) const
	{
		std::vector<size_t> s;
		std::vector<size_t> s12_inv = inverse_permutation(s12.cbegin(), s12.cend());
		// free zero value for sentinel
		std::transform(s12_inv.cbegin(), s12_inv.cend(), s12_inv.begin(), [](size_t value) {
			return value+1;
		});
		s12_inv.insert(s12_inv.end(), TRAILING_SENTINELS_COUNT, SENTINEL);


		auto it0 = s0.cbegin(), it12 = s12.cbegin();
		s.reserve(s0.size() + s12.size());
		while (it0 != s0.cend() && it12 != s12.cend()) {
			if (*it12%3 == 1) {
				std::array<size_t,2> pair0  = {in_string[*it0],  s12_inv[*it0+1]};
				std::array<size_t,2> pair12 = {in_string[*it12], s12_inv[*it12+1]};

				if (pair12 < pair0) {
					s.push_back(*it12++);
				}
				else {
					s.push_back(*it0++);
				}
			}
			else {
				std::array<size_t,3> triple0  = {in_string[*it0],  in_string[*it0+1],  s12_inv[*it0+2]};
				std::array<size_t,3> triple12 = {in_string[*it12], in_string[*it12+1], s12_inv[*it12+2]};

				if (triple12 < triple0) {
					s.push_back(*it12++);
				}
				else {
					s.push_back(*it0++);
				}
			}
		}

		s.insert(s.end(), it0, s0.cend());
		s.insert(s.end(), it12, s12.cend());

		return s;
	}
};

const size_t SuffixArray::TRAILING_SENTINELS_COUNT = 2;
const size_t SuffixArray::SENTINEL = std::numeric_limits<size_t>::min();




class LCPArray {
public:
	LCPArray() {}

	template <typename InputIterator>
	LCPArray(InputIterator begin, InputIterator end, const SuffixArray& sarray)
	{
		array = build_lcp(begin, end, sarray);
	}

	LCPArray(const SuffixArray& that)
	{
		*this = that;
	}

	LCPArray& operator=(const LCPArray& that)
	{
		array = that.array;

		return *this;
	}

	size_t operator[](size_t n) const
	{
		return array[n];
	}

	size_t size() const
	{
		return array.size();
	}

private:
	std::vector<size_t> array;

	// Kasai algorithm
	template <typename InputIterator>
	std::vector<size_t> build_lcp(InputIterator begin, InputIterator end, const SuffixArray& sarray) const
	{
		int length = end - begin;
		std::vector<size_t> lcp(length);
		std::vector<size_t> sarray_inv = inverse_permutation(sarray.cbegin(), sarray.cend());

		for (size_t index1 = 0, offset = 0; index1 < length; index1++) {
			if (offset > 0) {
				offset--;
			}
			if (sarray_inv[index1] == length-1) {
				lcp[length-1] = 0;
				offset = 0;
			}
			else {
				int index2 = sarray[sarray_inv[index1] + 1];
				while (std::max(index1 + offset, index2 + offset) < length && 
						*(begin + index1 + offset) == *(begin + index2 + offset)) {
					offset++;
				}
				lcp[sarray_inv[index1]] = offset;
			}
		}

		return lcp;
	}
};



int main()
{
	std::istream_iterator<char> begin(std::cin), end;
	std::string in_string(begin, end);

	SuffixArray suffix(in_string.cbegin(), in_string.cend());
	LCPArray lcp(in_string.cbegin(), in_string.cend(), suffix);

	size_t substrings_count = 0;
	for (size_t i = 0; i < suffix.size(); i++) {
		substrings_count += suffix.suffix_length(i) - lcp[i];
	}

	std::cout << substrings_count << std::endl;

	return 0;
}