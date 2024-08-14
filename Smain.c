#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/stat.h>
#include <errno.h>
#include <dirent.h>

#define DEBUG 1

#define SMAIN_SERVER_IP "127.0.0.1"

#define SMAIN_SERVER_PORT 4020
#define STEXT_SERVER_PORT 4014
#define SPDF_SERVER_PORT 4014

#define BUFFER_SIZE 1024

/**
 * @brief Process the client's commands.
 *
 * @param socket The client socket.
 * @param command_str The command string received from the client.
 */
void process_command(int socket, char *command_str);

/**
 * @brief Process the "ufile" command.
 *
 * @param socket The client socket.
 * @param commands The array of command arguments.
 * @return int Returns 1 if the file was successfully received, -1 otherwise.
 */
int process_ufile(int socket, char *commands[]);

/**
 * @brief Process the "dfile" command.
 *
 * @param socket The client socket.
 * @param commands The array of command arguments.
 * @return int Returns 1 if the file was successfully downloaded, -1 otherwise.
 */
int process_dfile(int socket, char *commands[]);

/**
 * @brief Process the "rmfile" command.
 *
 * @param socket The client socket.
 * @param commands The array of command arguments.
 * @return int Returns 1 if the file was successfully removed, -1 otherwise.
 */
int process_rmfile(int socket, char *commands[]);

/**
 * @brief Process the "display" command.
 *
 * @param socket The client socket.
 * @param commands The array of command arguments.
 * @return int Returns 1 if the files were successfully displayed, -1 otherwise.
 */
int process_display(int socket, char *commands[]);

/**
 * @brief Process the "dtar" command.
 *
 * @param socket The client socket.
 * @param commands The array of command arguments.
 * @return int Returns 1 if the tar file was successfully downloaded, -1 otherwise.
 */
int process_dtar(int socket, char *commands[]);

/**
 * @brief Send a file to the client.
 *
 * @param socket The client socket.
 * @param file_path The path of the file to send.
 * @return int Returns 1 if the file was successfully sent, -1 otherwise.
 */
int send_file(int socket, const char *file_path);

/**
 * @brief Send a file to the server.
 *
 * @param socket_to_server The server socket.
 * @param file_name The name of the file to send.
 * @param file_size The size of the file to send.
 * @param destination_path The destination path of the file on the server.
 * @return int Returns 1 if the file was successfully sent, -1 otherwise.
 */
int send_file_to_server(int socket_to_server, const char *file_name, int file_size, const char *destination_path);

/**
 * @brief Receive a file from the client.
 *
 * @param client_socket The client socket.
 * @param dir_path The directory path to save the file.
 * @param file_name The name of the file to receive.
 * @param file_size The size of the file to receive.
 * @return int Returns 1 if the file was successfully received, -1 otherwise.
 */
int receive_file(int client_socket, const char *dir_path, const char *file_name, int file_size);

/**
 * @brief Receive a file from the server.
 *
 * @param socket_to_server The server socket.
 * @param file_path The path to save the received file.
 * @return int Returns 1 if the file was successfully received, -1 otherwise.
 */
int receive_file_from_server(int socket_to_server, const char *file_path);

/**
 * @brief Remove a file from the client.
 *
 * @param socket The client socket.
 * @param file_path The path of the file to remove.
 * @return int Returns 1 if the file was successfully removed, -1 otherwise.
 */
int remove_file(int socket, const char *file_path);

/**
 * @brief Remove a file from the server.
 *
 * @param socket_to_server The server socket.
 * @param file_name The name of the file to remove.
 * @return int Returns 1 if the file was successfully removed, -1 otherwise.
 */
int remove_file_from_server(int socket_to_server, const char *file_name);

/**
 * @brief Display the files in a directory on the client.
 *
 * @param socket The client socket.
 * @param dir_path The directory path to display.
 * @return int Returns 1 if the files were successfully displayed, -1 otherwise.
 */
int display_files(int socket, const char *dir_path);

/**
 * @brief Display the files in a directory on the server.
 *
 * @param socket_to_server The server socket.
 * @param dir_path The directory path to display.
 * @param txt_file_paths The file paths of the text files.
 * @return int Returns 1 if the files were successfully displayed, -1 otherwise.
 */
int display_files_from_server(int socket_to_server, const char *dir_path, char *txt_file_paths);

/**
 * @brief Send a tar file to the client.
 *
 * @param socket The client socket.
 * @param tar_name The name of the tar file to send.
 * @return int Returns 1 if the tar file was successfully sent, -1 otherwise.
 */
int send_tar(int socket, const char *tar_name);

