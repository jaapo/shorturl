#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <signal.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>

#include "short.h"
#include "hasht.h"

#define SYSCALLERR(a,b) if(a == -1) {perror(b);exit(1);}
#define MIN(a,b) (a<b?a:b)
#define STREQU(a,b) !strncmp(a,b,MIN(strlen(a),strlen(b)))

#define BUFSIZE 2048

static struct hashtable ht;

int error_response(char* buffer, int buflen, int code) {
	int len;
	char* errtxt;

	switch (code) {
		case 400:
			errtxt = "Bad Request";
			break;
		case 404:
			errtxt = "Not Found";
			break;
		case 413:
			errtxt = "Request Entity Too Large";
			break;
		case 500:
		default:
			code = 500;
			errtxt = "Internal Server Error";
	}

	len = snprintf(buffer, buflen, "HTTP/1.1 %d %s\r\n\r\n", code, errtxt);
	if (len>=buflen) {
		printf("too small buffer for error response");
		exit(1);
	}

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
	char buffer[BUFSIZE];
	char content[BUFSIZE];
	int reslen, len;
	char *line, *req, *res;

	len = read(sd, buffer, sizeof buffer);
	if (len == sizeof buffer) {
		printf("too long request\n");
		reslen = error_response(buffer, sizeof buffer, 413);
		goto done;
	}
	
	res = buffer;
	req = strtok(buffer, "\n");

	int content_len = 0;
	while( (line = strtok(NULL, "\n") )) {
		if (STREQU(line, "Content-Length: ")) {
			content_len = atoi(line+16);

			if (content_len > sizeof content-1) {
				printf("too long request\n");
				reslen = error_response(buffer, sizeof buffer, 413);
				goto done;
			}
		} else if (line[0] == '\r' && line[1] == '\0') {
			//end of header, rest is content
			line = strtok(NULL, "\n");
			memcpy(content, line, content_len);
			content[content_len]='\0';
			break;
		}
	}

	if (STREQU("GET ", req)) {
		char* surl = strtok(req+4, " ");
		char* url = get_url(&ht, str_to_key(surl[0]=='/' ? surl+1 : surl));

		if (url) {
			reslen = snprintf(buffer, sizeof buffer, "HTTP/1.1 301 Moved Permanently\r\nLocation: %s\r\n\r\n", url);
		} else {
			reslen = error_response(buffer, sizeof buffer, 404);
		}

		if (reslen >= sizeof buffer) {
			reslen = error_response(buffer, sizeof buffer, 500);
			printf("too small buffer\n");
		}
	} else if (STREQU("POST /shorten ", req)) {
		char* tmp = strtok(content, "=\n");

		if (!STREQU(tmp, "url")) {
			reslen = error_response(buffer, sizeof buffer, 400);
			printf("unknown parameter: %s\n", tmp);
			goto done;
		}

		char* url = strtok(NULL, "=\n");
		if ( (len = www_decode(url)) == -1 ) {
			reslen = error_response(buffer, sizeof buffer, 400);
			printf("param decoding error\n");
			goto done;
		}

		uint64_t urlid = save_url(&ht, url, len);
		char shorturl[HANDLELEN];
		int surl_len = strlen(shorturl);

		key_to_str(urlid, shorturl);
		printf("shortened %s => %s\n", url, shorturl);
		reslen = snprintf(buffer, sizeof buffer, "HTTP/1.1 200 OK\r\nLength: %d\r\nContent-Type: text/plain\r\n\r\n%s", surl_len, shorturl);
	} else {
		reslen = error_response(buffer, sizeof buffer, 400);
		printf("unknown request %s\n", req);
		goto done;
	}

done:
	write(sd, res, reslen);
}

int main(int argc, char** argv) {
	if (argc != 2) {
		printf("usage: %s port\n", argv[0]);
		exit(1);
	}

	ht = ht_init(10);

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
	char cliaddrstr[INET_ADDRSTRLEN];
	for(;;) {
		clisd = accept(listsd, (struct sockaddr*) cliaddr, &cliaddrlen);
		SYSCALLERR(clisd, "accept");

		if (inet_ntop(AF_INET, (struct sockaddr*) &cliaddr, cliaddrstr, cliaddrlen) == NULL)
			SYSCALLERR(-1, "inet_ntop")
		printf("connection from %s\n", cliaddrstr);

		httpreq(clisd);
		close(clisd);
	}

	return 0;
}
