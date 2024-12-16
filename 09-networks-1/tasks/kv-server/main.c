#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

enum { CLIENT_BUFFER_SIZE = 1024 };

typedef struct StorageItem {
  char key[PATH_MAX];
  char value[PATH_MAX];
  struct StorageItem* next;
} StorageItem;

typedef struct Storage {
  struct StorageItem* head;
} Storage;

StorageItem* find(Storage* storage, char* key) {
  StorageItem* current = storage->head;
  while (current != NULL) {
    if (strncmp(current->key, key, PATH_MAX) == 0) {
      return current;
    }
    current = current->next;
  }
  return NULL;
}

void set(Storage* storage, char* key, char* value) {
  StorageItem* element = find(storage, key);
  if (element == NULL) {
    element = malloc(sizeof(StorageItem));
    strncpy(element->key, key, PATH_MAX - 1);
    element->next = storage->head;
    storage->head = element;
  }
  strncpy(element->value, value, PATH_MAX - 1);
}

char* get(Storage* storage, char* key) {
  StorageItem* element = find(storage, key);
  if (element == NULL) {
    return "";
  } else {
    return element->value;
  }
}

typedef struct client_node {
  int client_fd;
  struct client_node* next;
} client_node;

void main_cycle(int socket_fd, Storage* storage, client_node* clients_first) {
  while (true) {
    int new_client_fd = accept(socket_fd, NULL, NULL);

    // new client accepted
    if (new_client_fd == -1 && errno != EAGAIN) {
      perror("accept");
      exit(EXIT_FAILURE);
    } else if (new_client_fd != -1) {
      client_node* new_client = malloc(sizeof(client_node));

      new_client->client_fd = new_client_fd;
      new_client->next = clients_first;

      clients_first = new_client;

      fcntl(new_client_fd, F_SETFL,
            fcntl(new_client_fd, F_GETFL, 0) | O_NONBLOCK);
    }

    // parsing commands
    for (client_node* current_client = clients_first; current_client != NULL;
         current_client = current_client->next) {
      char client_buffer[CLIENT_BUFFER_SIZE];

      int client_fd = current_client->client_fd;

      int command_length =
          read(client_fd, client_buffer, CLIENT_BUFFER_SIZE - 1);

      if (command_length == -1 && errno != EAGAIN) {
        perror("read");
        exit(EXIT_FAILURE);
      } else if (command_length == -1) {
        continue;
      }

      client_buffer[command_length] = '\0';

      char* command_tokens_buffer;
      char* command = strtok_r(client_buffer, " ", &command_tokens_buffer);
      char* argument = strtok_r(NULL, " ", &command_tokens_buffer);

      if (command == NULL || argument == NULL) {
        continue;
      }

      // process command
      if (strncmp(command, "get", 3) == 0) {
        StorageItem* found_item = find(storage, argument);

        int item_length = (found_item == NULL ? 0 : strlen(found_item->value));
        char* response = malloc(item_length + 1);

        if (found_item != NULL) {
          strncpy(response, found_item->value, item_length);
        }

        response[item_length] = '\n';

        write(client_fd, response, item_length + 1);
      } else if (strncmp(command, "set", 3) == 0) {
        char* value = strtok_r(NULL, " ", &command_tokens_buffer);

        if (value == NULL) {
          continue;
        }

        set(storage, argument, value);
      }
    }
  }
}

int main(int argc, char* argv[]) {
  Storage* storage = malloc(sizeof(Storage));
  storage->head = NULL;
  int server_port = atoi(argv[1]);
  client_node* clients_first = NULL;

  // creating sockadrr_in struct to work with ipv4 adresses
  struct sockaddr_in localhost;
  localhost.sin_family = AF_INET;
  inet_pton(AF_INET, "127.0.0.1", &localhost.sin_addr);
  localhost.sin_port = htons(server_port);

  // creating socket
  int socket_fd;
  if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("socket creation");
    return EXIT_FAILURE;
  }

  // nonblock, therefore accept, write, read won't block execution
  if (fcntl(socket_fd, F_SETFL, fcntl(socket_fd, F_GETFL, 0) | O_NONBLOCK) ==
      -1) {
    perror("socket set to nonblock");
    return EXIT_FAILURE;
  }

  // bind socket and ip and port (c-style cast to sockaddr struct)
  if (bind(socket_fd, (struct sockaddr*)&localhost,
           sizeof(struct sockaddr_in)) == -1) {
    perror("bind");
    return EXIT_FAILURE;
  }

  // waiting for connections mode
  if (listen(socket_fd, 1 << 10) == -1) {
    perror("listen");
    return EXIT_FAILURE;
  }

  main_cycle(socket_fd, storage, clients_first);

  while (clients_first != NULL) {
    close(clients_first->client_fd);
    client_node* next_client = clients_first->next;

    free(clients_first);

    clients_first = next_client;
  }
  close(socket_fd);

  return EXIT_SUCCESS;
}
