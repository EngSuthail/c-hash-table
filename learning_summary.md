# Hash Table Implementation - Learning Summary

## Project Overview
A hash table implementation in C using **double hashing** for collision resolution. This data structure provides O(1) average-case lookup time for key-value pairs.

---

## 1. Structure Definitions

### `ht_item` - Key-Value Pair
```c
typedef struct {
    char* key;      // Pointer to key string
    char* value;    // Pointer to value string
} ht_item;
```

**Purpose:** Stores a single key-value pair in the hash table.

**Why Pointers (`char*`) Instead of Arrays?**
- **Flexibility:** Can store strings of any length
- **Memory Efficiency:** Allocates exactly the memory needed (no waste)
- **Dynamic:** Strings are allocated with `strdup()` on the heap

**Memory Layout:**
```
ht_item at 0x1000
┌──────────────┐
│ key: 0x2000 ─┼──→ "cat\0" (allocated separately)
│ value: 0x3000┼──→ "meow\0" (allocated separately)
└──────────────┘
```

### `ht_hash_table` - The Container
```c
typedef struct {
    int size;           // Number of buckets (53)
    int count;          // Current number of items stored
    ht_item** items;    // Array of pointers to items
} ht_hash_table;
```

**Purpose:** The main hash table structure that manages the buckets.

**Understanding `ht_item** items` (Double Pointer):**
- `ht_item*` = pointer to ONE item
- `ht_item**` = pointer to ARRAY of item pointers
- Creates a **sparse array** - most buckets can be NULL

**Visualization:**
```
ht_hash_table
┌───────────────┐
│ size: 53      │
│ count: 2      │
│ items: ───────┼───→ Array of 53 pointers
└───────────────┘     ┌─────┬─────┬─────┬─────┬────┬─────┐
                      │ [0] │ [1] │ [2] │ [5] │... │[52] │
                      └──┬──┴──┬──┴──┬──┴──┬──┴────┴─────┘
                         ↓     ↓     ↓     ↓
                       NULL  NULL  NULL  ht_item
                                          ┌─────────────┐
                                          │key: "cat"   │
                                          │value: "meow"│
                                          └─────────────┘
```

**Why Array of Pointers?**
- **Memory Efficient:** Empty buckets are just NULL pointers (8 bytes each)
- **Flexible:** Can point to items anywhere in memory
- **Sparse:** If only 5 items in 53 buckets, only 5 items allocated

---

## 2. Header File (hash_table.h)

### Purpose
- **Interface/Contract:** Declares what functions are available
- **Type Definitions:** Defines the data structures
- **Public API:** Shows what users of the library can call

### Structure
```c
#ifndef HASH_TABLE_H    // Include guard start
#define HASH_TABLE_H

// Type definitions
typedef struct { ... } ht_item;
typedef struct { ... } ht_hash_table;

// Public function declarations
ht_hash_table* ht_new(void);
void ht_del_hash_table(ht_hash_table* ht);
void ht_insert(ht_hash_table* ht, const char* key, const char* value);
char* ht_search(ht_hash_table* ht, const char* key);

#endif                  // Include guard end
```

### Key Concepts

**Include Guards:**
- Prevents double inclusion
- Without guards, including the header twice would redefine structures (compiler error)

**Declarations vs Definitions:**
- Header has **declarations** (what exists)
- Source file has **definitions** (how it works)

**Public vs Private:**
- Functions declared in header = **public** (anyone can call)
- Static functions in .c file = **private** (internal helpers only)

---

## 3. Source File (hash_table.c)

### Constants
```c
#define HT_PRIME_1 151
#define HT_PRIME_2 163
```

**Purpose:** Prime numbers for the hash function
- **Better Distribution:** Primes minimize clustering
- **Two Different Values:** Used for double hashing (collision resolution)

---

## 4. Function Implementations

### `static ht_item* ht_new_item(const char* k, const char* v)`

**Purpose:** Create a new key-value pair item.

**Why Static?** Private helper function - users don't create items directly.

**How It Works:**
1. Allocate memory for the struct: `malloc(sizeof(ht_item))`
2. Duplicate the key string: `strdup(k)` (allocates and copies)
3. Duplicate the value string: `strdup(v)` (allocates and copies)
4. Return pointer to the new item

**Memory Allocations:**
```
Three separate allocations:
1. malloc() for struct        (e.g., 16 bytes)
2. strdup() for key string    (e.g., 4 bytes for "cat")
3. strdup() for value string  (e.g., 5 bytes for "meow")

Total: 3 allocations that must ALL be freed later
```

