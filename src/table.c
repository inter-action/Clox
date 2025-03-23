#include <stdlib.h>
#include <string.h>

#include "memory.h"
#include "object.h"
#include "table.h"
#include "value.h"

#define TABLE_MAX_LOAD 0.75

// @see https://craftinginterpreters.com/hash-tables.html

// design notes by the author
// - hash map types: open / close
// - hash function must satisfy the following traits
//    - deterministic
//    - uniform
//    - fast

void initTable(Table* table) {
    table->count = 0;
    table->capacity = 0;
    table->entries = NULL;
}

void freeTable(Table* table) {
    FREE_ARRAY(Entry, table->entries, 0);
    // Q: why not use free here?
    // A: - maybe this table gonna be hold by others
    //    - free it may cause a dangling pointer
    //    - by doing this, large chunk of data is freed
    initTable(table);
}

Entry* findEntry(Entry* entries, int capacity, ObjString* key) {
    uint32_t index = key->hash % capacity;

    Entry* tombstone = NULL;
    while (true) {
        Entry* entry = &entries[index];
        // !: there's is no case that entry pointer can be NULL
        // so we only need to check the `key` field
        if (entry->key == key) {
            return entry;
        }

        // suppose we currently have this type of array in our
        // hash table and both a has the same hash value.
        // [NULL, NULL, a, tombstone, a, NULL, NULL]
        //                               ^ here we reached
        if (entry->key == NULL) {
            if (IS_NIL(entry->value)) {
                // we reach to NULL value
                return tombstone != NULL ? tombstone : entry;
            } else { // we found our tombstone
                // we should place a dedicated tombstone type
                // for readability
                // tombstone satisfy both
                //  - entry->key == NULL
                //  - entry->value == BOOL_VAL(true)

                // mark the first tombstone
                if (tombstone == NULL)
                    tombstone = entry;
            }
        }

        // !we're comparing string's pointer, not string itself
        // @see https://craftinginterpreters.com/hash-tables.html#string-interning
        if (entry->key == key || entry->key == NULL) {
            return entry;
        }
        index = (index + 1) % capacity;
    }
}

static void adjustCapacity(Table* table, int capacity) {
    Entry* entries = ALLOCATE(Entry, capacity);
    // c: it's important to initialize all the values after any malloc
    for (int i = 0; i < capacity; i++) {
        entries[i].key = NULL;
        entries[i].value = NIL_VAL;
    }

    // but to this case, we need to refill each data, due to how
    // we implemented hash table, the index will shift. so a simple
    // copy would not work

    // https://craftinginterpreters.com/hash-tables.html#allocating-and-resizing
    // >Back when we were doing a dynamic array, we could just use
    // >realloc() and let the C standard library copy everything over.
    // >That doesn’t work for a hash table. Remember that to choose
    // >the bucket for each entry, we take its hash key modulo the
    // >array size. That means that when the array size changes,
    // >entries may end up in different buckets.

    // need to count cout tombstone nodes
    table->count = 0;
    for (int i = 0; i < table->capacity; i++) {
        Entry* entry = &table->entries[i];
        if (entry->key == NULL)
            continue;

        // re-calc each index of hash table
        Entry* dest = findEntry(entries, capacity, entry->key);
        //            ^ this findEntry ensures tombstone as empty node

        // no need do this.
        // bool isNewKey = dest>key != NULL;
        // if (isNewKey) table->count++;

        dest->key = entry->key;
        dest->value = entry->value;
        table->count++;
    }

    // this is unnecessary as realloc would free the old pointer
    // but keep it here to align with the book
    FREE_ARRAY(Entry, table->entries, table->capacity);
    table->entries = entries;
    table->capacity = capacity;
}

bool tableGet(Table* table, ObjString* key, Value* value) {
    if (table->count == 0)
        return false;

    Entry* entry = findEntry(table->entries, table->capacity, key);
    if (entry->key == NULL) {
        return false;
    }
    *value = entry->value;
    return true;
}

// @returns isNewKey, if the entry is newly created
bool tableSet(Table* table, ObjString* key, Value value) {
    if (table->count + 1 > table->capacity * TABLE_MAX_LOAD) {
        int capacity = GROW_CAPACITY(table->capacity);
        adjustCapacity(table, capacity);
    }

    Entry* entry = findEntry(table->entries, table->capacity, key);
    bool isNewKey = entry->key == NULL;

    if (isNewKey && IS_NIL(entry->value)) {
        //          ^ also not a tombstone
        // we didn't reduce count when insert tombstone upon deletion
        table->count++;
    }

    entry->key = key;
    entry->value = value;

    return isNewKey;
}

bool tableDelete(Table* table, ObjString* key) {
    if (table->count == 0)
        return false;

    Entry* entry = findEntry(table->entries, table->capacity, key);
    if (entry->key == NULL) {
        return false;
    }

    // Place a tombstone in the entry
    entry->key = NULL;
    entry->value = BOOL_VAL(true);
    // disable reduce count, as we need count tombstone in
    // as a factor to our array load
    // entry->count--
    return true;
}

// cp from -> to
void tableAddAll(Table* from, Table* to) {
    for (int i = 0; i < from->capacity; i++) {
        Entry* entry = &from->entries[i];
        if (entry->key == NULL)
            continue;
        tableSet(to, entry->key, entry->value);
    }
}

ObjString* tableFindString(Table* table, char* chars, int length, uint32_t hash) {
    if (table->count == 0)
        return NULL;

    uint32_t index = hash % table->capacity;
    while (true) {
        Entry* entry = &table->entries[index];
        if (entry->key == NULL) {
            // Stop if we find an empty non-tombstone entry.
            if (IS_NIL(entry->value))
                return NULL;
        } else if (entry->key->length == length && entry->key->hash == hash &&
                   memcmp(entry->key->chars, chars, length) == 0) {
            // found the key
            return entry->key;
        }

        index = (index + 1) % table->capacity;
    }
}