/**
 * @brief Create a tar file.
 *
 * @param tar_name The name of the tar file to create.
 * @param source_path The source path of the files to include in the tar file.
 * @return int Returns 1 if the tar file was successfully created, -1 otherwise.
 */
int create_tar(const char *tar_name, const char *source_path);

/**
 * @brief Create directories if they do not exist.
 *
 * @param path The path of the directories to create.
 * @return int Returns 0 if the directories were successfully created, -1 otherwise.
 */
int create_directories(const char *path);

/**
 * @brief Traverse a directory and collect file paths.
 *
 * @param root The root directory.
 * @param dir_path The current directory path.
 * @param file_paths The collected file paths.
 */
void traverse_directory(const char *root, const char *dir_path, char *file_paths);

/**
 * @brief Tokenize a command string.
 *
 * @param cmd_str The command string to tokenize.
 * @param commands The array to store the command arguments.
 * @return int Returns the number of command arguments.
 */
int tokenize_command(char *cmd_str, char *commands[]);

/**
 * @brief Send a message to the client with acknowledgement.
 *
 * @param socket The client socket.
 * @param message The message to send.
 * @param ack_message The acknowledgement message to receive.
 * @return int Returns the number of bytes sent if the message was successfully sent, -1 otherwise.
 */
int send_with_acknowledgement(int socket, const char *message, char *ack_message);

/**
 * @brief Receive a message from the client with acknowledgement.
 *
 * @param socket The client socket.
 * @param ack_message The acknowledgement message to send.
 * @param message The received message.
 * @return int Returns the number of bytes received if the message was successfully received, -1 otherwise.
 */
int receive_with_acknowledgement(int socket, const char *ack_message, char *message);

int server_socket; // Global variable for the server socket

int stext_server_socket; // Global variable for the stext server socket

int spdf_server_socket; // Global variable for the spdf server socket

/**
 * @brief Handles the SIGINT signal by closing the server sockets and exiting the program.
 *
 * This function is called when the SIGINT signal is received, typically when the user presses Ctrl+C.
 * It closes the server sockets and terminates the program gracefully.
 *
 * @param sig The signal number (SIGINT).
 */
void handle_sigint(int sig)
{
  printf("\nClosing socket...\n", sig);
  close(server_socket);
  close(stext_server_socket);
  close(spdf_server_socket);
  exit(0);
}

/**
 * @file Smain.c
 * @brief Main server program for Smain server.
 *
 * This program creates a server socket, binds it to a specific port, and listens for incoming connections.
 * When a client connects, a child process is forked to handle the client.
 * The server communicates with other servers (stext_server and spdf_server) using TCP sockets.
 * The server also creates directories if they do not exist.
 *
 * @note This program assumes the existence of the following constants:
 * - `SMAIN_SERVER_IP`: IP address of the Smain server
 * - `SMAIN_SERVER_PORT`: Port number for the Smain server
 * - `STEXT_SERVER_PORT`: Port number for the Stext server
 * - `SPDF_SERVER_PORT`: Port number for the Spdf server
 * - `DEBUG`: Flag indicating whether to print debug messages
 *
 * @return 0 on success, -1 on failure
 */
int main()
{
  int client_socket;
  struct sockaddr_in server_addr, client_addr;
  socklen_t addr_size;
  pid_t child_pid;
  int opt = 1;

  // Register signal handler for SIGINT
  signal(SIGINT, handle_sigint);

  // create ./smain, ./tar directories if they do not exist
  if (create_directories("./smain") != 0)
  {
    perror("Failed to create directories");
    exit(EXIT_FAILURE);
  }
  if (create_directories("./tar") != 0)
  {
    perror("Failed to create directories");
    exit(EXIT_FAILURE);
  }
  // Connect to the stext server
  connect_to_server(&stext_server_socket, SMAIN_SERVER_IP, STEXT_SERVER_PORT);

  // Connect to the spdf server
  connect_to_server(&spdf_server_socket, SMAIN_SERVER_IP, SPDF_SERVER_PORT);

  // Create socket
  server_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (server_socket < 0)
  {
    perror("Socket creation failed");
    exit(EXIT_FAILURE);
  }

  // Bind socket
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(SMAIN_SERVER_PORT);
  server_addr.sin_addr.s_addr = INADDR_ANY;
  if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
  {
    perror("Bind failed");
    close(server_socket);
    exit(EXIT_FAILURE);
  }

  // Listen for connections
  if (listen(server_socket, 10) < 0)
  {
    perror("Listen failed");
    close(server_socket);
    exit(EXIT_FAILURE);
  }

  printf("Smain server listening on port %d\n", SMAIN_SERVER_PORT);

  while (1)
  {
    addr_size = sizeof(client_addr);
    client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &addr_size);
    if (client_socket < 0)
    {
      perror("Accept failed");
      continue;
    }
    // Fork a child process to handle the client
    if (DEBUG)
      printf("Forking child process for new Client\n");
    child_pid = fork();
    if (child_pid == 0)
    {
      // Child process
      close(server_socket);
      prcclient(client_socket);
      exit(0);
    }
    else if (child_pid > 0)
    {
      // Parent process
      close(client_socket);
    }
    else
    {
      perror("Fork failed");
    }
  }

  close(server_socket);
  return 0;
}