**Key Learning:**
- `strdup()` = `malloc()` + `strcpy()` combined
- Returns pointer to newly allocated string
- Every `strdup()` needs a matching `free()`

---

### `ht_hash_table* ht_new(void)`

**Purpose:** Create and initialize an empty hash table.

**Steps:**
1. Allocate the hash table struct
2. Set `size = 53` (prime number of buckets)
3. Set `count = 0` (no items yet)
4. Allocate array of 53 pointers using `calloc()` (initialized to NULL)

**Why `calloc()` Instead of `malloc()`?**
- `malloc()`: Allocates memory with garbage values
- `calloc()`: Allocates memory **initialized to 0** (all pointers start as NULL)

**Initialization:**
```c
ht->items = calloc(53, sizeof(ht_item*));
// Creates: [NULL][NULL][NULL]...[NULL] (53 NULL pointers)
```

**Return Value:** Pointer to the new hash table (or NULL if allocation fails)

---

### `static void ht_del_item(ht_item* i)`

**Purpose:** Free all memory associated with a single item.

**Critical: Three Frees for Three Allocations**
```c
free(i->key);      // Free key string (from strdup)
free(i->value);    // Free value string (from strdup)
free(i);           // Free the struct itself (from malloc)
```

**Order Matters:**
- Must free strings BEFORE freeing the struct
- If you free the struct first, you lose access to `i->key` and `i->value` (memory leak!)

**Why Static?** Users don't delete individual items - they delete the whole table.

---

### `void ht_del_hash_table(ht_hash_table* ht)`

**Purpose:** Free the entire hash table and all items in it.

**Steps:**
1. Loop through all 53 buckets
2. For each non-NULL bucket, call `ht_del_item()`
3. Free the items array itself
4. Free the hash table struct

**Cleanup Order:**
```
For each item:
  free(item->key)
  free(item->value)
  free(item)
free(ht->items)
free(ht)
```

**Key Learning:**
- Always clean up in **reverse order** of allocation
- Allocated: struct → items array → individual items
- Free: individual items → items array → struct

---

### `static int ht_hash(const char* s, const int a, const int m)`

**Purpose:** Convert a string to a number (hash value).

**Algorithm:**
```c
hash = 0
for each character in string:
    hash += (a^(len-i-1)) * char_value
    hash = hash % m
return hash
```

**Parameters:**
- `s`: String to hash (e.g., "cat")
- `a`: Prime number (151 or 163)
- `m`: Number of buckets (53)

**Example: Hashing "cat"**
```
String: "cat"
a = 151, m = 53

i=0: hash += (151^2) * 'c'  = 22801 * 99  = 2,257,299
     hash %= 53                            = 5

i=1: hash += (151^1) * 'a'  = 151 * 97    = 14,647
     hash = (5 + 14,647) % 53              = 17

i=2: hash += (151^0) * 't'  = 1 * 116     = 116
     hash = (17 + 116) % 53                = 27

Final: hash("cat") = 27
```

**Key Properties:**
1. **Deterministic:** Same input always gives same output
2. **Uniform Distribution:** Spreads values across all buckets
3. **Fast:** O(n) where n is string length
4. **Avalanche Effect:** Small change in input → big change in output

**Why `% m`?**
- Keeps hash value in range [0, m-1]
- Maps any large number to a valid bucket index

---

### `static int ht_get_hash(const char* s, const int num_buckets, const int attempt)`

**Purpose:** Handle collisions using double hashing.

**Formula:**
```c
index = (hash_a + (attempt * (hash_b + 1))) % num_buckets
```

**How It Works:**
```c
hash_a = ht_hash(s, HT_PRIME_1, num_buckets)  // First hash
hash_b = ht_hash(s, HT_PRIME_2, num_buckets)  // Second hash

attempt 0: index = hash_a
attempt 1: index = (hash_a + hash_b + 1) % num_buckets
attempt 2: index = (hash_a + 2*(hash_b + 1)) % num_buckets
attempt 3: index = (hash_a + 3*(hash_b + 1)) % num_buckets
```

**Example: "dog" Collisions**
```
hash_a = ht_hash("dog", 151, 53) = 5
hash_b = ht_hash("dog", 163, 53) = 8

attempt 0: (5 + 0*(8+1)) % 53 = 5   → Try bucket 5
attempt 1: (5 + 1*(8+1)) % 53 = 14  → Try bucket 14
attempt 2: (5 + 2*(8+1)) % 53 = 23  → Try bucket 23
attempt 3: (5 + 3*(8+1)) % 53 = 32  → Try bucket 32
```

**Probe Sequence:** The series of buckets to try: 5 → 14 → 23 → 32 → ...

