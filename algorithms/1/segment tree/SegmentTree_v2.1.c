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
} InputOperation;

typedef struct {
	int64_t cur_len;
	int64_t max_len;
	size_t buffer;
} Segment;

typedef struct {
	size_t size;
	Segment values[];
} SegmentTree;


typedef struct {
	size_t left;
	size_t right;
} PointIndexPair;

typedef struct {
	size_t operation_index;
	int32_t point;
	bool is_right;
} PointInfo;


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

int PointInfo_cmp(const PointInfo *a, const PointInfo *b)
{
	return (a->point > b->point) ? 1 : (a->point == b->point) ? 0 : -1;
}
DEFINE_VOID(PointInfo_cmp, PointInfo);

size_t get_uniq_points(int32_t *uniq_points, PointIndexPair *operation_points_map, InputOperation *ops, size_t ops_num)
{
	PointInfo *all_points = malloc_or_die(2*ops_num*sizeof(all_points[0]));

	for (size_t i = 0; i < ops_num; i++) {
		// left operation point
		all_points[2*i].operation_index = i;
		all_points[2*i].point = ops[i].left;
		all_points[2*i].is_right = 0;

		// right operation point
		all_points[2*i + 1].operation_index = i;
		all_points[2*i + 1].point = ops[i].right;
		all_points[2*i + 1].is_right = 1;
	}
	qsort(all_points, 2*ops_num, sizeof(all_points[0]), USE_VOID(PointInfo_cmp));

	size_t count = 0;
	for (size_t i = 0; i < 2*ops_num; i++) {
		// add point to uniq points array
		if (i == 0 || uniq_points[count-1] != all_points[i].point) {
			uniq_points[count++] = all_points[i].point;
		}

		// add points indexes to operation-points map
		if (all_points[i].is_right == 0)
			operation_points_map[all_points[i].operation_index].left = count-1;
		else
			operation_points_map[all_points[i].operation_index].right = count-1;
	}

	free(all_points);

	return count;
}

size_t get_segments(int32_t *segments, int32_t *points, size_t size)
{
	for (size_t i = 0; i < size - 1; i++) {
		segments[i] = points[i + 1] - points[i];
	}
	return size - 1;
}

void build_segment_subtree(SegmentTree *stree, int32_t *segments, size_t current, size_t left, size_t right)
{
	stree->values[current].cur_len = 0;
	stree->values[current].buffer = 0;

	if (left == right) {
		stree->values[current].max_len = segments[left];
	}
	else {
		size_t middle = (left + right)/2;

		build_segment_subtree(stree, segments, 2*current, left, middle);
		build_segment_subtree(stree, segments, 2*current + 1, middle + 1, right);
		stree->values[current].max_len = stree->values[2*current].max_len + stree->values[2*current + 1].max_len;
	}
}

SegmentTree *build_segment_tree(int32_t *segments, size_t size)
{
	if (size == 0)
		return NULL;

	SegmentTree *stree = malloc_or_die(sizeof(SegmentTree) + 4*size*sizeof(Segment));

	stree->size = size;
	build_segment_subtree(stree, segments, 1, 0, size - 1);

	return stree;
}

int64_t get_segment_value(Segment segment)
{
	return (segment.buffer != 0) ? segment.max_len : segment.cur_len;
}

int64_t segment_subtree_operation(SegmentTree *stree, size_t current, size_t left, size_t right,
									size_t from, size_t to, SegmentTreeOperation operation)
{
	if (to < left || right < from) {
		return get_segment_value(stree->values[current]);
	}

	if ((from <= left && right <= to) || (left == right)) {
		operation == STO_ADD ? stree->values[current].buffer++ : stree->values[current].buffer--;
		return get_segment_value(stree->values[current]);
	}

	size_t middle = (left + right)/2;
	stree->values[current].cur_len =  segment_subtree_operation(stree, 2*current, left, middle, from, to, operation)
									+ segment_subtree_operation(stree, 2*current + 1, middle + 1, right, from, to, operation);

	return get_segment_value(stree->values[current]);

}

void segment_tree_operation(SegmentTree *stree, size_t from, size_t to, SegmentTreeOperation operation)
{
	segment_subtree_operation(stree, 1, 0, stree->size - 1, from, to, operation);
}

int64_t segment_tree_sum(SegmentTree *stree)
{
	return get_segment_value(stree->values[1]);
}

int main()
{
	size_t ops_count = 0;
	scanf("%zu", &ops_count);

	PointIndexPair *operation_points_map = malloc_or_die(ops_count * sizeof(operation_points_map[0]));
	InputOperation *ops = malloc_or_die(ops_count * sizeof(ops[0]));
	int32_t *points = malloc_or_die(2*ops_count * sizeof(points[0]));
	int32_t *segments = malloc_or_die((2*ops_count - 1) * sizeof(segments[0]));

	char sign;
	for (size_t i = 0; i < ops_count; i++) {
		scanf(" %c", &sign);
		scanf("%"SCNd32, &ops[i].left);
		scanf("%"SCNd32, &ops[i].right);
		ops[i].sign = (sign == '+') ? 1 : 0;
	}

	size_t pts_count = get_uniq_points(points, operation_points_map, ops, ops_count);
	size_t seg_count = get_segments(segments, points, pts_count);

	SegmentTree *stree = build_segment_tree(segments, seg_count);

	size_t from_point, to_point;
	for (size_t i = 0; i < ops_count; i++) {
		from_point = operation_points_map[i].left;
		to_point = operation_points_map[i].right;

		if (from_point != to_point) {
			if (ops[i].sign == 1)
				segment_tree_operation(stree, from_point, to_point-1, STO_ADD);
			else
				segment_tree_operation(stree, from_point, to_point-1, STO_SUB);
		}
		printf("%"PRId64"\n", segment_tree_sum(stree));
	}

	free(stree);
	free(segments);
	free(points);
	free(ops);
	free(operation_points_map);

	return 0;
}