#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdbool.h>


typedef enum {
	TT_INORDER,
	TT_POSTORDER,
} TraversalType;

typedef struct {
	size_t index;
	int32_t value;
} Item;

typedef struct {
	size_t size;
	size_t capacity;
	Item values[];
} Stack;

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

Stack *stack_alloc(size_t n)
{
	Stack *stack = malloc_or_die(sizeof(Stack) + n * sizeof(Item));
	stack->capacity = n;
	stack->size = 0;

	return stack;
}

bool stack_empty(Stack *stack)
{
	return stack->size == 0;
}

void stack_push(Stack *stack, Item item)
{
	if (stack->size == stack->capacity)
		print_and_exit("stack is full");

	stack->values[stack->size++] = item;
}

Item stack_pop(Stack *stack)
{
	if (stack_empty(stack))
		print_and_exit("stack is empty");

	return stack->values[stack->size--];
}

Item stack_top(Stack *stack)
{
	if (stack_empty(stack))
		print_and_exit("stack is empty");

	return stack->values[stack->size-1];
}

void all_nearest_greater(size_t *result, int32_t *values, size_t size)
{
	Stack *stack = stack_alloc(size);
	size_t current;

	for (size_t i = 0; i < size; i++) {
		current = size-i-1;
		while (!stack_empty(stack) && values[current] > stack_top(stack).value)
			stack_pop(stack);

		result[current] = stack_empty(stack) ? size : stack_top(stack).index;
		stack_push(stack, (Item){current, values[current]});
	}

	free(stack);
}

void subtree_print(int32_t *tree_keys, size_t left, size_t right, size_t *nearest_greater, TraversalType ttype)
{
	size_t middle = nearest_greater[left];

	// subtree is empty
	if (left > right) {
		return;
	}

	// subtree is leaf
	if (left == right) {
		printf("%"PRId32" ", tree_keys[left]);
		return;
	}

	subtree_print(tree_keys, left+1, middle-1, nearest_greater, ttype);
	if (ttype == TT_INORDER)
		printf("%"PRId32" ", tree_keys[left]);

	subtree_print(tree_keys, middle, right, nearest_greater, ttype);
	if (ttype == TT_POSTORDER)
		printf("%"PRId32" ", tree_keys[left]);
}

void tree_print(int32_t *tree_keys, size_t *nearest_greater, size_t size, TraversalType ttype)
{
	subtree_print(tree_keys, 0, size - 1, nearest_greater, ttype);
	printf("\n");
}

int main()
{
	int32_t* tree_keys = NULL;
	size_t *nearest_greater = NULL;

	int size;
	scanf("%d", &size);

	tree_keys = malloc_or_die(size * sizeof(tree_keys[0]));
	nearest_greater = malloc_or_die(size * sizeof(nearest_greater[0]));

	for (size_t i = 0; i < size; i++) {
		scanf("%" SCNd32, &tree_keys[i]);
	}

	all_nearest_greater(nearest_greater, tree_keys, size);

	tree_print(tree_keys, nearest_greater, size, TT_POSTORDER);
	tree_print(tree_keys, nearest_greater, size, TT_INORDER);

	free(tree_keys);
	free(nearest_greater);

	return 0;
}