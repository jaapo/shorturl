#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include "hasht.h"

#ifdef DEBUG
#define DEBUGMSG(a,...) fprintf(stderr, "DEBUG: " a "\n", ##__VA_ARGS__);
#else
#define DEBUGMSG(a,...)
#endif

int hash_uint64(struct hashtable *ht, uint64_t keyval) {
	unsigned int hashval = 0;
	int i = 0;
	uint8_t* keyar = (uint8_t*) &keyval;

	while (i<8) {
		hashval += keyar[i++];
	}

	DEBUGMSG("hashed %lx to %d", keyval, hashval)
	return hashval%ht->size;
}

struct hashtable ht_init(int size) {
	struct hashtable ht;
	ht.size = size;
	ht.table = calloc(size, sizeof(struct ht_entry));
	DEBUGMSG("hashtable initialized, size: %d", size);

	return ht;
}

int ht_insert(struct hashtable *ht, uint64_t key, char* value) {
	struct ht_entry *e;

	int index = hash_uint64(ht, key);
	int count = ht->size;

	DEBUGMSG("hashtable insert, key: %lu", key);
	for(;;) {
		DEBUGMSG("  index: %d, count: %d", index, count);
		e = &ht->table[index];
		if (e->key == 0) break;
		index = (index+1)%ht->size;

		if (!count--) {
			fprintf(stderr, "table full\n");
			return -1;
		}
	}

	e->key = key;
	e->value = value;
	return index;
}

char* ht_get(struct hashtable *ht, uint64_t key) {
	struct ht_entry *e;

	int index = hash_uint64(ht, key);
	int count = ht->size;

	DEBUGMSG("hashtable get, key: %lu", key);
	for(;;) {
		DEBUGMSG("  index: %d, count: %d", index, count);
		e = &ht->table[index];
		if (e->key == key) break;
		index = (index+1)%ht->size;

		if (!count-- || e->key == 0) {
			DEBUGMSG("  not found, count=%d", count);
			return NULL;
		}
	}

	DEBUGMSG("  found, index=%d", index);
	return e->value;
}
