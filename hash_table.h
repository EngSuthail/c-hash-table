#ifndef HASH_TABLE_H
#define HASH_TABLE_H

typedef struct 
{
    char* key; //
    char* value;
} ht_item;

/*
Why use pointers (char*) instead of arrays (char key[50])?
1) Flexibility: It can handle keys and values of any length.

2) Memory Efficiency: You only allocate exactly as much memory as the string needs, 
rather than guessing a maximum size like 50 or 100.

3) Warning: Because these are pointers, you are responsible for allocating 
(using malloc or strdup) and freeing the memory for the strings manually to avoid memory leaks.
*/

typedef struct 
{
    int size;
    int count;
    ht_item** items;
} ht_hash_table;

/*
'ht_item** items;' creates a dynamic array of pointers, allowing each bucket to either point to an item or stay NULL, 
which saves memory in a sparse table
*/

ht_hash_table* ht_new(void);
void ht_del_hash_table(ht_hash_table* ht);
void ht_insert(ht_hash_table* ht, const char* key, const char* value);
char* ht_search(ht_hash_table* ht, const char* key);
void ht_delete(ht_hash_table* ht, const char* key);
#endif