void prcclient(int client_socket)
{
  // Declare a structure to hold client address information
  struct sockaddr_in client_addr;
  // Declare a variable to hold the size of the address structure
  socklen_t addr_size = sizeof(client_addr);

  // Get the address information of the connected client
  getpeername(client_socket, (struct sockaddr *)&client_addr, &addr_size);

  // Print the client's IP address and port number
  printf("Client connected: %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

  // Enter an infinite loop to continuously receive commands from the client
  while (1)
  {
    // Declare a buffer to hold the received command
    char command[BUFFER_SIZE];

    // Receive a command from the client with acknowledgement
    int bytes_received = receive_with_acknowledgement(client_socket, "ack", command);

    // Check if there was an error receiving the command
    if (bytes_received < 0)
    {
      // Print an error message and break the loop
      perror("Failed to receive command");
      break;
    }

    // Print the received command
    printf("Command received: %s\n", command);

    // Process the received command
    process_command(client_socket, command);
  }

  // Close the client socket after exiting the loop
  close(client_socket);

  // Print the client's IP address and port number indicating disconnection
  printf("Client disconnected: %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
}

void connect_to_server(int *client_socket, const char *server_ip, int server_port)
{
  struct sockaddr_in server_addr;

  // Create socket
  *client_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (*client_socket < 0)
  {
    perror("Socket creation failed");
    exit(EXIT_FAILURE);
  }

  // Connect to server
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(server_port);
  server_addr.sin_addr.s_addr = inet_addr(server_ip);
  if (connect(*client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
  {
    perror("Connection failed");
    close(*client_socket);
    exit(EXIT_FAILURE);
  }
}

void process_command(int socket, char *command_str)
{
  // Process the command
  char *commands[5];
  // Tokenize the command string
  int count = tokenize_command(command_str, commands);

  // extract command
  char *command = commands[0];

  if (strcmp(command, "ufile") == 0)
  {
    // Receive File from client
    if (process_ufile(socket, commands) == 1)
      send(socket, "File received by server", 24, 0);
    else
      send(socket, "Failed to receive file", 23, 0);
  }
  else if (strcmp(command, "dfile") == 0)
  {
    if (DEBUG)
      printf("Processing dfile command\n");
    // Send File to client
    if (process_dfile(socket, commands) == 1)
      send(socket, "File downloaded", 16, 0);
    else
      send(socket, "Failed to download file", 24, 0);
  }
  else if (strcmp(command, "rmfile") == 0)
  {
    if (DEBUG)
      printf("Processing rmfile command\n");
    // Remove file
    if (process_rmfile(socket, commands) == 1)
      send(socket, "File removed", 13, 0);
    else
      send(socket, "Failed to remove file", 22, 0);
  }
  else if (strcmp(command, "dtar") == 0)
  {
    if (DEBUG)
      printf("Processing dtar command\n");
    // Create tar file and send to client
    if (process_dtar(socket, commands) == 1)
      send(socket, "Tar file downloaded", 19, 0);
    else
      send(socket, "Failed to download tar file", 27, 0);
  }
  else if (strcmp(command, "display") == 0)
  {
    if (DEBUG)
      printf("Processing display command\n");
    // Display files
    if (process_display(socket, commands) == 1)
      send(socket, "File paths saved as file", 24, 0);
    else
      send(socket, "Failed to get files", 19, 0);
  }
  else
  {
    if (DEBUG)
      printf("Invalid command\n");
    send(socket, "Invalid command", 15, 0);
  }
}

int process_ufile(int socket, char *commands[])
{
  // Sample command: ufile fileName fileSize /destination/path
  // extract file name, file size and destination path
  char *filename = commands[1];
  int file_size = atoi(commands[2]);

  // extract file extension
  char *file_extension = strrchr(filename, '.');
  if (file_extension == NULL)
  {
    perror("Failed to extract file extension");
    return -1;
  }

  // create destination path by prepending ./smain/
  char destination_path[256];
  if (strcmp(file_extension, ".c") == 0)
    snprintf(destination_path, sizeof(destination_path), "./smain/%s", commands[3]);
  else
    snprintf(destination_path, sizeof(destination_path), "./smain/");

  if (DEBUG)
    printf("File name: %s, File size: %d, Destination path: %s\n", filename, file_size, destination_path);

  // receive file content in chunks
  receive_file(socket, destination_path, filename, file_size);

  // check if file extension is .txt or .pdf
  if (strcmp(file_extension, ".txt") == 0 || strcmp(file_extension, ".pdf") == 0)
  {
    int socket_to_server = (strcmp(file_extension, ".txt") == 0) ? stext_server_socket : spdf_server_socket;

    send_file_to_server(socket_to_server, filename, file_size, commands[3]);

    // get file full path by appending filename to destination path
    char file_full_path[256];
    snprintf(file_full_path, sizeof(file_full_path), "%s/%s", destination_path, filename);

    // delete file
    if (remove(file_full_path) != 0)
    {
      perror("Failed to remove file");
      return -1;
    }
  }

  printf("File received\n");
  return 1;
}

int process_dfile(int socket, char *commands[])
{
  // Sample command: ufile fileName fileSize /destination/path
  // extract file path
  char *file_path = commands[1];

  int file_fetch = 0;

  // extract file extension
  char *file_extension = strrchr(file_path, '.');
  if (file_extension == NULL)
  {
    perror("Failed to extract file extension");
    return -1;
  }

  // check if file extension is .txt
  if (strcmp(file_extension, ".txt") == 0 || strcmp(file_extension, ".pdf") == 0)
  {
    int socket_to_server = (strcmp(file_extension, ".txt") == 0) ? stext_server_socket : spdf_server_socket;
    file_fetch = 1;

    // receive file from server
    if (receive_file_from_server(socket_to_server, file_path) != 1)
    {
      perror("Failed to receive file from server");
      file_fetch = 0;
    }
  }

  if (DEBUG)
    printf("Sending file: %s\n", file_path);

  // create file path by prepending ./smain/
  char file_full_path[256];
  snprintf(file_full_path, sizeof(file_full_path), "./smain/%s", file_path);

  // receive file content in chunks
  int result = send_file(socket, file_full_path);

  if (file_fetch == 1)
  {
    // delete file
    if (remove(file_full_path) != 0)
    {
      perror("Failed to remove file");
      return -1;
    }
  }

  return result;
}

int process_rmfile(int socket, char *commands[])
{
  // Sample command: rmfile fileName
  // extract file name
  char *file_name = commands[1];

  // extract file extension
  char *file_extension = strrchr(file_name, '.');
  if (file_extension == NULL)
  {
    perror("Failed to extract file extension");
    return -1;
  }

  // check if file extension is .txt or .pdf
  if (strcmp(file_extension, ".txt") == 0 || strcmp(file_extension, ".pdf") == 0)
  {
    int socket_to_server = (strcmp(file_extension, ".txt") == 0) ? stext_server_socket : spdf_server_socket;
    int res = remove_file_from_server(socket_to_server, file_name);
    // send acknowledgement to client
    if (send(socket, "ack", 3, 0) < 0)
    {
      perror("Failed to send acknowledgement");
      return -1;
    }
    return res;
  }

  if (DEBUG)
    printf("Removing file: %s\n", file_name);

  // create file path by prepending ./smain/
  char file_full_path[256];
  snprintf(file_full_path, sizeof(file_full_path), "./smain/%s", file_name);

  // remove file
  return remove_file(socket, file_full_path);
}

int process_display(int socket, char *commands[])
{
  // Sample command: display /path/to/directory
  // extract directory path
  char *dir_path = commands[1];

  if (DEBUG)
    printf("Displaying files in directory: %s\n", dir_path);

  // display files
  return display_files(socket, dir_path);
}

int process_dtar(int socket, char *commands[])
{
  // Sample command: dtar /path/to/directory
  // extract directory path
  char *file_type = commands[1];

  if (DEBUG)
    printf("Creating tar file for filetype: %s\n", file_type);

  // check if file type is txt or pdf
  if (strcmp(file_type, "txt") == 0 || strcmp(file_type, "pdf") == 0)
  {
    int socket_to_server = (strcmp(file_type, "txt") == 0) ? stext_server_socket : spdf_server_socket;
    // send dtar string to stext server
    char ack_message[BUFFER_SIZE];
    if (send_with_acknowledgement(socket_to_server, "dtar", ack_message) <= 0)
    {
      perror("Failed to send dtar command to stext server");
      return -1;
    }

    // receive acknowledgement from stext server
    char response[BUFFER_SIZE];
    int bytes_received = recv(socket_to_server, response, BUFFER_SIZE, 0);
    if (bytes_received < 0)
    {
      perror("Failed to receive acknowledgement from stext server");
      return -1;
    }
  }
  else if (strcmp(file_type, "c") == 0)
  {
    // create tar file
    char tar_name[256];
    snprintf(tar_name, sizeof(tar_name), "./tar/%s.tar", file_type);

    if (create_tar(tar_name, "./smain") != 0)
    {
      perror("Failed to create tar file");
      return -1;
    }
  }
  sleep(1);

  // create file path
  char tar_path[256];
  snprintf(tar_path, sizeof(tar_path), "./tar/%s.tar", file_type);

  return send_tar(socket, tar_path);
}

int send_file(int socket, const char *file_path)
{
  // open file
  FILE *file = fopen(file_path, "rb");
  if (file == NULL)
  {
    // send with acknowledgement file size -1
    char ack_message[BUFFER_SIZE];
    if (send_with_acknowledgement(socket, "-1", ack_message) <= 0)
    {
      perror("Failed to send file size");
      return -1;
    }
    printf("File not found\n");
    return -1;
  }

  // get file size
  fseek(file, 0, SEEK_END);
  int file_size = ftell(file);
  fseek(file, 0, SEEK_SET);

  // send file size
  char ack_message[BUFFER_SIZE];
  char file_size_str[BUFFER_SIZE];
  sprintf(file_size_str, "%d", file_size);

  printf("Socket: %d\n", socket);
  if (send_with_acknowledgement(socket, file_size_str, ack_message) <= 0)
  {
    perror("Failed to send file size");
    fclose(file);
    return -1;
  }

  if (DEBUG)
    printf("File size: %d\n", file_size);

  // send file content
  char buffer[BUFFER_SIZE];
  int bytes_read;
  while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, file)) > 0)
  {
    if (send(socket, buffer, bytes_read, 0) < 0)
    {
      perror("Failed to send file");
      fclose(file);
      return -1;
    }
  }
  fclose(file);

  if (DEBUG)
    printf("File sent, waiting for acknowledgement\n");
  // receive acknowledgement
  char response[BUFFER_SIZE];
  int bytes_received = recv(socket, response, BUFFER_SIZE, 0);
  if (bytes_received < 0)
  {
    perror("Failed to receive acknowledgement");
    return -1;
  }
  return 1;
}

int send_file_to_server(int socket_to_server, const char *file_name, int file_size, const char *destination_path)
{
  // create command string
  char command_str[256];
  snprintf(command_str, sizeof(command_str), "ufile %s %d %s", file_name, file_size, destination_path);

  // send full command string to server
  char ack_message[BUFFER_SIZE];
  if (send_with_acknowledgement(socket_to_server, command_str, ack_message) <= 0)
  {
    perror("Failed to send command to stext server");
    return -1;
  }

  // create file path
  char file_path[256];
  snprintf(file_path, sizeof(file_path), "./smain/%s", file_name);

  // send file to server
  if (send_file(socket_to_server, file_path) != 1)
  {
    perror("Failed to send file to stext server");
    return -1;
  }

  // receive acknowledgement from server
  char response[BUFFER_SIZE];
  int bytes_received = recv(socket_to_server, response, BUFFER_SIZE, 0);
  if (bytes_received < 0)
  {
    perror("Failed to receive acknowledgement from stext server");
    return -1;
  }
  return 1;
}

int receive_file(int client_socket, const char *dir_path, const char *file_name, int file_size)
{
  // Create directories if they do not exist
  char *dir = strdup(dir_path);
  if (create_directories(dir) != 0)
  {
    perror("Failed to create directories");
    free(dir);
    return -1;
  }
  free(dir);

  // create file
  // create file path
  char file_path[256];
  snprintf(file_path, sizeof(file_path), "%s/%s", dir_path, file_name);

  FILE *file = fopen(file_path, "wb");
  if (file == NULL)
  {
    perror("Failed to create file");
    return -1;
  }

  int total_bytes_received = 0;

  while (total_bytes_received < file_size)
  {
    // receive without acknowledgement file content from client
    char response[BUFFER_SIZE];
    int bytes_received = recv(client_socket, response, BUFFER_SIZE, 0);
    if (bytes_received < 0)
    {
      perror("Failed to receive file");
      fclose(file);
      return -1;
    }

    total_bytes_received += bytes_received;

    if (fwrite(response, 1, bytes_received, file) < 0)
    {
      perror("Failed to write to file");
      fclose(file);
      return -1;
    }
  }

  // send acknowledgement
  if (send(client_socket, "ack", 3, 0) < 0)
  {
    perror("Failed to send acknowledgement");
    fclose(file);
    return -1;
  }

  fclose(file);
  return 0;
}

int receive_file_from_server(int socket_to_server, const char *file_path)
{
  // create command string
  char command_str[256];
  snprintf(command_str, sizeof(command_str), "dfile %s", file_path);

  // send full command string to stext server
  char ack_message[BUFFER_SIZE];
  if (send_with_acknowledgement(socket_to_server, command_str, ack_message) <= 0)
  {
    perror("Failed to send command to stext server");
    return -1;
  }

  // extract file name
  char *file_name = strrchr(file_path, '/');
  if (file_name != NULL)
  {
    file_name++; // Skip the '/'
  }
  else
  {
    file_name = file_path; // If no '/' found, use the whole path as file name
  }

  // receive with acknowledgement file size from stext server
  char file_size_str[BUFFER_SIZE];
  if (receive_with_acknowledgement(socket_to_server, "ack", file_size_str) <= 0)
  {
    perror("Failed to receive file size from stext server");
    return -1;
  }
  int file_size = atoi(file_size_str);

  if (DEBUG)
    printf("File size from server: %d\n", file_size);

  if (file_size == -1)
  {
    printf("File not found\n");
    return -1;
  }

  // receive file from stext server
  if (receive_file(socket_to_server, "./smain/", file_path, file_size) != 0)
  {
    perror("Failed to receive file from stext server");
    return -1;
  }

  // receive acknowledgement from stext server
  char response[BUFFER_SIZE];
  int bytes_received = recv(socket_to_server, response, BUFFER_SIZE, 0);
  if (bytes_received < 0)
  {
    perror("Failed to receive acknowledgement from stext server");
    return -1;
  }
  return 1;
}

int remove_file(int socket, const char *file_path)
{
  if (remove(file_path) != 0)
  {
    perror("Failed to remove file");
    return -1;
  }

  // send acknowledgement
  if (send(socket, "ack", 3, 0) < 0)
  {
    perror("Failed to send acknowledgement");
    return -1;
  }
  return 1;
}

int remove_file_from_server(int socket_to_server, const char *file_name)
{
  // create command string
  char command_str[256];
  snprintf(command_str, sizeof(command_str), "rmfile %s", file_name);

  // send full command string to stext server
  char ack_message[BUFFER_SIZE];
  if (send_with_acknowledgement(socket_to_server, command_str, ack_message) <= 0)
  {
    perror("Failed to send command to stext server");
    return -1;
  }

  // receive acknowledgement from stext server
  char response[BUFFER_SIZE];
  int bytes_received = recv(socket_to_server, response, BUFFER_SIZE, 0);
  if (bytes_received < 0)
  {
    perror("Failed to receive acknowledgement from stext server");
    return -1;
  }

  if (strcmp(response, "File removed") != 0)
  {
    printf("Failed to remove file\n");
    return -1;
  }
  return 1;
}

int display_files(int socket, const char *dir_path)
{
  // create dir path
  char dir_path_full[256];
  snprintf(dir_path_full, sizeof(dir_path_full), "./smain/%s", dir_path);

  // get all files and sub-files in the directory
  char file_paths[15360] = "";
  traverse_directory(dir_path_full, dir_path_full, file_paths);

  char pdf_file_paths[5120] = "";
  display_files_from_server(spdf_server_socket, dir_path, pdf_file_paths);

  // append pdf_file_paths to file_paths
  strcat(file_paths, pdf_file_paths);

  char txt_file_paths[5120] = "";
  display_files_from_server(stext_server_socket, dir_path, txt_file_paths);

  // append txt_file_paths to file_paths
  strcat(file_paths, txt_file_paths);

  // get file size
  int msg_size = strlen(file_paths);

  if (msg_size == 0)
  {
    // send with acknowledgement file size 0
    char ack_message[BUFFER_SIZE];
    if (send_with_acknowledgement(socket, "0", ack_message) <= 0)
    {
      perror("Failed to send file size");
    }
    return -1;
  }

  // send file size
  char ack_message[BUFFER_SIZE];
  char msg_size_str[BUFFER_SIZE];
  sprintf(msg_size_str, "%d", msg_size);
  if (send_with_acknowledgement(socket, msg_size_str, ack_message) <= 0)
  {
    perror("Failed to send file size");
    return -1;
  }

  // send content in chunks
  char buffer[BUFFER_SIZE];
  int bytes_sent = 0;
  while (bytes_sent < msg_size)
  {
    int bytes_to_send = msg_size - bytes_sent;
    if (bytes_to_send > BUFFER_SIZE)
    {
      bytes_to_send = BUFFER_SIZE;
    }
    strncpy(buffer, file_paths + bytes_sent, bytes_to_send);
    if (send(socket, buffer, bytes_to_send, 0) < 0)
    {
      perror("Failed to send file");
      return -1;
    }
    bytes_sent += bytes_to_send;
  }

  if (DEBUG)
    printf("File sent, waiting for acknowledgement\n");

  // receive acknowledgement
  char response[BUFFER_SIZE];
  int bytes_received = recv(socket, response, BUFFER_SIZE, 0);
  if (bytes_received < 0)
  {
    perror("Failed to receive acknowledgement");
    return -1;
  }
  return 1;
}

int display_files_from_server(int socket_to_server, const char *dir_path, char *txt_file_paths)
{
  // create command string
  char command_str[256];
  snprintf(command_str, sizeof(command_str), "display %s", dir_path);

  // send full command string to stext server
  char ack_message[BUFFER_SIZE];
  if (send_with_acknowledgement(socket_to_server, command_str, ack_message) <= 0)
  {
    perror("Failed to send command to stext server");
    return -1;
  }

  // receive with acknowledgement file size from stext server
  char file_size_str[BUFFER_SIZE];
  if (receive_with_acknowledgement(socket_to_server, "ack", file_size_str) <= 0)
  {
    perror("Failed to receive file size from stext server");
    return -1;
  }
  int file_size = atoi(file_size_str);

  // get content in chunks and save to txt_file_paths variable
  int total_bytes_received = 0;
  while (total_bytes_received < file_size)
  {
    // receive without acknowledgement file content from stext server
    char response[BUFFER_SIZE];
    int bytes_received = recv(socket_to_server, response, BUFFER_SIZE, 0);
    if (bytes_received < 0)
    {
      perror("Failed to receive file");
      return -1;
    }

    total_bytes_received += bytes_received;

    if (strncat(txt_file_paths, response, bytes_received) == NULL)
    {
      perror("Failed to write to file");
      return -1;
    }
  }

  // send acknowledgement to stext server
  if (send(socket_to_server, "ack", 3, 0) < 0)
  {
    perror("Failed to send acknowledgement");
    return -1;
  }

  // receive acknowledgement from stext server
  char response[BUFFER_SIZE];
  int bytes_received = recv(socket_to_server, response, BUFFER_SIZE, 0);
  if (bytes_received < 0)
  {
    perror("Failed to receive acknowledgement from stext server");
    return -1;
  }
  return 1;
}

int send_tar(int socket, const char *tar_name)
{
  if (DEBUG)
    printf("Sending tar file: %s\n", tar_name);
  // open file
  FILE *file = fopen(tar_name, "rb");
  if (file == NULL)
  {
    // send with acknowledgement file size -1
    char ack_message[BUFFER_SIZE];
    if (send_with_acknowledgement(socket, "-1", ack_message) <= 0)
    {
      perror("Failed to send file size");
      return -1;
    }
    printf("File not found\n");
    return -1;
  }

  // get file size
  fseek(file, 0, SEEK_END);
  int file_size = ftell(file);
  fseek(file, 0, SEEK_SET);

  // send file size
  char ack_message[BUFFER_SIZE];
  char file_size_str[BUFFER_SIZE];
  sprintf(file_size_str, "%d", file_size);
  if (send_with_acknowledgement(socket, file_size_str, ack_message) <= 0)
  {
    perror("Failed to send file size");
    fclose(file);
    return -1;
  }

  // send file content
  char buffer[BUFFER_SIZE];
  int bytes_read;
  while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, file)) > 0)
  {
    if (send(socket, buffer, bytes_read, 0) < 0)
    {
      perror("Failed to send file");
      fclose(file);
      return -1;
    }
  }
  fclose(file);

  if (DEBUG)
    printf("File sent, waiting for acknowledgement\n");
  // receive acknowledgement
  char response[BUFFER_SIZE];
  int bytes_received = recv(socket, response, BUFFER_SIZE, 0);
  if (bytes_received < 0)
  {
    perror("Failed to receive acknowledgement");
    return -1;
  }
  return 1;
}

