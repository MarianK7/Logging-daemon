# Logging daemon

This program reads messages from a Unix socket `/dev/log`, writes them to one or more files specified in the command line arguments, and keeps track of the frequency of each unique message. When the program receives a `SIGINT` signal, it prints the most frequent message to standard output and unlinks the socket file before exiting.

## Usage
To use the program: 
1. Clone the repository
2. Unlink the existing /dev/log with `sudo unlink /dev/log`
3. Stop rsyslog and systemd-journald via `systemctl stop rsyslog` and `systemctl stop systemd-journald`
4. Run the following commands in the terminal:

```
make     # compile the program
make run # run the program with default arguments
```

- Default arguments can be changed by editing the `ARGS` variable in the `makefile`.
- Provide at least one file path to write messages to. 

## Dependencies
This program depends on the following C libraries:
- `stdio.h`
- `stdlib.h`
- `string.h`
- `signal.h`
- `sys/socket.h`
- `sys/un.h`
- `unistd.h`
- `stdbool.h`

## Functionality
The program follows these steps:
1. Check that at least one file path is provided in the command line arguments.
2. Register a signal handler for `SIGINT`.
3. Create and bind a Unix socket `/dev/log`.
4. Read messages from the socket and write them to the specified files.
5. Print the message to standard output.
6. Keep track of the frequency of each unique message.
7. When a `SIGINT` signal is received, print the most frequent message to standard output and unlink the socket file.

## Limitations
- The program supports a maximum of 1000 unique messages. If this limit is reached, the program will exit with an error message.
- The program assumes that messages are delimited by a colon and a space (`": "`), and that the message itself follows the delimiter. If a message does not follow this format, the entire message buffer is treated as the message.
- The program assumes that the length of each message does not exceed 1024 bytes. Messages longer than this limit will be truncated.
