#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>

#include <string.h>
#include <strings.h>

#include "short.h"

#define HANDLECHARS 62
#define HANDLELEN 16

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

/*
int read_line(int fd, char* buffer, int buflen) {
	int i = 0;
	int err;

	for(;;) {
		err = read(fd, &buffer[i], 1);
		if (err == -1) {
			perror("read");
			exit(1);
		} else if (err == 0 || buffer[i] == '\n') {
			buffer[i] = '\0';
			break;
		}
		i++;
	}

	return i;
}*/

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
/*
int main(int argc, char** argv) {
	struct hashtable ht;
	int size;
	if (argc != 2 || !(size = atoi(argv[1])))
		size = 100;
	ht = ht_init(size);


	char url[1024];
	char handle[HANDLELEN];
	char* url2;
	for(;;) {
		int len = read_line(STDIN_FILENO, url, 1024);
		if (len == 0) break;
		url[len] = '\0';
		url2 = malloc(len+1);
		memcpy(url2, url, len+1);
		uint64_t urlid = next_id();
		int k = ht_insert(&ht, urlid, url2);
		key_to_str(urlid, handle);
		uint64_t back = str_to_key(handle);
		printf("url %s saved to index key %d with key %ld (url: %s = %ld)\n", url, k, urlid, handle, back);
	}

	char idstr[HANDLELEN];
	printf("get by key: ");
	fflush(stdout);
	read_line(STDIN_FILENO, idstr, 10);
	uint64_t id = str_to_key(idstr);
	printf("getting id %ld\n", id);

	url2 = ht_get(&ht, id);

	printf("here: %s\n", url2);

	return 0;
}*/
