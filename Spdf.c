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
 * @brief Function to handle the client process.
 *
 * This function is responsible for handling the communication with a client.
 * It receives commands from the client, processes them, and sends back the appropriate responses.
 *
 * @param client_socket The socket descriptor for the client connection.
 */
void prcclient(int client_socket);

/**
 * @brief Function to process a command received from the client.
 *
 * This function processes the command received from the client and performs the corresponding action.
 * The command is parsed and the appropriate function is called based on the command type.
 *
 * @param socket The socket descriptor for the client connection.
 * @param command_str The command string received from the client.
 */
void process_command(int socket, char *command_str);

/**
 * @brief Function to process the "ufile" command.
 *
 * This function handles the "ufile" command, which is used to receive a file from the client.
 * It extracts the file name, file size, and destination path from the command arguments.
 * The file content is then received in chunks and saved to the specified destination path.
 *
 * @param socket The socket descriptor for the client connection.
 * @param commands An array of command arguments.
 * @return Returns 1 if the file is successfully received, 0 otherwise.
 */
int process_ufile(int socket, char *commands[]);

/**
 * @brief Function to process the "dfile" command.
 *
 * This function handles the "dfile" command, which is used to send a file to the client.
 * It extracts the file path from the command arguments and sends the file content in chunks to the client.
 *
 * @param socket The socket descriptor for the client connection.
 * @param commands An array of command arguments.
 * @return Returns 1 if the file is successfully sent, 0 otherwise.
 */
int process_dfile(int socket, char *commands[]);

/**
 * @brief Function to process the "rmfile" command.
 *
 * This function handles the "rmfile" command, which is used to remove a file.
 * It extracts the file name from the command arguments and removes the file from the server.
 *
 * @param socket The socket descriptor for the client connection.
 * @param commands An array of command arguments.
 * @return Returns 1 if the file is successfully removed, 0 otherwise.
 */
int process_rmfile(int socket, char *commands[]);

/**
 * @brief Function to process the "display" command.
 *
 * This function handles the "display" command, which is used to display the files in a directory.
 * It extracts the directory path from the command arguments and sends the file paths to the client.
 *
 * @param socket The socket descriptor for the client connection.
 * @param commands An array of command arguments.
 * @return Returns 1 if the file paths are successfully sent, 0 otherwise.
 */
int process_display(int socket, char *commands[]);

/**
 * @brief Function to process the "dtar" command.
 *
 * This function handles the "dtar" command, which is used to create a tar file.
 * It extracts the directory path from the command arguments and creates a tar file for the specified directory.
 *
 * @param socket The socket descriptor for the client connection.
 * @param commands An array of command arguments.
 * @return Returns 1 if the tar file is successfully created, 0 otherwise.
 */
int process_dtar(int socket, char *commands[]);

/**
 * @brief Function to send a file to the client.
 *
 * This function sends a file to the client.
 * It opens the file, gets the file size, sends the file size to the client, and then sends the file content in chunks.
 *
 * @param socket The socket descriptor for the client connection.
 * @param file_path The path of the file to be sent.
 * @return Returns 1 if the file is successfully sent, -1 otherwise.
 */
int send_file(int socket, const char *file_path);

/**
 * @brief Function to receive a file from the client.
 *
 * This function receives a file from the client and saves it to the specified directory path.
 * It receives the file size from the client, receives the file content in chunks, and saves it to the specified path.
 *
 * @param client_socket The socket descriptor for the client connection.
 * @param dir_path The directory path where the file should be saved.
 * @param file_name The name of the file to be saved.
 * @param file_size The size of the file to be received.
 * @return Returns 1 if the file is successfully received, 0 otherwise.
 */
int receive_file(int client_socket, const char *dir_path, const char *file_name, int file_size);

/**
 * @brief Function to remove a file.
 *
 * This function removes a file from the server.
 *
 * @param socket The socket descriptor for the client connection.
 * @param file_path The path of the file to be removed.
 * @return Returns 1 if the file is successfully removed, 0 otherwise.
 */
int remove_file(int socket, const char *file_path);

/**
 * @brief Function to display the files in a directory.
 *
 * This function displays the files in a directory.
 * It sends the file paths to the client.
 *
 * @param socket The socket descriptor for the client connection.
 * @param dir_path The path of the directory to be displayed.
 * @return Returns 1 if the file paths are successfully sent, 0 otherwise.
 */
int display_files(int socket, const char *dir_path);

