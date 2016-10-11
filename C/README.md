# server-examples
Implementation of servers in C.

### Server-epoll
Server based on epoll.

Compile:
```sh
clang server-epoll.c -o server-epoll
```

Usage:
```sh
./server-epoll
```

### Server-threads
Server based on threads (one per request).

Compile:
```sh
clang server-threads.c -o server-threads -pthread
```

Usage:
```sh
./server-threads
```

### Tester
Connect and print strings from server.

Compile:
```sh
clang tester.c -o tester -pthread
```

Usage:
```sh
./tester <address> <port> <count of threads> <count of requests>
```
