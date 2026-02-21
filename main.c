#include <stdlib.h>
#include <string.h>
#include "hash_table.h"
#include <stdio.h>

int main(void) {
    ht_hash_table* ht = ht_new();
    
    // Insert items
    ht_insert(ht, "cat", "meow");
    ht_insert(ht, "dog", "woof");
    ht_insert(ht, "mouse", "squeak");
    printf("Count after insert: %d\n", ht->count);  // Should be 3
    
    // Delete middle item
    ht_delete(ht, "dog");
    printf("Count after delete: %d\n", ht->count);  // Should be 2
    
    // Search should still find mouse
    char* result = ht_search(ht, "mouse");
    printf("Found mouse: %s\n", result);  // Should print "squeak"
    
    // Reuse deleted spot
    ht_insert(ht, "bird", "tweet");
    printf("Count after reinsert: %d\n", ht->count);  // Should be 3
    
    // Cleanup (should not crash on sentinel)
    ht_del_hash_table(ht);
    printf("Cleanup successful\n");
    
    return 0;
}