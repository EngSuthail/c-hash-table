#include <stdlib.h>
#include <string.h>
#include "hash_table.h"
#include <stdio.h>

int main(void) {
    // Test hash table creation
    ht_hash_table* ht = ht_new();
    
    printf("Hash table created:\n");
    printf("  Size: %d\n", ht->size);
    printf("  Count: %d\n", ht->count);
    
    // TODO: Test insert/search when implement them
    ht_insert(ht, "cat", "meow");
    ht_insert(ht, "dog", "woof");
    for(int i = 0; i< ht->size; i++){
        if(ht->items[i] != NULL){
            ht_item* item = ht->items[i];
            printf("bucket[%d] | key: %s | value: %s\n", i, item->key, item->value);
        }
    }
    
    // Cleanup
    ht_del_hash_table(ht);
    printf("Hash table deleted successfully\n");
    
    return 0;
}