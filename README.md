shortener
==========

This is a simple URL shortener.

Build
-----

	make

Start
-----

	./bin/main.o 8080

Use
---

Shorten URL by sending a POST-request to **/shorten** with a parameter named url

GET-requests to /xxxxx 301 redirected to an URL. If no URL is found, 404 is returned.



