// List.h - Lists of integers (interface)
// Written by John Shepherd, July 2008

#ifndef List_H
#define List_H

#include <stdio.h>

// External view of List
// Implementation given in List.c

typedef struct ListRep *List;

// create a new empty List
List newList();

// free up all space associated with list
void freeList(List);

// display list as one integer per line on stdout
void showList(List);

// apppend one distinct integer to the end of a list
void ListInsert(List, int);

// delete first occurrence of v from a list
// if v does not occur in List, no effect
void ListDelete(List, int);

// return number of elements in a list
int ListLength(List);

// returns if List contains v
int ListContains(List L, int v);

// display list as one integer per line to a file
// assume that the file is open for writing
void ListPrint(FILE *, List);

#endif
