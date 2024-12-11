#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

enum {
  MAX_ARGS_COUNT = 256,
  MAX_CHAIN_LINKS_COUNT = 256,
  OUTPUT_BUFFER_MAX_SIZE = 1024
};

typedef struct {
  char* command;
  uint64_t argc;
  char* argv[MAX_ARGS_COUNT];
} chain_link_t;

typedef struct {
  uint64_t chain_links_count;
  chain_link_t chain_links[MAX_CHAIN_LINKS_COUNT];
} chain_t;

void create_chain(char* command, chain_t* chain) {
  // TODO: Parse command and init chain_t object
  char* pipeline_string;
  char* curr_command;
  char* pipe_string;
  char* curr_argument;
  char* command_tokens_buffer;
  char* arg_tokens_buffer;

  chain_link_t* chain_link = chain->chain_links;
  chain->chain_links_count = 0;
  pipeline_string = command;

  while (true) {
    // to get the next token strtok_r first argument must be NULL
    curr_command = strtok_r(pipeline_string, "|", &command_tokens_buffer);
    if (curr_command == NULL) {
      break;
    }

    ++(chain->chain_links_count);
    char** curr_argument_copy = &chain_link->command;
    chain_link->argc = 0;
    pipe_string = curr_command;

    while (true) {
      // to get the next token strtok_r first argument must be NULL
      curr_argument = strtok_r(pipe_string, " ", &arg_tokens_buffer);
      if (curr_argument == NULL) {
        break;
      }

      ++(chain_link->argc);

      *curr_argument_copy = strdup(curr_argument);

      // argv[0] is command itself
      if (curr_argument_copy == &chain_link->command) {
        chain_link->argv[0] = strdup(chain_link->command);
        curr_argument_copy = chain_link->argv + 1;
      } else {
        ++curr_argument_copy;
      }

      pipe_string = NULL;
    }

    // The array of pointers must be terminated by a null pointer.
    if (chain_link->argc > 0) {
      chain_link->argv[chain_link->argc] = NULL;
    }

    ++chain_link;
    pipeline_string = NULL;
  }
}

void run_chain(chain_t* chain) {
  // TODO: Run pipeline described in chain object
  // [0] for reading, [1] for writing
  int prev_pipe[2] = {-1, -1};
  int curr_pipe[2] = {-1, -1};
  char output_buffer[OUTPUT_BUFFER_MAX_SIZE];
  int bytes_read;

  for (uint64_t i = 0; i < chain->chain_links_count; ++i) {
    if (pipe(curr_pipe) == -1) {
      exit(EXIT_FAILURE);
    }

    // creation of a new child process
    if (fork() == 0) {
      if (prev_pipe[0] != -1) {
        close(prev_pipe[1]);
        dup2(prev_pipe[0], STDIN_FILENO);
      }

      close(curr_pipe[0]);
      dup2(curr_pipe[1], STDOUT_FILENO);

      // swap current process with the i'th in chain
      if (execvp(chain->chain_links[i].command, chain->chain_links[i].argv) ==
          -1) {
        exit(EXIT_FAILURE);
      }
    }

    if (prev_pipe[0] != -1) {
      close(prev_pipe[0]);
      close(prev_pipe[1]);
    }

    prev_pipe[0] = curr_pipe[0];
    prev_pipe[1] = curr_pipe[1];
  }

  // all data processed
  close(curr_pipe[1]);

  while ((bytes_read =
              read(curr_pipe[0], output_buffer, OUTPUT_BUFFER_MAX_SIZE)) > 0) {
    write(STDOUT_FILENO, output_buffer, bytes_read);
  }

  close(curr_pipe[0]);
}

int main(int argc, char* argv[]) {
  // TODO: Use implemented functions
  chain_t chain;

  if (argc != 2) {
    return EXIT_FAILURE;
  }

  create_chain(argv[1], &chain);

  run_chain(&chain);

  return EXIT_SUCCESS;
}