/**
 * @brief Function to send a tar file to the client.
 *
 * This function sends a tar file to the client.
 * It creates a tar file for the specified directory and sends it to the client.
 *
 * @param socket The socket descriptor for the client connection.
 * @param file_type The type of files to include in the tar file.
 * @return Returns 1 if the tar file is successfully created and sent, 0 otherwise.
 */
int send_tar(int socket, const char *file_type);

/**
 * @brief Function to create a tar file.
 *
 * This function creates a tar file for the specified directory.
 *
 * @param tar_name The name of the tar file to be created.
 * @param source_path The path of the directory to be included in the tar file.
 * @return Returns 0 if the tar file is successfully created, -1 otherwise.
 */
int create_tar(const char *tar_name, const char *source_path);

/**
 * @brief Function to create directories.
 *
 * This function creates directories if they do not exist.
 *
 * @param path The path of the directories to be created.
 * @return Returns 0 if the directories are successfully created, -1 otherwise.
 */
int create_directories(const char *path);

/**
 * @brief Function to traverse a directory and get the file paths.
 *
 * This function traverses a directory and gets the file paths.
 *
 * @param root The root directory.
 * @param dir_path The path of the directory to be traversed.
 * @param file_paths The buffer to store the file paths.
 */
void traverse_directory(const char *root, const char *dir_path, char *file_paths);

/**
 * @brief Function to tokenize a command string.
 *
 * This function tokenizes a command string into an array of command arguments.
 *
 * @param cmd_str The command string to be tokenized.
 * @param commands The array to store the command arguments.
 * @return Returns the number of command arguments.
 */
int tokenize_command(char *cmd_str, char *commands[]);

/**
 * @brief Function to send a message with acknowledgement.
 *
 * This function sends a message to the client and waits for an acknowledgement message.
 *
 * @param socket The socket descriptor for the client connection.
 * @param message The message to be sent.
 * @param ack_message The buffer to store the acknowledgement message.
 * @return Returns the number of bytes received if the acknowledgement is received, -1 otherwise.
 */
int send_with_acknowledgement(int socket, const char *message, char *ack_message);

/**
 * @brief Function to receive a message with acknowledgement.
 *
 * This function receives a message from the client and sends an acknowledgement message.
 *
 * @param socket The socket descriptor for the client connection.
 * @param ack_message The acknowledgement message to be sent.
 * @param message The buffer to store the received message.
 * @return Returns the number of bytes received if the message is received, -1 otherwise.
 */
int receive_with_acknowledgement(int socket, const char *ack_message, char *message);

/**
 * @brief Signal handler for SIGINT.
 *
 * This function is called when the SIGINT signal is received.
 * It closes the server socket and exits the program.
 *
 * @param sig The signal number.
 */
void handle_sigint(int sig);

int server_socket; // Global variable for the server socket

void handle_sigint(int sig)
{
  printf("\nClosing socket...\n", sig);
  close(server_socket);
  exit(0);
}

