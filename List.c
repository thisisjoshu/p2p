// List.c - Lists of integers
// Written by John Shepherd, July 2008
// 

#include <stdlib.h>
#include <stdio.h>
#include "assert.h"
#include "List.h"

// data structures representing List

struct ListNode {
	int  data;  // value of this list item
	struct ListNode *next;
	            // pointer to node containing next element
};

struct ListRep {
	int  size;  // number of elements in list
	struct ListNode *first;
	            // node containing first value
	struct ListNode *last;
	            // node containing last value
};

// create a new empty List
List newList()
{
	struct ListRep *L;

	L = malloc(sizeof (struct ListRep));
	assert (L != NULL);
	L->size = 0;
	L->first = NULL;
	L->last = NULL;
	return L;
}

// free up all space associated with list
void freeList(List L)
{
	// does nothing ...
}

// display list as one integer per line on stdout
void showList(List L)
{
	ListPrint(stdout, L);
}

// create a new ListNode with value v
// (this function is local to this ADT)
static struct ListNode *newListNode(int v)
{
	struct ListNode *n;

	n = malloc(sizeof (struct ListNode));
	assert(n != NULL);
	n->data = v;
	n->next = NULL;
	return n;
}

// apppend one distinct integer to the end of a list
void ListInsert(List L, int v)
{
	if (ListContains(L, v) == 1)	
		return;
		
	struct ListNode *n;
	
	assert(L != NULL);
	n = newListNode(v);
	if (L->first == NULL)
		L->first = L->last = n;
	else {
		L->last->next = n;
		L->last = n;
	}
	L->size++;
}

// delete first occurrence of v from a list
// if v does not occur in List, no effect
void ListDelete(List L, int v)
{
	struct ListNode *curr, *prev;

	assert(L != NULL);

	// find where v occurs in list
	prev = NULL; curr = L->first;
	while (curr != NULL && curr->data != v) {
		prev = curr;
		curr = curr->next;
	}
	// not found; give up
	if (curr == NULL) return;
	// unlink curr
	if (prev == NULL)
		L->first = curr->next;
	else
		prev->next = curr->next;
	if (L->last == curr)
		L->last = prev;
	L->size--;
	// remove curr
	free(curr);
}

// return number of elements in a list
int ListLength(List L)
{
	assert(L != NULL);
	return L->size;
}

int ListContains(List L, int v) {
	struct ListNode *curr;

	assert(L != NULL);
	for (curr = L->first; curr != NULL; curr = curr->next) {
		if (curr->data == v) return 1;
	}
	
	return 0;
}

// display list as one integer per line to a file
// assume that the file is open for writing
void ListPrint(FILE *outf, List L)
{
	struct ListNode *curr;

	assert(L != NULL);
	int i = 1;
	for (curr = L->first; curr != NULL; curr = curr->next, i++)
		printf("%d. %d\n", i, curr->data);
}
