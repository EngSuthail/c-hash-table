#include <stdlib.h>
#include <string.h>
#include "hash_table.h"
#include <stdio.h>

int main(){
    printf("The hash value for 'cat' is %d\n", ht_hash("cat", 151, 53));
    printf("The hash value for 'dog' is %d", ht_hash("dog", 151, 53));
}