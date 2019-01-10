# Cache-Server
A simple cache server made in C, which caches the 5 most recently visited websites. 

The server receives a request to cache a website from the client, and attempts to do so. This server can serve multiple hosts, one after the other.

**Compilation:** gcc -o server CacheServer.c
**Execution:** ./server 9000

The client sends a request for a website and receives the website from the server. The caching makes this happen faster. 

**Compilation:** gcc -o client client.c
**Execution:** ./client 9000