/* UTILITY FUNCTIONS */

int create_tar(const char *tar_name, const char *source_path)
{
  pid_t pid = fork();

  if (pid < 0)
  {
    perror("Fork failed");
    return -1;
  }

  if (pid == 0)
  {
    // Child process
    char *argv[] = {"tar", "-czf", tar_name, source_path, NULL};
    execvp("tar", argv);
    // If execvp returns, it must have failed
    perror("execvp failed");
    return -1;
  }
  else
  {
    // Parent process
    int status;
    if (waitpid(pid, &status, 0) == -1)
    {
      perror("waitpid failed");
      return -1;
    }
    if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
    {
      return 0; // Success
    }
    else
    {
      fprintf(stderr, "Child process failed with status %d\n", WEXITSTATUS(status));
      return -1;
    }
  }
  return -1;
}

int create_directories(const char *path)
{
  // Create directories recursively
  char temp[256];
  char *p = NULL;
  size_t len;

  snprintf(temp, sizeof(temp), "%s", path);
  len = strlen(temp);
  if (temp[len - 1] == '/')
  {
    temp[len - 1] = 0;
  }
  for (p = temp + 1; *p; p++)
  {
    if (*p == '/')
    {
      *p = 0;
      // Create directory if it doesn't exist
      if (mkdir(temp, S_IRWXU) != 0 && errno != EEXIST)
      {
        return -1;
      }
      *p = '/';
    }
  }
  // Create the final directory
  if (mkdir(temp, S_IRWXU) != 0 && errno != EEXIST)
  {
    return -1;
  }
  return 0;
}

