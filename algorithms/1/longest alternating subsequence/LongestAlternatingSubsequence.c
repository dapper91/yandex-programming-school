#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>


void *malloc_or_die(size_t bytes)
{
	void *result = malloc(bytes);
	if (result == NULL) {
		fprintf(stderr, "memory can't be allocated");
			exit(1);
	}
}

void reverseArray(int32_t *array, size_t len)
{
	int32_t tmp;

	for (size_t i = 0; i < len/2; i++)
	{
		tmp = array[i];
		array[i] = array[len - 1 - i];
		array[len - 1 - i] = tmp;
	}
}

size_t findMax(size_t *array, size_t len)
{
	size_t index = -1;
	size_t max = array[0];

	for (size_t i = 0; i < len; i++) {
		if (max <= array[i]) {
			max = array[i];
			index = i;
		}
	}
	return index;
}

void fillArray(size_t *array, size_t len, size_t value)
{
	for (size_t i = 0; i < len; i++) {
		array[i] = value;
	}
}

int32_t *getLAS(int32_t *sequence, size_t in_len, size_t *out_len)
{
	if (in_len == 0) {
		out_len = 0;
		return NULL;
	}

	size_t *length_greater = malloc_or_die(in_len * sizeof(size_t));
	size_t *length_smaller = malloc_or_die(in_len * sizeof(size_t));
	size_t *previous_greater = malloc_or_die(in_len * sizeof(size_t));
	size_t *previous_smaller = malloc_or_die(in_len * sizeof(size_t));
	int32_t *result_subsequence = malloc_or_die(in_len * sizeof(int32_t));

	size_t result_len = 0;
	size_t current_index = 0;
	size_t current_arrays_pair = 0;

	size_t *length_arrays_pair[2] = {length_smaller, length_greater};
	size_t *previous_arrays_pair[2] = {previous_smaller, previous_greater};


	fillArray(length_greater, in_len, 1);
	fillArray(length_smaller, in_len, 1);
	fillArray(previous_greater, in_len, -1);
	fillArray(previous_smaller, in_len, -1);

	reverseArray(sequence, in_len);

	for (size_t i = 0; i < in_len; i++) {
		for (size_t j = 0; j < i; j++) {
			if (sequence[i] < sequence[j] && (length_smaller[i] - 1) <= length_greater[j]) {
				length_smaller[i] = length_greater[j] + 1;
				previous_smaller[i] = j;
			}
			if (sequence[i] > sequence[j] && (length_greater[i] - 1) <= length_smaller[j]) {
				length_greater[i] = length_smaller[j] + 1;
				previous_greater[i] = j;
			}
		}
	}

	size_t index1 = findMax(length_smaller, in_len);
	size_t index2 = findMax(length_greater, in_len);

	if (length_smaller[index1] > length_greater[index2]) {
		current_index = index1;
		current_arrays_pair = 0;
	}
	else {
		current_index = index2;
		current_arrays_pair = 1;
	}

	do {
		result_subsequence[result_len] = sequence[current_index];
		current_index = previous_arrays_pair[current_arrays_pair][current_index];
		current_arrays_pair = (current_arrays_pair + 1) % 2;
		result_len++;
	} while (current_index != -1);

	result_subsequence = realloc(result_subsequence, result_len * sizeof(result_subsequence[0]));
	if (result_subsequence == NULL)
		exit (1);

	*out_len = result_len;

	free(length_greater);
	free(length_smaller);
	free(previous_greater);
	free(previous_smaller);

	return result_subsequence;
}

int main()
{
	size_t in_len = 0;
	size_t out_len = 0;

	scanf("%zu", &in_len);

	int32_t *in_sequence = malloc_or_die(in_len * sizeof(int32_t));

	for (size_t i = 0; i < in_len; i++) {
		scanf("%" SCNd32, &in_sequence[i]);
	}

	int32_t *out_sequence = getLAS(in_sequence, in_len, &out_len);

	for (size_t i = 0; i < out_len; i++) {
		printf("%" PRId32 " ", out_sequence[i]);
	}

	free(in_sequence);
	free(out_sequence);

	return 0;
}