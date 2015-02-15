#include "hasht.h"

#ifndef SHORT_H
#define SHORT_H
char* key_to_str(uint64_t key, char* str);
uint64_t str_to_key(char* str);
uint64_t save_url(struct hashtable *ht, const char *url, int len);
char* get_url(struct hashtable *ht, uint64_t key);
#endif //SHORT_H