int main()
{
  int client_socket;
  struct sockaddr_in server_addr, client_addr;
  socklen_t addr_size;
  pid_t child_pid;
  int opt = 1;

  // Register signal handler for SIGINT
  signal(SIGINT, handle_sigint);

  // create ./spdf directories if they do not exist
  if (create_directories("./spdf") != 0)
  {
    perror("Failed to create directories");
    exit(EXIT_FAILURE);
  }

  // Create socket
  server_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (server_socket < 0)
  {
    perror("Socket creation failed");
    exit(EXIT_FAILURE);
  }

  // Bind socket
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(SPDF_SERVER_PORT);
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

  printf("spdf server listening on port %d\n", SPDF_SERVER_PORT);

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
  // print information about the client
  struct sockaddr_in client_addr;
  socklen_t addr_size = sizeof(client_addr);
  getpeername(client_socket, (struct sockaddr *)&client_addr, &addr_size);
  printf("Client connected: %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
  while (1)
  {
    // Receive command from client
    char command[BUFFER_SIZE];
    int bytes_received = receive_with_acknowledgement(client_socket, "ack", command);
    if (bytes_received < 0)
    {
      perror("Failed to receive command");
      break;
    }

    if (bytes_received == 0)
    {
      printf("Client disconnected: %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
      break;
    }
    // print the command
    printf("Command received: %s\n", command);
    process_command(client_socket, command);
  }

  // Close client socket
  close(client_socket);
  // print information about the client
  printf("Client disconnected: %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
}

void process_command(int socket, char *command_str)
{
  // Process the command
  char *commands[5];
  int count = tokenize_command(command_str, commands);

  char *command = commands[0];

  if (strcmp(command, "ufile") == 0)
  {
    // Receive File from client
    if (process_ufile(socket, commands) == 1)
      send(socket, "File received by server", 14, 0);
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
  // create destination path by prepending ./spdf/
  char destination_path[256];
  snprintf(destination_path, sizeof(destination_path), "./spdf/%s", commands[3]);

  if (DEBUG)
    printf("File name: %s, File size: %d, Destination path: %s\n", filename, file_size, destination_path);

  // receive file content in chunks
  receive_file(socket, destination_path, filename, file_size);

  printf("File received\n");
  return 1;
}

int process_dfile(int socket, char *commands[])
{
  // Sample command: ufile fileName fileSize /destination/path
  // extract file path
  char *file_path = commands[1];

  if (DEBUG)
    printf("Sending file: %s\n", file_path);

  // create file path by prepending ./spdf/
  char file_full_path[256];
  snprintf(file_full_path, sizeof(file_full_path), "./spdf/%s", file_path);

  // receive file content in chunks
  return send_file(socket, file_full_path);
}

int process_rmfile(int socket, char *commands[])
{
  // Sample command: rmfile fileName
  // extract file name
  char *file_name = commands[1];

  if (DEBUG)
    printf("Removing file: %s\n", file_name);

  // create file path by prepending ./spdf/
  char file_full_path[256];
  snprintf(file_full_path, sizeof(file_full_path), "./spdf/%s", file_name);

  // remove file
  return remove_file(socket, file_full_path);
}

int process_display(int socket, char *commands[])
{
  // Sample command: display /path/to/directory
  // extract directory path
  char *dir_path = commands[1];

  // create directory path by prepending ./spdf/
  char full_dir_path[256];
  snprintf(full_dir_path, sizeof(full_dir_path), "./spdf/%s", dir_path);

  if (DEBUG)
    printf("Displaying files in directory: %s\n", full_dir_path);

  // display files
  return display_files(socket, full_dir_path);
}

int process_dtar(int socket, char *commands[])
{
  // Sample command: dtar /path/to/directory
  // extract directory path

  if (DEBUG)
    printf("Creating tar file for filetype: pdf\n");

  // create tar file
  char tar_name[256];
  snprintf(tar_name, sizeof(tar_name), "./tar/pdf.tar");

  if (create_tar(tar_name, "./spdf") != 0)
  {
    perror("Failed to create tar file");
    return -1;
  }
  return 1;
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

  if (DEBUG)
    printf("Receiving file: %s\n", file_name);

  // receive with acknowledgement file size
  char file_size_str[BUFFER_SIZE];
  if (receive_with_acknowledgement(client_socket, "ack", file_size_str) <= 0)
  {
    perror("Failed to receive file size");
    return -1;
  }

  if (DEBUG)
    printf("File size: %s\n", file_size_str);

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

int remove_file(int socket, const char *file_path)
{
  if (remove(file_path) != 0)
  {
    perror("Failed to remove file");
    return -1;
  }
  return 1;
}

int display_files(int socket, const char *dir_path)
{
  // get all files and sub-files in the directory
  char file_paths[5120] = "";
  traverse_directory(dir_path, dir_path, file_paths);

  // send file sizes to the client
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

int send_tar(int socket, const char *file_type)
{
  // create tar file
  char tar_name[256];
  snprintf(tar_name, sizeof(tar_name), "./tar/%s.tar", file_type);

  if (create_tar(tar_name, "./smain") != 0)
  {
    perror("Failed to create tar file");
    return -1;
  }

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
  pid_t pid = fork(); // Create a new process by duplicating the calling process

  if (pid < 0)
  {
    // If fork() returns a negative value, the creation of a child process was unsuccessful
    perror("Fork failed"); // Print an error message to stderr
    return -1;             // Return an error code
  }

  if (pid == 0)
  {
    // This block is executed by the child process
    char *argv[] = {"tar", "-czf", tar_name, source_path, NULL}; // Arguments for the execvp function
    execvp("tar", argv);                                         // Replace the current process image with a new process image specified by argv
    // If execvp returns, it must have failed
    perror("execvp failed"); // Print an error message to stderr
    return -1;               // Return an error code
  }
  else
  {
    // This block is executed by the parent process
    int status; // Variable to store the status information of the child process
    if (waitpid(pid, &status, 0) == -1)
    {
      // waitpid() waits for the child process to change state
      perror("waitpid failed"); // Print an error message to stderr
      return -1;                // Return an error code
    }
    if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
    {
      // WIFEXITED(status) returns true if the child terminated normally
      // WEXITSTATUS(status) returns the exit status of the child
      return 0; // Success
    }
    else
    {
      // If the child process did not terminate normally or exited with a non-zero status
      fprintf(stderr, "Child process failed with status %d\n", WEXITSTATUS(status)); // Print an error message to stderr
      return -1;                                                                     // Return an error code
    }
  }
  return -1; // Return an error code if none of the above conditions are met
}

int create_directories(const char *path)
{
  char temp[256]; // Buffer to hold the modified path
  char *p = NULL; // Pointer to traverse the path
  size_t len;     // Variable to store the length of the path

  // Copy the input path to the temp buffer safely
  snprintf(temp, sizeof(temp), "%s", path);

  // Get the length of the path
  len = strlen(temp);

  // Remove trailing slash if it exists
  if (temp[len - 1] == '/')
  {
    temp[len - 1] = 0;
  }

  // Traverse the path and create directories as needed
  for (p = temp + 1; *p; p++)
  {
    if (*p == '/')
    {
      *p = 0; // Temporarily terminate the string at the current position

      // Create the directory if it doesn't exist
      if (mkdir(temp, S_IRWXU) != 0 && errno != EEXIST)
      {
        return -1; // Return error if directory creation fails
      }

      *p = '/'; // Restore the slash
    }
  }

  // Create the final directory if it doesn't exist
  if (mkdir(temp, S_IRWXU) != 0 && errno != EEXIST)
  {
    return -1; // Return error if directory creation fails
  }

  return 0; // Return success
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

  // Read entries from the directory
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
      // Create a buffer to hold the relative file path
      char file_path[256];
      // Copy the relative file path (excluding the root directory part) into file_path
      strncpy(file_path, path + strlen(root), sizeof(file_path));

      // Get the file name from the file path
      char *file_name = strrchr(file_path, '/');
      if (file_name != NULL)
      {
        file_name++; // Skip the '/'
      }
      else
      {
        file_name = file_path; // If no '/' found, use the whole path as file name
      }

      // Create a buffer to hold the file entry (file name and relative file path)
      char file_entry[512];
      // Format the file entry as "file_name - file_path\n"
      snprintf(file_entry, sizeof(file_entry), "%s - %s\n", file_name, file_path);
      // Append the file entry to file_paths
      strcat(file_paths, file_entry);
    }
  }

  // Close the directory stream
  closedir(dir);
}

int tokenize_command(char *cmd_str, char *commands[])
{
  int count = 0; // Initialize a counter to keep track of the number of tokens

  // Use strtok to split the input string 'cmd_str' into tokens separated by spaces
  char *token = strtok(cmd_str, " ");

  // Loop through all tokens
  while (token != NULL)
  {
    // Store the current token in the 'commands' array and increment the counter
    commands[count++] = token;

    // Get the next token
    token = strtok(NULL, " ");
  }

  // Return the total number of tokens found
  return count;
}

int send_with_acknowledgement(int socket, const char *message, char *ack_message)
{
  // Attempt to send the message through the socket
  if (send(socket, message, strlen(message), 0) < 0)
  {
    // If sending fails, print an error message and return 0
    perror("Failed to send message");
    return 0;
  }

  // Receive an acknowledgement message from the socket
  int bytes_received = recv(socket, ack_message, BUFFER_SIZE, 0);
  if (bytes_received < 0)
  {
    // If receiving fails, print an error message and return 0
    perror("Failed to receive acknowledgement");
    return 0;
  }

  // Null-terminate the received acknowledgement message
  ack_message[bytes_received] = '\0';

  // Return the length of the original message sent
  return strlen(message);
}

int receive_with_acknowledgement(int socket, const char *ack_message, char *message)
{
  // Seek to the beginning of the message in the socket
  lseek(socket, 0, SEEK_SET);

  // Receive the message from the socket
  int bytes_received = recv(socket, message, BUFFER_SIZE, 0);

  // Check if there was an error receiving the message
  if (bytes_received < 0)
  {
    // Print an error message if receiving failed
    perror("Failed to receive message");
    // Return 0 to indicate failure
    return 0;
  }

  // Null-terminate the received message
  message[bytes_received] = '\0';

  // Send an acknowledgement message back through the socket
  if (send(socket, ack_message, strlen(ack_message), 0) < 0)
  {
    // Print an error message if sending the acknowledgement failed
    perror("Failed to send acknowledgement");
    // Return 0 to indicate failure
    return 0;
  }

  // Return the number of bytes received
  return bytes_received;
}