#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>
#include <netdb.h>   
#include <ctype.h>   
#include <stdbool.h> 

#define PORT "8000"
#define BACKLOG 5
#define BUFFER_SIZE 1024

// Function prototypes
void initialization();
void updateState(char *input);
bool isGameOver();
void displayWorld(int client_socket);

// Global variables
struct Quote
{
    char *phrase;
    char *author;
    struct Quote *next;
};
typedef struct Quote Quote;

extern Quote *currentPuzzle;
extern char playerKey[26];

char *msg200 = "HTTP/1.1 200 OK\r\ncontent-type: text/html; charset=UTF-8 \r\n\r\n";

void *handle_request(void *arg);
void send_response(int client_socket, const char *status, const char *content);
void handleGame(int client_socket, char *request);

int main()
{
    char *files_path = "/home/fahdi/downloads/webserve"; // file path
    int server_socket, client_socket;
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage client_addr;
    socklen_t addr_size;
    pthread_t tid;

    // Set up getaddrinfo
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    // Get address information
    if (getaddrinfo(NULL, PORT, &hints, &servinfo) != 0)
    {
        perror("Error in getaddrinfo");
        exit(EXIT_FAILURE);
    }

    // Iterate through the results and bind 
    for (p = servinfo; p != NULL; p = p->ai_next)
    {
        server_socket = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (server_socket == -1)
        {
            perror("Error creating socket");
            continue;
        }

        if (bind(server_socket, p->ai_addr, p->ai_addrlen) == -1)
        {
            close(server_socket);
            perror("Error binding socket");
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo);

    if (p == NULL)
    {
        fprintf(stderr, "Failed to bind socket\n");
        exit(EXIT_FAILURE);
    }

    // Start listening for connections
    if (listen(server_socket, BACKLOG) == -1)
    {
        perror("Error listening on socket");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %s...\n", PORT);

    // Main server loop
    while (1)
    {
        addr_size = sizeof client_addr;
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &addr_size);
        if (client_socket == -1)
        {
            perror("Error accepting connection");
            continue;
        }

        // Create a new thread to handle 
        if (pthread_create(&tid, NULL, handle_request, &client_socket) != 0)
        {
            perror("Error creating thread");
            close(client_socket);
        }
    }

    close(server_socket);

    return 0;
}

// handle client request
void *handle_request(void *arg)
{
    int client_socket = *(int *)arg;
    char buffer[BUFFER_SIZE];

    // Receive request from client
    ssize_t bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_received < 0)
    {
        perror("Error receiving data from client");
        close(client_socket);
        pthread_exit(NULL);
    }

    buffer[bytes_received] = '\0';

    // Parse the request
    char *request = strtok(buffer, " ");
    char *path = strtok(NULL, " ");

    if (strcmp(request, "GET") == 0)
    {
        if (strncmp(path, "/crypt", 6) == 0)
        {
            handleGame(client_socket, path);
        }
        else
        {
            // Serve files
            // TODO: Implement file serving logic
        }
    }
    else
    {
        send_response(client_socket, "HTTP/1.0 400 Bad Request", "<h1>400 Bad Request</h1>");
    }

    // Close client socket and exit thread
    close(client_socket);
    pthread_exit(NULL);
}

// handle game requests
void handleGame(int client_socket, char *request)
{
    if (strstr(request, "?move=") == NULL)
    {
        // New game request
        initialization();
        displayWorld(client_socket);
    }
    else
    {
        // Game request in progress
        char *move = strstr(request, "?move=") + 6;
        updateState(move);
        if (!isGameOver())
        {
            displayWorld(client_socket);
        }
        else
        {
            send_response(client_socket, msg200, "<html><body>Congratulations! You solved it! <a href=\"crypto\">Another?</a></body></html>");
        }
    }
}

// display game state
void displayWorld(int client_socket)
{
    if (currentPuzzle == NULL || currentPuzzle->phrase == NULL)
    {
        // Handle error
        return;
    }

    char response[BUFFER_SIZE];
    int len = snprintf(response, BUFFER_SIZE, "%sEncrypted: %s <br/>Decrypted: ", msg200, currentPuzzle->phrase);
    if (len >= BUFFER_SIZE)
    {
        // Handle error
        return;
    }

    for (int i = 0; i < strlen(currentPuzzle->phrase); i++)
    {
        if (isalpha(currentPuzzle->phrase[i]))
        {
            char keyChar = playerKey[toupper(currentPuzzle->phrase[i]) - 'A'];
            if (keyChar == '\0')
            {
                if (len + 1 >= BUFFER_SIZE)
                {
                    // Handle error
                    return;
                }
                strcat(response, "_");
                len++;
            }
            else
            {
                if (len + 1 >= BUFFER_SIZE)
                {
                    // Handle error
                    return;
                }
                strncat(response, &keyChar, 1);
                len++;
            }
        }
        else
        {
            if (len + 1 >= BUFFER_SIZE)
            {
                // Handle error
                return;
            }
            strncat(response, &currentPuzzle->phrase[i], 1);
            len++;
        }
    }

    if (len + strlen("<br/><form action=\"crypt\"><input type=\"text\" name=\"move\" autofocus maxlength=\"2\"></input></form></body></html>") >= BUFFER_SIZE)
    {
        return;
    }
    strcat(response, "<br/><form action=\"crypt\"><input type=\"text\" name=\"move\" autofocus maxlength=\"2\"></input></form></body></html>");
    send_response(client_socket, msg200, response);
}

// send HTTP response to client
void send_response(int client_socket, const char *status, const char *content)
{
    char response[BUFFER_SIZE];
    sprintf(response, "%s\r\nContent-Length: %lu\r\n\r\n", status, content ? strlen(content) : 0);
    send(client_socket, response, strlen(response), 0);
    if (content)
        send(client_socket, content, strlen(content), 0);
}
