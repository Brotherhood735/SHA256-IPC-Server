# SHA-256 Hashing Server with System V IPC

A client–server program in C for Linux. A client sends a file to a server process, the server computes its SHA-256 hash, and the hash is sent back.
The two processes communicate only through System V inter-process communication (IPC). I made it as a university project.

## What it is

There are two programs, `server` and `client`, and they communicate using three IPC resources:

- **Shared memory (1 MB).** The client reads the file straight into a shared memory segment, so the file's contents are never copied through a message.
- **A request message queue.** The client sends the file's name, its size and its own process ID (PID).
- **A response message queue.** The server sends back the 32-byte hash, tagged with the client's PID so that each client receives only its own answer.

Both programs generate the same keys with `ftok`, so they find the same queues and memory segment without passing anything to each other directly.
The server runs in a loop waiting for requests and computes each hash with OpenSSL.
The client prints the result as a 64-character hex string.