void traverse_directory(const char *root, const char *dir_path, char *file_paths)
{
  DIR *dir;             // Pointer to the directory stream
  struct dirent *entry; // Pointer to the directory entry

  // Open the directory specified by dir_path
  dir = opendir(dir_path);
  if (dir == NULL)
  {
    // If the directory cannot be opened, print an error message and return
    perror("Failed to open directory");
    return;
  }

  // Read each entry in the directory
  while ((entry = readdir(dir)) != NULL)
  {
    // Skip the current directory (".") and parent directory ("..") entries
    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
    {
      continue;
    }

    // Construct the full path of the current entry
    char path[256];
    snprintf(path, sizeof(path), "%s/%s", dir_path, entry->d_name);

    // If the entry is a directory, recursively traverse it
    if (entry->d_type == DT_DIR)
    {
      traverse_directory(root, path, file_paths);
    }
    // If the entry is a regular file
    else if (entry->d_type == DT_REG)
    {
      // Create a buffer to store the file path relative to the root directory
      char file_path[256];
      strncpy(file_path, path + strlen(root), sizeof(file_path));

      // Get the file name from the file path
      char *file_name = strrchr(file_path, '/');
      if (file_name != NULL)
      {
        file_name++; // Skip the '/' character
      }
      else
      {
        file_name = file_path; // If no '/' found, use the whole path as the file name
      }

      // Create a buffer to store the file entry (file name and file path)
      char file_entry[512];
      snprintf(file_entry, sizeof(file_entry), "%s - %s\n", file_name, file_path);

      // Append the file entry to the file_paths string
      strcat(file_paths, file_entry);
    }
  }

  // Close the directory stream
  closedir(dir);
}

