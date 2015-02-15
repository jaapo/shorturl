#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>

#include <string.h>
#include <strings.h>

#include "short.h"

#define HANDLECHARS 61

static const char* handlealph = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVXYZ0123456789";


char* key_to_str(uint64_t key, char* str) {
	uint64_t tmpw;

	int i;
	for(i=0;i<HANDLELEN && key>0;i++) {
		tmpw = key%HANDLECHARS;
		key /= HANDLECHARS;
		str[i] = handlealph[tmpw];
	}

	str[i] = '\0';
	return str;
}

uint64_t str_to_key(char* str) {
	uint64_t key = 0;
	uint64_t tmpw;
	char* charp;

	for(int i=strlen(str)-1;i>=0;i--) {
		key *= HANDLECHARS;
		charp = index(handlealph, str[i]);
		if (!charp) return 0;
		tmpw = charp - handlealph;
		key += tmpw;
	}

	return key;
}

static uint64_t next_id() {
	static uint64_t shortid = 1;
	shortid *= 1103515245;
	shortid += 12345;
	return shortid%(((uint64_t)1)<<56);
}

uint64_t save_url(struct hashtable *ht, const char *url, int len) {
	char* stored = malloc(len+1);
	memcpy(stored, url, len);
	stored[len] = '\0';

	uint64_t urlid = next_id();
	int k = ht_insert(ht, urlid, stored);
	if (k < 0) {
		printf("error\n");
		return 0;
	}

	return urlid;
}

char* get_url(struct hashtable *ht, uint64_t key) {
	if (key == 0 || ht == NULL) return NULL;
	return ht_get(ht, key);
}
