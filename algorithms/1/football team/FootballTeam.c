#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>


#define VOID_PREFIX voidfn
#define USE_VOID(name) VOID_PREFIX ## name
#define DEFINE_VOID(name, type) \
int32_t VOID_PREFIX ## name(const void *a, const void *b) \
{ \
	return name((type*)a, (type*)b); \
}



struct item
{
	size_t index;
	int32_t value;
};

typedef struct item item;


int32_t item_cmp(const item *a, const item *b)
{
	return a->value - b->value;
}
DEFINE_VOID(item_cmp, item)

int32_t size_t_cmp(const size_t *a, const size_t *b)
{
	return *a - *b;
}
DEFINE_VOID(size_t_cmp, size_t)

void *malloc_or_die(size_t bytes)
{
	void *result = malloc(bytes);
	if (result == NULL) {
		fprintf(stderr, "memory allocation error");
		exit(1);
	}
	return result;
}

void swap(void *a, void *b, size_t size)
{
	char tmp;
	char *abyte = a;
	char *bbyte = b;

	while (size--) {
		tmp = *abyte;
		*abyte++ = *bbyte;
		*bbyte++ = tmp;
	}
}

void quick_sort(void *a, size_t len, size_t size, int (*compar)(const void*, const void*))
{
	size_t i = 1;
	size_t j = len-1;
	char *array = a;

	if (j <= 0 || i >= len)
		return;

	swap(&array[0], &array[(rand()%(len-1))*size], size);
	char* pivot = &array[0];

	do {
		while (compar(&array[i*size], pivot) < 0 && i < len) i++;
		while (compar(&array[j*size], pivot) > 0 && j > 0) j--;
		if (i <= j) {
			swap(&array[i*size], &array[j*size], size);
			i++; j--;
		}
	} while(i <= j);

	swap(pivot, &array[j*size], size);

	quick_sort(array, j, size, compar);
	quick_sort(&array[i*size], len - i, size, compar);
}

size_t left_edge(item *array, size_t len, int64_t value, size_t start)
{
	size_t index = SIZE_MAX;

	for (size_t i = start; i < len; i++) {
		if (array[i].value > value)
			break;
		index = i;
	}
	return index;
}

void index_copy(size_t *to_array, item *from_array, size_t len)
{
	for (size_t i = 0; i < len; i++) {
		to_array[i] = from_array[i].index;
	}
}

size_t find_max(int64_t *array, int64_t len)
{
	size_t index = SIZE_MAX;
	int64_t max = INT64_MIN;

	for (size_t i = 0; i < len; i++) {
		if (array[i] >= max) {
			max = array[i];
			index = i;
		}
	}
	return index;
}

size_t *get_most_effective_subgroup(item *group, size_t in_len, size_t *out_len, int64_t *efficiency)
{
	if (in_len == 0)
		return NULL;

	int64_t *subgroups_efficiency = malloc_or_die(in_len*sizeof(int64_t));
	int64_t *sum_efficiency = malloc_or_die(in_len*sizeof(int64_t));
	size_t *subgroups_top = malloc_or_die(in_len*sizeof(size_t));
	size_t *top_subgroup_indexes = NULL;

	size_t top_item = 0;
	size_t top_subgroup = 0;
	size_t top_subgroup_size = 0;

	quick_sort(group, in_len, sizeof(group[0]), USE_VOID(item_cmp));

	for (size_t i = 0; i < in_len; i++) {
		sum_efficiency[i] = group[i].value + (i == 0 ? 0 : sum_efficiency[i-1]);
	}

	for (size_t i = 0; i < in_len; i++) {
		top_item = ((i==in_len-1) ? in_len-1 : left_edge(group, in_len, (int64_t)group[i].value + (int64_t)group[i+1].value, top_item));
		subgroups_top[i] = top_item;
		subgroups_efficiency[i] = sum_efficiency[top_item] - ((i==0) ? 0 : sum_efficiency[i-1]);
	}

	top_subgroup = find_max(subgroups_efficiency, in_len);
	top_subgroup_size = subgroups_top[top_subgroup] - top_subgroup + 1;

	top_subgroup_indexes = malloc_or_die(top_subgroup_size*sizeof(size_t));

	index_copy(top_subgroup_indexes, &group[top_subgroup], top_subgroup_size);
	quick_sort(top_subgroup_indexes, top_subgroup_size, sizeof(top_subgroup_indexes[0]), USE_VOID(size_t_cmp));

	*efficiency = subgroups_efficiency[top_subgroup];
	*out_len = top_subgroup_size;

	free(sum_efficiency);
	free(subgroups_top);
	free(subgroups_efficiency);

	return top_subgroup_indexes;
}

int main()
{
	int64_t efficiency = 0;
	size_t *indexes = NULL;
	size_t out_len = 0;
	size_t in_len = 0;

	scanf("%zu", &in_len);

	item *group = malloc_or_die(in_len*sizeof(item));

	for (size_t i = 0; i < in_len; i++) {
		group[i].index = i + 1;
		scanf("%" SCNd32, &(group[i].value));
	}

	indexes = get_most_effective_subgroup(group, in_len, &out_len, &efficiency);

	printf("%" PRId64 "\n", efficiency);
	for (size_t i = 0; i < out_len; i++) {
		printf("%zu ", indexes[i]);
	}

	free(indexes);
	free(group);

	return 0;
}