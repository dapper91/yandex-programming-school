#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <inttypes.h>


#define VOID_PREFIX voidfn
#define DEFINE_VOID(name, type)\
int VOID_PREFIX ## name(const void *a, const void *b)\
{\
	return name((type*)a, (type*)b);\
}
#define USE_VOID(name) VOID_PREFIX ## name


typedef enum {
	STO_ADD,
	STO_SUB
} SegmentTreeOperation;

typedef struct {
	bool sign;
	int32_t left;
	int32_t right;
} InOperation;

typedef struct {
	int64_t cur_len;
	int64_t max_len;
	size_t buffer;
} Segment;

typedef struct {
	size_t size;
	Segment *values;
} SegmentTree;

void print_and_exit(const char *mesg)
{
	fputs(mesg, stderr);
	fputs("\n", stderr);
	exit(1);
}

void *malloc_or_die(size_t bytes)
{
	void *result = malloc(bytes);

	if (result == NULL)
		print_and_exit("memory allocation error");

	return result;
}

int int32_cmp(const int32_t *a, const int32_t *b)
{
	return (*a > *b) ? 1 : (*a == *b) ? 0 : -1;
}
DEFINE_VOID(int32_cmp, int32_t);

size_t get_uniq_points(int32_t *points, InOperation *ops, size_t size)
{		
	int32_t *buffer = malloc_or_die(2*size*sizeof(buffer[0]));

	for (size_t i = 0; i < size; i++) {
		buffer[2*i] = ops[i].left;
		buffer[2*i + 1] = ops[i].right;
	}
	qsort(buffer, 2*size, sizeof(buffer[0]), USE_VOID(int32_cmp));	

	size_t count = 0;	
	for (size_t i = 0; i < 2*size; i++) {
		if (i != 0 && points[count-1] == buffer[i]) {
			continue;
		}
		points[count++] = buffer[i];
	}

	free(buffer);

	return count;
}

size_t get_segments(int32_t *segments, int32_t *points, size_t size)
{
	for (size_t i = 0; i < size - 1; i++) {
		segments[i] = points[i + 1] - points[i];
	}
	return size - 1;
}

size_t index_bsearch(void* key, void* base, size_t num, size_t size, int (*compar)(const void*,const void*))
{
	char *right = bsearch(key, base, num, size, compar);
	char *left = base;

	if (right == NULL)
		return SIZE_MAX;

	return (right - left)/size;
}

void build_segment_subtree(Segment *stree, int32_t *segments, size_t current, size_t left, size_t right)
{
	stree[current].cur_len = 0;
	stree[current].buffer = 0;

	if (left == right) {
		stree[current].max_len = segments[left];		
	}
	else {	
		size_t middle = (left + right)/2;	

		build_segment_subtree(stree, segments, 2*current, left, middle);
		build_segment_subtree(stree, segments, 2*current + 1, middle + 1, right);
		stree[current].max_len = stree[2*current].max_len + stree[2*current + 1].max_len;		
	}	
}

Segment *build_segment_tree(int32_t *segments, size_t size)
{
	if (size == 0)
		return NULL;

	Segment *stree = malloc_or_die(4*size*sizeof(stree[0]));
	build_segment_subtree(stree, segments, 1, 0, size - 1);

	return stree;
}

int64_t get_segment_value(Segment segment) 
{
	return (segment.buffer != 0) ? segment.max_len : segment.cur_len;
}

int64_t segment_subtree_operation(Segment *stree, size_t current, size_t left, size_t right, size_t from, size_t to, SegmentTreeOperation operation)
{
	if (to < left || right < from) {		
		return get_segment_value(stree[current]);
	}

	if ((from <= left && right <= to) || (left == right)) {
		operation == STO_ADD ? stree[current].buffer++ : stree[current].buffer--;
		return get_segment_value(stree[current]);
	}

	size_t middle = (left + right)/2;
	stree[current].cur_len = segment_subtree_operation(stree, 2*current, left, middle, from, to, operation) 
						   + segment_subtree_operation(stree, 2*current + 1, middle + 1, right, from, to, operation);	

	return get_segment_value(stree[current]);
	
}

void segment_tree_operation(Segment *stree, size_t from, size_t to, SegmentTreeOperation operation, size_t seg_count)
{
	segment_subtree_operation(stree, 1, 0, seg_count - 1, from, to, operation);
}

int64_t segment_tree_sum(Segment *stree) 
{
	return get_segment_value(stree[1]);
}

int main()
{	
	size_t ops_count = 0;
	scanf("%zu", &ops_count);

	InOperation *ops = malloc_or_die(ops_count * sizeof(ops[0]));
	int32_t *points = malloc_or_die(2*ops_count * sizeof(points[0]));
	int32_t *segments = malloc_or_die((2*ops_count - 1) * sizeof(segments[0]));

	char sign;
	for (size_t i = 0; i < ops_count; i++) {
		scanf(" %c", &sign);
		scanf("%"SCNd32, &ops[i].left);
		scanf("%"SCNd32, &ops[i].right);
		ops[i].sign = (sign == '+') ? 1 : 0;
	}
	
	size_t pts_count = get_uniq_points(points, ops, ops_count);
	size_t seg_count = get_segments(segments, points, pts_count);

	Segment *stree = build_segment_tree(segments, seg_count);

	size_t lpindex;
	size_t rpindex;
	for (size_t i = 0; i < ops_count; i++) {
		lpindex = index_bsearch(&ops[i].left, points, pts_count, sizeof(int32_t), USE_VOID(int32_cmp));
		rpindex = index_bsearch(&ops[i].right, points, pts_count, sizeof(int32_t), USE_VOID(int32_cmp));		

		if (lpindex != rpindex) {
			if (ops[i].sign == 1)
				segment_tree_operation(stree, lpindex, rpindex - 1, STO_ADD, seg_count);
			else
				segment_tree_operation(stree, lpindex, rpindex - 1, STO_SUB, seg_count);
		}		
		printf("%"PRId64"\n", segment_tree_sum(stree));
	}

	free(stree);
	free(segments);
	free(points);
	free(ops);

	return 0;
}