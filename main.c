#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include <string.h>

#include <signal.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "short.h"
#include "hasht.h"

#define SYSCALLERR(a,b) if(a == -1) {perror(b);exit(1);}
#define MIN(a,b) (a<b?a:b)
#define STREQU(a,b) !strncmp(a,b,MIN(strlen(a),strlen(b)))

#define DOCROOT "html/"

static struct hashtable ht;

int file_response(char* buffer, int buflen, char* req_fn) {
	int len = -1;
	int filesize, err, fd, left;
	char filename[1024];
	int fnlen;

	if (req_fn[0] == '/') {
		fnlen = snprintf(filename, sizeof filename, DOCROOT "%s", req_fn+1);
		if (fnlen>sizeof filename) {
			printf("requested filename too long\n");
			exit(1);
		}
	} else {
		printf("requested filename should start with '/'\n");
		exit(1);
	}
	struct stat fstat;

	left = buflen;
	err = stat(filename, &fstat);

	if ((err == -1 && errno == ENOENT) || !S_ISREG(fstat.st_mode)) {
		len = snprintf(buffer, buflen, "HTTP/1.1 404 Not Found\r\n\r\n");
		if (len > buflen) {
			printf("buffer too small\n");
			exit(1);
		}
	} else if (err == 0) {
		fd = open(filename, O_RDONLY);
		SYSCALLERR(fd, "file_response: open");
		filesize = fstat.st_size;
		len = snprintf(buffer, buflen, "HTTP/1.1 200 OK\r\nLength: %d\r\n\r\n", filesize);
		left -= len;
		err = read(fd, &buffer[len], left);
		SYSCALLERR(err, "read");
		if (err != filesize) {
			printf("buffer too small\n");
			exit(1);
		}
		len += err;
	} else {
		SYSCALLERR(err, "file_response: stat()");
	}

	return len;
}

int shortened_response(char* buffer, int buflen, char* shorturl) {
	len = 
	return len;
}

int www_decode(char* str) {
	int i=0,j=0;

	while (str[j]) {
		if (str[j]=='%') {
			unsigned char val;
			if (sscanf(&str[j], "%%%02hhx", &val) == 1) {
				str[i]=val;
			} else {
				return -1;
			}
			j+=3;
		} else {
			str[i] = str[j];
			j++;
		}
		i++;
	}
	str[i] = '\0';
	return i;
}

void httpreq(int sd) {
	char buffer[2048];
	char content[1024];
	read(sd, buffer, sizeof(buffer));
	
	int reslen;
	char *line, *req, *res;
	req = strtok(buffer, "\n");

	int content_len = 0;
	while( (line = strtok(NULL, "\n") )) {
		if (STREQU(line, "Content-Length: ")) {
			content_len = atoi(line+16);

			if (content_len > sizeof content-1) {
				printf("too long request body\n");
				exit(1);
			}
		} else if (line[0] == '\r' && line[1] == '\0') {
			//end of header, rest is content
			line = strtok(NULL, "\n");
			memcpy(content, line, len);
			content[len]='\0';
			break;
		}
	}

	if (STREQU("GET ", req)) {
		char* surl = strtok(req+4, " ");
		char* url = get_url(&ht, str_to_key(surl[0]=='/' ? surl+1 : surl));

		if (url) {
			reslen = snprintf(buffer, sizeof buffer, "HTTP/1.1 301 Moved Permanently\r\nLocation: %s\r\n\r\n", url);
		} else {
			reslen = file_response(buffer, sizeof buffer, surl);
		}

		if (reslen >= sizeof buffer) {
			printf("too small buffer\n");
			exit(1);
		}

		res = buffer;
	} else if (STREQU("POST /shorten ", req)) {
		char* tmp = strtok(content, "=\n");

		if (!STREQU(tmp, "url")) {
			printf("unknown parameter: %s\n", tmp);
			return;
		}

		char* url = strtok(NULL, "=\n");
		if ( (len = www_decode(url)) == -1 ) {
			printf("param decoding error\n");
			exit(1);
		}

		uint64_t urlid = save_url(&ht, url, len);
		char shorturl[32];
		int surl_len = strlen(shorturl);

		key_to_str(urlid, shorturl);
		printf("shortened %lu, %s\n", urlid, shorturl);
		reslen = snprintf(buffer, sizeof buffer, "HTTP/1.1 200 OK\r\nLength: %d\r\nContent-Type: text/plain\r\n\r\n%s", surl_len, shorturl);

		res = buffer;
	} else {
		printf("unknown request%s\n", req);
	}

	write(sd, res, reslen);
}

int main(int argc, char** argv) {
	if (argc != 2) {
		printf("usage: %s port\n", argv[0]);
		exit(1);
	}

	ht = ht_init(100);

	int listsd, err;
	struct sockaddr_in listaddr;
	struct sockaddr_in* cliaddr = NULL;
	socklen_t cliaddrlen = sizeof *cliaddr;

	int port = atoi(argv[1]);
	if (!(port > 0 && port <= 65563)) {
		printf("invalid port number\n");
		exit(1);
	}

	socklen_t listaddrlen = sizeof(listaddr);

	listaddr.sin_family = PF_INET;
	listaddr.sin_port = htons(port);
	listaddr.sin_addr.s_addr = INADDR_ANY;

	listsd = socket(PF_INET, SOCK_STREAM, PF_UNSPEC);
	SYSCALLERR(listsd, "socket")

	int on = 1;
	setsockopt(listsd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof on);

	err = bind(listsd, (struct sockaddr*) &listaddr, listaddrlen);
	SYSCALLERR(err, "bind")
	
	err = listen(listsd, 5);
	SYSCALLERR(err, "listen")

	int clisd;
	for(;;) {
		clisd = accept(listsd, (struct sockaddr*) cliaddr, &cliaddrlen);
		SYSCALLERR(clisd, "accept");
		httpreq(clisd);
		close(clisd);
	}

	return 0;
}
