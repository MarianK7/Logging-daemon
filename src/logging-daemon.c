#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/un.h> 
#include <unistd.h>

// Message buffer size for reading from the socket
#define BUFFER_SIZE 1024

void write_to_file(const char* path, const char* message) {
    // Open the file for appending
    FILE* fp = fopen(path, "a");
    if (fp == NULL) {
        perror("fopen");
        exit(1);
    }

    // Write the message to the file
    fprintf(fp, "%s\n", message);

    // Close the file
    fclose(fp);
}

void signal_handler(int signo) {
    if (signo == SIGINT) {
        // Unlink the socket file and exit
        unlink("/dev/log");
        printf("\nThe END!\n");
        exit(0);
    }
}

int main(int argc, char* argv[]) {
    int fd, len;
    char buffer[BUFFER_SIZE];

    // Check that at least one file path is provided
    if (argc < 2) {
        fprintf(stderr, "Usage: %s file1 [file2 ...]\n", argv[0]);
        exit(1);
    }

    // Register signal handler for SIGINT
    if (signal(SIGINT, signal_handler) == SIG_ERR) {
        perror("Signal handler registration failed");
        exit(1);
    }

    // Create and bind the socket
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, "/dev/log", sizeof(addr.sun_path) - 1);
    fd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (fd < 0) {
        perror("Socket creation failed");
        exit(1);
    }
    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("Bind failed");
        exit(1);
    }

    // Read messages from the socket and write them to the files
    while (1) {
        len = recv(fd, buffer, sizeof(buffer), 0);
        if (len < 0) {
            perror("Recv failed");
            exit(1);
        }

        // Write the message to each file
        for (int i = 1; i < argc; i++) {
            write_to_file(argv[i], buffer);
        }

        // Print the message to stdout
        printf("%.*s\n", len, buffer);
    }

    return 0;
}
