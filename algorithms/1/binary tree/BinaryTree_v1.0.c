#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdbool.h>


typedef struct {
	int32_t item;
	size_t left;
	size_t right;	
} Node;

typedef struct {
	size_t capacity;
	size_t size;
	size_t head;
	Node nodes[];
} Tree;


void *malloc_or_die(size_t bytes)
{
	void *result = malloc(bytes);
	if (result == NULL) {
		fprintf(stderr, "memory allocation error");
		exit(1);
	}
	return result;
}

Tree *tree_alloc(size_t n)
{
	Tree *tree = malloc_or_die(sizeof(Tree) + n * sizeof(Node));
	tree->size = 0;
	tree->capacity = n;
	tree->head = -1;

	return tree;
}

void add_node(Tree *tree, int32_t item, size_t current_node, bool is_next_left)
{	
	size_t next = tree->size++;

	if (current_node == -1) {
		tree->head = 0;
	}
	else {
		if (is_next_left == true)
			tree->nodes[current_node].left = next;
		else
			tree->nodes[current_node].right = next;
	}

	tree->nodes[next].item = item;
	tree->nodes[next].left = -1;
	tree->nodes[next].right = -1;
}

void tree_insert(Tree *tree, int32_t item)
{
	size_t current_node = -1;
	size_t next_node = tree->head;
	bool is_next_left = false;

	if (tree->size == tree->capacity) {
		fprintf(stderr, "tree is full");
		exit(1);
	}

	while (next_node != -1) {			
		current_node = next_node;
		if (item < tree->nodes[current_node].item) {
			next_node = tree->nodes[current_node].left;
			is_next_left = true;
		}
		else {
			next_node = tree->nodes[current_node].right;
			is_next_left = false;
		}
	}

	add_node(tree, item, current_node, is_next_left);
}

void inorder_print(Tree* tree, size_t index)
{
	size_t *stack = malloc_or_die(tree->size * sizeof(size_t));
	size_t stack_top = -1;

	while (stack_top != -1 || index != -1) {
		if (index != -1) {
			stack[++stack_top] = index;
			index = tree->nodes[index].left;
		}
		else {
			index = stack[stack_top--];
			printf("%"PRId32" ", tree->nodes[index].item);	
			index = tree->nodes[index].right;
		}
	}	
}

void postorder_print(Tree* tree, size_t index)
{
	size_t *stack = malloc_or_die(tree->size * sizeof(size_t));
	size_t stack_top = -1;
	size_t parent = -1;
	size_t last_visited = -1;

	while (stack_top != -1 || index != -1) {
		if (index != -1) {
			stack[++stack_top] = index;
			index = tree->nodes[index].left;
		}
		else {
			parent = stack[stack_top];
			if (tree->nodes[parent].right != -1 && last_visited != tree->nodes[parent].right) {
				index = tree->nodes[parent].right;
			}
			else {
				printf("%"PRId32" ", tree->nodes[parent].item);
				last_visited = stack[stack_top--];
			}
		}
	}	
}


int main()
{
	int count = 0;
	int32_t tmp;
	Tree *tree = NULL;

	scanf("%d", &count);
	tree = tree_alloc(count);

	for (size_t i = 0; i < count; i++) {
		scanf("%" SCNd32, &tmp);
		tree_insert(tree, tmp);		
	}

	postorder_print(tree, tree->head);
	printf("\n");
	inorder_print(tree, tree->head);
	printf("\n");

	free(tree);

	return 0;
}