int tokenize_command(char *cmd_str, char *commands[])
{
  int count = 0; // Initialize a counter to keep track of the number of tokens

  // Use strtok to split the input string cmd_str into tokens separated by spaces
  char *token = strtok(cmd_str, " ");

  // Loop through all tokens
  while (token != NULL)
  {
    // Store the current token in the commands array and increment the counter
    commands[count++] = token;

    // Get the next token
    token = strtok(NULL, " ");
  }

  // Return the total number of tokens found
  return count;
}

int send_with_acknowledgement(int socket, const char *message, char *ack_message)
{
  // Send the message through the socket
  if (send(socket, message, strlen(message), 0) < 0)
  {
    // Print an error message if sending fails
    perror("Failed to send message");
    return 0; // Return 0 to indicate failure
  }

  // Receive the acknowledgement message from the socket
  int bytes_received = recv(socket, ack_message, BUFFER_SIZE, 0);
  if (bytes_received < 0)
  {
    // Print an error message if receiving fails
    perror("Failed to receive acknowledgement");
    return 0; // Return 0 to indicate failure
  }

  // Null-terminate the received acknowledgement message
  ack_message[bytes_received] = '\0';

  // Return the length of the sent message to indicate success
  return strlen(message);
}

int receive_with_acknowledgement(int socket, const char *ack_message, char *message)
{
  // Seek to the beginning of the message in the socket
  lseek(socket, 0, SEEK_SET);

  // Receive the message from the socket
  int bytes_received = recv(socket, message, BUFFER_SIZE, 0);
  if (bytes_received < 0)
  {
    // Print an error message if receiving fails
    perror("Failed to receive message");
    return 0; // Return 0 to indicate failure
  }

  // Null-terminate the received message
  message[bytes_received] = '\0';

  // Send the acknowledgement message through the socket
  if (send(socket, ack_message, strlen(ack_message), 0) < 0)
  {
    // Print an error message if sending fails
    perror("Failed to send acknowledgement");
    return 0; // Return 0 to indicate failure
  }

  // Return the number of bytes received to indicate success
  return bytes_received;
}