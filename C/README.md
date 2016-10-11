# server-examples
Implementation of servers in C.

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
