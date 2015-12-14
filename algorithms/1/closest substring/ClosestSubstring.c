#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include <stdbool.h>
#include <complex.h>
#include <math.h>

#ifndef M_PI
#define M_PI  3.14159265358979323846
#endif

typedef double complex complex_d;


void *malloc_or_die(size_t bytes)
{
	void *result = malloc(bytes);

	if (result == NULL) {
		fprintf(stderr, "memory allocation error");
		exit(1);
	}
	return result;
}

size_t round_up_to_power2(size_t value)
{
	size_t result = 1;

	while (result < value)
		result <<= 1;

	return result;
}

void bit_representation(bool *to, char* from, size_t len, char c)
{
	for (size_t i = 0; i < len; i++) {
		to[i] = (from[i] == c);
	}
}

void vector_sum(int32_t *a, int32_t *b, size_t len)
{
	for (size_t i = 0; i < len; i++) {
		a[i] = a[i] + b[i];
	}
}

size_t find_left_max(int32_t *a, size_t len)
{
	size_t index = SIZE_MAX;
	int32_t max = INT32_MIN;
	size_t currnet;

	for (size_t i = 0; i < len; i++) {
		currnet = len-i-1;
		if (a[currnet] >= max) {
			max = a[currnet];
			index = currnet;
		}
	}
	return index;
}

void fft (complex_d *a, size_t len, bool invert) {
	if (len == 1)
		return;

	complex_d *a_even = malloc_or_die(len/2 * sizeof(complex_d));
	complex_d *a_odd = malloc_or_die(len/2 * sizeof(complex_d));

	for (int i = 0, j = 0; i < len; i += 2, j++) {
		a_even[j] = a[i];
		a_odd[j] = a[i+1];
	}

	fft (a_even, len/2, invert);
	fft (a_odd, len/2, invert);

	double angle = 2*M_PI/len * (invert ? -1 : 1);
	complex_d eps = 1;
	complex_d epsn = cos(angle) + I * sin(angle);

	for (int i = 0; i < len/2; i++) {
		a[i] = a_even[i] + eps * a_odd[i];
		a[i + len/2] = a_even[i] - eps * a_odd[i];
		if (invert)
			a[i] /= 2,  a[i + len/2] /= 2;
		eps *= epsn;
	}

	free(a_even);
	free(a_odd);
}

void cross_correlation(int32_t *result, bool *a, bool* b, size_t len)
{
	complex_d *ca = malloc_or_die(len * sizeof(complex_d));
	complex_d *cb = malloc_or_die(len * sizeof(complex_d));

	for (size_t i = 0; i < len; i++) {
		ca[i] = a[i];
		cb[i] = b[i];
	}

	fft (ca, len, false);
	fft (cb, len, false);

	for (size_t i = 0; i < len; ++i)
		ca[i] = conj(ca[i]) * cb[i];

	fft (ca, len, true);

	for (size_t i = 0; i < len; ++i)
		result[i] = (int32_t)(round(creal(ca[i])));

	free(ca);
	free(cb);
}

size_t most_similar_substring(char *text, size_t text_len,
							  char *pattern, size_t pattern_len,
							  char *symbols, size_t symbols_len)
{
	size_t index = 0;
	size_t new_len = round_up_to_power2(text_len + pattern_len);

	int32_t *ccorrelation_sum = malloc_or_die(new_len * sizeof(int32_t));
	int32_t *ccorrelation_tmp = malloc_or_die(new_len * sizeof(int32_t));
	bool *pattern_bit_rep = malloc_or_die(new_len * sizeof(bool));
	bool *text_bit_rep = malloc_or_die(new_len * sizeof(bool));

	memset(ccorrelation_sum, 0, new_len);
	memset(pattern_bit_rep, 0, new_len);
	memset(text_bit_rep, 0, new_len);

	for (size_t i = 0; i < symbols_len; i++) {
		bit_representation(text_bit_rep, text, text_len, symbols[i]);
		bit_representation(pattern_bit_rep, pattern, pattern_len, symbols[i]);
		cross_correlation(ccorrelation_tmp, pattern_bit_rep, text_bit_rep, new_len);
		vector_sum(ccorrelation_sum, ccorrelation_tmp, new_len);
	}

	index = find_left_max(ccorrelation_sum, text_len - pattern_len + 1);

	free(ccorrelation_sum);
	free(ccorrelation_tmp);
	free(pattern_bit_rep);
	free(text_bit_rep);

	return index;
}

int main()
{
	char *buffer = malloc_or_die((200001)*sizeof(char));

	char symbols[4] = {'A', 'C', 'G', 'T'};
	char *text = NULL;
	char *pattern = NULL;

	size_t symbols_len = sizeof(symbols)/sizeof(symbols[0]);
	size_t text_len = 0;
	size_t pattern_len = 0;

	size_t index = 0;


	scanf("%200000s", buffer);
	text_len = strlen(buffer);
	text = malloc_or_die((text_len + 1) * sizeof(text[0]));
	strncpy(text, buffer, text_len + 1);

	scanf("%200000s", buffer);
	pattern_len = strlen(buffer);
	pattern = malloc_or_die((pattern_len + 1) * sizeof(pattern[0]));
	strncpy(pattern, buffer, pattern_len + 1);

	index = most_similar_substring(text, text_len, pattern, pattern_len,
								   symbols, symbols_len);

	printf("%zu", index + 1);

	free(text);
	free(pattern);
	free(buffer);

	return 0;
}