**Why Two Different Primes?**
- Different probe patterns for different strings
- Better distribution (avoids clustering)
- Minimizes secondary collisions

**Key Insight:**
- Same string + same attempt = same index
- Insert and search follow the **same probe sequence**

---

### `void ht_insert(ht_hash_table* ht, const char* key, const char* value)`

**Purpose:** Add a key-value pair to the hash table.

**Algorithm:**
1. Create the item first
2. Find an empty bucket using the probe sequence
3. Insert the item into that bucket
4. Increment the count

**Step-by-Step:**
```c
// 1. Create item
ht_item* item = ht_new_item(key, value);

// 2. Start with attempt 0
int index = ht_get_hash(key, ht->size, 0);
ht_item* cur_item = ht->items[index];

// 3. Find empty bucket
int i = 1;
while (cur_item != NULL) {
    index = ht_get_hash(key, ht->size, i);
    cur_item = ht->items[index];
    i++;
}

// 4. Insert into empty bucket
ht->items[index] = item;
ht->count++;
```

**Example: Inserting "cat" and "dog"**
```
Initial: All buckets NULL

Insert "cat":
  attempt 0: index = 5, bucket[5] is NULL
  → Insert at bucket[5] ✓

Insert "dog":
  attempt 0: index = 5, bucket[5] occupied by "cat"
  attempt 1: index = 14, bucket[14] is NULL
  → Insert at bucket[14] ✓

Result:
  bucket[5]  = {key: "cat", value: "meow"}
  bucket[14] = {key: "dog", value: "woof"}
  count = 2
```

**Key Learning:**
- Loop continues while `cur_item != NULL` (bucket occupied)
- Stops when finds `NULL` (empty bucket)
- Uses `ht_get_hash()` with increasing attempt numbers
- Probe sequence: deterministic path through buckets

---

### `char* ht_search(ht_hash_table* ht, const char* key)`

**Purpose:** Find the value associated with a key.

**Algorithm:**
1. Follow the **same probe sequence** as insert
2. Check each bucket until:
   - Found the matching key → return value ✓
   - Found empty bucket → key doesn't exist, return NULL

**Step-by-Step:**
```c
// 1. Start with attempt 0
int index = ht_get_hash(key, ht->size, 0);
ht_item* cur_item = ht->items[index];

// 2. Search along probe sequence
int i = 1;
while (cur_item != NULL && strcmp(cur_item->key, key) != 0) {
    index = ht_get_hash(key, ht->size, i);
    cur_item = ht->items[index];
    i++;
}

// 3. Check why loop stopped
if (cur_item == NULL) {
    return NULL;  // Not found
}
return cur_item->value;  // Found it!
```

**Loop Condition Explained:**
```c
while (cur_item != NULL && strcmp(cur_item->key, key) != 0)
```

**Keep looping while BOTH are true:**
1. `cur_item != NULL` → There's something in this bucket
2. `strcmp(...) != 0` → But it's NOT the key we want

**Stop when EITHER is false:**
1. `cur_item == NULL` → Found empty bucket (not found)
2. `strcmp(...) == 0` → Found matching key (found!)

**Example: Searching for "dog"**
```
Table state:
  bucket[5]  = "cat"
  bucket[14] = "dog"

Search "dog":
  attempt 0: index = 5
    cur_item = "cat"
    cur_item != NULL? YES ✓
    strcmp("cat", "dog") != 0? YES ✓
    Continue...

  attempt 1: index = 14
    cur_item = "dog"
    cur_item != NULL? YES ✓
    strcmp("dog", "dog") != 0? NO ✗
    Stop! Found it!

  return "woof"
```

**Example: Searching for "bird" (not in table)**
```
Table state:
  bucket[5]  = "cat"
  bucket[14] = "dog"
  bucket[23] = NULL

Search "bird":
  attempt 0: index = 5
    cur_item = "cat"
    strcmp("cat", "bird") != 0? YES ✓
    Continue...

  attempt 1: index = 14
    cur_item = "dog"
    strcmp("dog", "bird") != 0? YES ✓
    Continue...

  attempt 2: index = 23
    cur_item = NULL
    cur_item != NULL? NO ✗
    Stop! Not found.

  return NULL
```

**Critical Insight: Why Search Works**

The hash function guarantees the same probe sequence:
```
Insert "dog": Tries bucket 5 → 14 (inserts here)
Search "dog": Tries bucket 5 → 14 (finds here)

Same key + same attempt = same index
```

**Key Learning:**
- Must check NULL before accessing `cur_item->key`
- Short-circuit evaluation: `&&` stops if first condition is false
- Return `NULL` if not found, `value` if found
- Follow the same probe path that insert used

