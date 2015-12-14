#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>


typedef struct {
	size_t index;
	int32_t value;
} item;


void *malloc_or_die(size_t bytes)
{
	void *result = malloc(bytes);
	if (result == NULL) {
		fprintf(stderr, "memory allocation error");
		exit(1);
	}
	return result;
}


void get_nearest_greater(size_t *to, int32_t *from, size_t size)
{	
	item *stack = malloc_or_die(size * sizeof(size_t));
	size_t stack_top = -1;


	for (size_t i = size - 1; i != -1; i--) {
		while (from[i] > stack[stack_top].value && stack_top != -1)
			stack_top--;
		
		if (stack_top == -1) {
			to[i] = -1;
			stack_top++;
			stack[stack_top].value = from[i];
			stack[stack_top].index = i;			
		}
		else {
			to[i] = stack[stack_top].index;
			stack_top++;
			stack[stack_top].value = from[i];
			stack[stack_top].index = i;
		}

	free(stack);
}

void inorder_print(size_t *nearest_greater, int32_t *tree_keys, size_t left, size_t right)
{		
	if (left > right) {
		return;
	}

	if (left == right) {
		printf("%"PRId32" ", tree_keys[left]);
		return;
	}

	size_t pivot = nearest_greater[left];

	if (pivot == -1) {
		inorder_print(nearest_greater, tree_keys, left+1, right);
		printf("%"PRId32" ", tree_keys[left]);
		return;
	}

	inorder_print(nearest_greater, tree_keys, left+1, pivot-1);
	printf("%"PRId32" ", tree_keys[left]);
	inorder_print(nearest_greater, tree_keys, pivot, right);
}

void postorder_print(size_t *nearest_greater, int32_t *tree_keys, size_t left, size_t right)
{		
	if (left > right) {
		return;
	}

	if (left == right) {
		printf("%"PRId32" ", tree_keys[left]);
		return;
	}

	size_t pivot = nearest_greater[left];

	if (pivot == -1) {
		postorder_print(nearest_greater, tree_keys, left+1, right);
		printf("%"PRId32" ", tree_keys[left]);
		return;
	}

	postorder_print(nearest_greater, tree_keys, left+1, pivot-1);	
	postorder_print(nearest_greater, tree_keys, pivot, right);
	printf("%"PRId32" ", tree_keys[left]);
}

int main()
{
	int size = 0;
	
	scanf("%d", &size);
	int32_t* tree_keys = malloc_or_die(size * sizeof(int32_t));
	size_t *nearest_greater = malloc_or_die(size * sizeof(size_t));


	for (size_t i = 0; i < size; i++) {
		scanf("%" SCNd32, &tree_keys[i]);		
	}

	get_nearest_greater(nearest_greater, tree_keys, size);

	postorder_print(nearest_greater, tree_keys, 0, size - 1);	
	printf("\n");
	inorder_print(nearest_greater, tree_keys, 0, size - 1);	
	
	free(tree_keys);
	free(nearest_greater);

	return 0;
}