#include <inttypes.h>

#ifndef HASHT_H
#define HASHT_H
struct hashtable {
	int size;
	struct ht_entry *table;
	void *data;
};

struct ht_entry {
	uint64_t key;
	char* value;
};

int hash(struct hashtable *ht, char* value);
int hash_uint64(struct hashtable *ht, uint64_t keyval);
struct hashtable ht_init(int size);
int ht_insert(struct hashtable *ht, uint64_t key, char* value);
char* ht_get(struct hashtable *ht, uint64_t key);

#endif //HASHT_H