---

## Key Concepts Summary

### 1. Memory Management
```
Every malloc/calloc/strdup needs a matching free
Free in REVERSE order of allocation
Three allocations per item = three frees per item
```

### 2. Pointers
```
char*      = pointer to string
ht_item*   = pointer to struct
ht_item**  = pointer to array of pointers
```

### 3. Collision Handling
```
Problem: Two keys map to same bucket
Solution: Double hashing (try sequence of buckets)
Key: Insert and search follow same probe sequence
```

### 4. Hash Function Properties
```
Deterministic: Same input → same output
Uniform: Spreads values evenly
Fast: O(n) for string length n
Range: Output always in [0, size-1]
```

### 5. Public vs Private Functions
```
Public (in .h):  ht_new, ht_insert, ht_search, ht_del_hash_table
Private (static): ht_new_item, ht_del_item, ht_hash, ht_get_hash
```

### 6. String Comparison in C
```
❌ str1 == str2     (compares addresses, not content)
❌ str1 != str2     (compares addresses, not content)
✅ strcmp(str1, str2) == 0   (compares content, returns 0 if equal)
✅ strcmp(str1, str2) != 0   (compares content, returns non-0 if different)
```

---

## Performance Characteristics

### Time Complexity
- **Insert:** O(1) average, O(n) worst case (if many collisions)
- **Search:** O(1) average, O(n) worst case
- **Delete:** O(1) average, O(n) worst case

### Space Complexity
- **Fixed overhead:** 53 pointers (53 * 8 bytes = 424 bytes on 64-bit)
- **Per item:** 16 bytes (struct) + key length + value length

### Load Factor
```
Load Factor = count / size
Current: count / 53

When load factor > 0.7, should resize (not yet implemented)
```

---

## Common Pitfalls & Solutions

### 1. Segmentation Fault
```
❌ Problem: Accessing NULL pointer
if (item->key == "cat")  // Crashes if item is NULL

✅ Solution: Check NULL first
if (item != NULL && strcmp(item->key, "cat") == 0)
```

### 2. Memory Leaks
```
❌ Problem: Not freeing all allocations
free(item);  // Leaked key and value strings!

✅ Solution: Free everything
free(item->key);
free(item->value);
free(item);
```

### 3. String Comparison
```
❌ Problem: Using == for strings
if (str1 == str2)  // Compares pointers, not content

✅ Solution: Use strcmp
if (strcmp(str1, str2) == 0)
```

### 4. Infinite Loop in Search
```
❌ Problem: while(1) with no exit condition
while(1) {
    // Could loop forever if table is full
}

✅ Solution: Check for NULL
while(cur_item != NULL && strcmp(...) != 0)
```

---

## Testing Your Implementation

### Basic Tests
```c
ht_hash_table* ht = ht_new();

// Test insert
ht_insert(ht, "cat", "meow");
ht_insert(ht, "dog", "woof");

// Test search - found
char* result = ht_search(ht, "cat");
assert(strcmp(result, "meow") == 0);

// Test search - not found
char* result2 = ht_search(ht, "bird");
assert(result2 == NULL);

// Cleanup
ht_del_hash_table(ht);
```

### Edge Cases to Test
1. Empty table search
2. Duplicate key insert (should this update or ignore?)
3. Many collisions (insert keys that hash to same bucket)
4. Full table (all 53 buckets occupied)
5. NULL key or value

---

## What's Not Implemented Yet

1. **Update existing keys:** Currently inserts duplicate
2. **Delete function:** Remove individual items
3. **Resize:** Grow table when too full
4. **Error handling:** Check malloc/strdup failures
5. **Iteration:** Function to loop through all items

---

## Further Learning

### Next Steps
1. Implement `ht_delete(key)` function
2. Add resize when load factor > 0.7
3. Handle duplicate key inserts (update value)
4. Add comprehensive error handling
5. Write unit tests

### Related Topics
- Separate chaining (alternative collision method)
- Perfect hashing
- Cuckoo hashing
- Robin Hood hashing
- Hash functions (MD5, SHA, etc.)

---

## Key Takeaways

1. **Hash tables trade space for speed:** O(1) lookups by using extra memory
2. **Collisions are inevitable:** Need a resolution strategy (we used double hashing)
3. **Probe sequences are deterministic:** Same key always follows same path
4. **Memory management is manual:** Every allocation needs a free
5. **Pointers enable flexibility:** Dynamic sizing, sparse arrays
6. **Prime numbers matter:** Better distribution, fewer collisions

This implementation demonstrates core data structure concepts that are fundamental to understanding how databases, caches, compilers, and many other systems work under the hood.