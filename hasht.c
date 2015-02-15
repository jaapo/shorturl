#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include "hasht.h"

int hash_uint64(struct hashtable *ht, uint64_t keyval) {
	int hashval = 0;
	int i = 0;
	uint8_t* keyar = (uint8_t*) &keyval;

	while (i<8) {
		hashval += keyar[i++];
	}
	if (hashval<0)
		hashval*=-1;
	return hashval%ht->size;
}

int hash(struct hashtable *ht, char* value) {
	int hashval = 0;
	int i = 0;
	while (value[i]) {
		hashval += value[i++];
	}
	return hashval%ht->size;
}

struct hashtable ht_init(int size) {
	struct hashtable ht;
	ht.size = size;
	ht.table = calloc(size, sizeof(struct ht_entry));

	return ht;
}

int ht_insert(struct hashtable *ht, uint64_t key, char* value) {
	struct ht_entry *e;

	int index = hash_uint64(ht, key);
	int count = ht->size;

	for(;;) {
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

	for(;;) {
		e = &ht->table[index];
		if (e->key == key) break;
		index = (index+1)%ht->size;

		if (!count-- || e->key == 0) {
			fprintf(stderr, "not found %ld\n", key);
			return NULL;
		}
	}

	return e->value;
}
