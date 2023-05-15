#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdbool.h>

#define BUFFER_SIZE 1024  // Message buffer size for reading from the socket
#define MAX_MESSAGES 1000 // maximum number of unique messages to store

// Struct for storing a message and its frequency
struct message_frequency
{
    char message[BUFFER_SIZE];
    int frequency;
};

struct message_frequency messages[MAX_MESSAGES]; // Array of messages
int num_messages = 0;                            // current number of unique messages
bool found = false;                              // flag for checking if a message is found

void write_to_file(const char *path, const char *message)
{
    // Open the file for appending
    FILE *fp = fopen(path, "a");
    if (fp == NULL)
    {
        perror("fopen");
        exit(1);
    }

    // Write the message to the file
    fprintf(fp, "%s\n", message);

    // Close the file
    fclose(fp);
}

void print_most_frequent_message(struct message_frequency *message_freqs, int num_messages)
{
    int max_freq_index = 0;
    for (int i = 1; i < num_messages; i++)
    {
        if (message_freqs[i].frequency > message_freqs[max_freq_index].frequency)
        {
            max_freq_index = i;
        }
    }
    printf("\nMost frequent message: %s (frequency: %d)\n", message_freqs[max_freq_index].message, message_freqs[max_freq_index].frequency);
}

void signal_handler(int signo)
{
    if (signo == SIGINT)
    {
        // Print the most frequent message
        print_most_frequent_message(messages, num_messages);
        // Unlink the socket file and exit
        unlink("/dev/log");
        printf("\nThe END!\n");
        exit(0);
    }
}

int main(int argc, char *argv[])
{
    int fd, len;
    char buffer[BUFFER_SIZE];

    // Check that at least one file path is provided
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s file1 [file2 ...]\n", argv[0]);
        exit(1);
    }

    // Register signal handler for SIGINT
    if (signal(SIGINT, signal_handler) == SIG_ERR)
    {
        perror("Signal handler registration failed");
        exit(1);
    }

    // Create and bind the socket
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, "/dev/log", sizeof(addr.sun_path) - 1);
    fd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (fd < 0)
    {
        perror("Socket creation failed");
        exit(1);
    }
    if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("Bind failed");
        exit(1);
    }

    // Read messages from the socket and write them to the files
    while (1)
    {
        len = recv(fd, buffer, sizeof(buffer), 0);
        if (len < 0)
        {
            perror("Recv failed");
            exit(1);
        }

        // Write the message to each file
        for (int i = 1; i < argc; i++)
        {
            write_to_file(argv[i], buffer);
        }

        // Print the message to stdout
        printf("%.*s\n", len, buffer);

        char *msg = strstr(buffer, ": "); // find the first occurrence of ": "
        if (msg != NULL)
        {
            msg += 2; // move past ": "
        }
        else
        {
            msg = buffer; // if no delimiter found, use the whole buffer as the message
        }

        // Check if the message already exists in the array
        if (num_messages > 0)
        {
            for (int i = 0; i < num_messages; i++)
            {

                char *msg_arr = strstr(messages[i].message, ": "); // find the first occurrence of ": "
                if (msg_arr != NULL)
                {
                    msg_arr += 2; // move past ": "
                }
                else
                {
                    msg_arr = messages[i].message; // if no delimiter found, use the whole buffer as the message
                }

                //printf("Message in array %d: %s\n", i, msg_arr);

                if (strcmp(msg_arr, msg) == 0)
                {
                    messages[i].frequency = messages[i].frequency + 1;
                    found = true;
                    break;
                }
            }
        }

        // If the message is not found, add it to the array
        if (found == false)
        {
            if (num_messages >= MAX_MESSAGES)
            {
                fprintf(stderr, "Too many unique messages\n");
                exit(1);
            }
            strncpy(messages[num_messages].message, buffer, strlen(buffer));
            messages[num_messages].frequency = 1;
            num_messages++;
        }

        found = false;
    }

    return 0;
}
