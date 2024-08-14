#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>`

#define DEBUG 1

#define SMAIN_SERVER_IP "127.0.0.1"

#define SMAIN_SERVER_PORT 4020
#define STEXT_SERVER_PORT 4014
#define SPDF_SERVER_PORT 4014

#define BUFFER_SIZE 1024

/**
 * @brief Communicates with the server using the specified socket.
 *
 * @param socket The socket to communicate with the server.
 */
void communicate_with_server(int socket);

/**
 * @brief Processes the command by sending it to the server and receiving the response.
 *
 * @param socket The socket to communicate with the server.
 * @param cmd_str The command string to process.
 * @param response The response received from the server.
 */
void process_command(int socket, char *cmd_str, char *response);

/**
 * @brief Sends a file to the server.
 *
 * @param server_socket The socket to communicate with the server.
 * @param file_path The path of the file to send.
 * @param destination_path The destination path on the server.
 * @return int Returns 0 if the file was sent successfully, -1 otherwise.
 */
int send_file(int server_socket, const char *file_path, const char *destination_path);

/**
 * @brief Downloads a file from the server.
 *
 * @param server_socket The socket to communicate with the server.
 * @param file_path The path of the file to download.
 * @return int Returns 0 if the file was downloaded successfully, -1 otherwise.
 */
int download_file(int server_socket, const char *file_path);

/**
 * @brief Removes a file from the server.
 *
 * @param server_socket The socket to communicate with the server.
 * @param file_path The path of the file to remove.
 * @return int Returns 0 if the file was removed successfully, -1 otherwise.
 */
int remove_file(int server_socket, const char *file_path);

/**
 * @brief Displays the files in the specified directory on the server.
 *
 * @param server_socket The socket to communicate with the server.
 * @param file_path The path of the directory to display files from.
 * @return int Returns 0 if the files were displayed successfully, -1 otherwise.
 */
int display_files(int server_socket, const char *file_path);

/**
 * @brief Tokenizes the command string into individual commands.
 *
 * @param cmd_str The command string to tokenize.
 * @param commands An array to store the individual commands.
 * @return int Returns the number of commands tokenized.
 */
int tokenize_command(char *cmd_str, char *commands[]);

/**
 * @brief Sends a message to the server and waits for an acknowledgement.
 *
 * @param socket The socket to communicate with the server.
 * @param message The message to send.
 * @param ack_message The acknowledgement message to wait for.
 * @return int Returns 0 if the message was sent and acknowledged successfully, -1 otherwise.
 */
int send_with_acknowledgement(int socket, const char *message, char *ack_message);

/**
 * @brief Receives a message from the server and sends an acknowledgement.
 *
 * @param socket The socket to communicate with the server.
 * @param ack_message The acknowledgement message to send.
 * @param message The received message from the server.
 * @return int Returns 0 if the message was received and acknowledged successfully, -1 otherwise.
 */
int receive_with_acknowledgement(int socket, const char *ack_message, char *message);

int main()
{
  int client_socket;
  struct sockaddr_in server_addr;
  char buffer[BUFFER_SIZE];
  int bytes_read;

  // Create socket
  client_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (client_socket < 0)
  {
    perror("Socket creation failed");
    exit(EXIT_FAILURE);
  }

  // Connect to Smain server
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(SMAIN_SERVER_PORT);
  server_addr.sin_addr.s_addr = inet_addr(SMAIN_SERVER_IP);
  if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
  {
    perror("Connection failed");
    close(client_socket);
    exit(EXIT_FAILURE);
  }

  // Communicate with Smain server
  communicate_with_server(client_socket);

  close(client_socket);
  return 0;
}

void communicate_with_server(int socket)
{
  char command[BUFFER_SIZE];
  char response[BUFFER_SIZE];

  while (1)
  {
    // Get command from user
    printf("Enter command: ");
    fgets(command, BUFFER_SIZE, stdin);
    command[strcspn(command, "\n")] = 0; // Remove newline
    if (strcmp(command, "exit") == 0)
      break;

    if (strcmp(command, "") == 0)
      continue;

    // Process the command
    process_command(socket, command, response);

    if (strcmp(response, "") == 0)
      continue;
    // Print the server response
    printf("%s\n", response);
  }
}

void process_command(int socket, char *cmd_str, char *response)
{
  char *commands[3];
  int count = tokenize_command(cmd_str, commands);

  if (count == 0)
  {
    strcpy(response, NULL);
    return;
  }

  char *command = commands[0];

  if (strcmp(command, "ufile") == 0)
  {
    // Upload file
    // Example: ufile filename destination_path

    if (count != 3)
    {
      strcpy(response, "Invalid Usage \n Usage: ufile filename destination_path");
      return;
    }

    char *filename = commands[1];
    char *destination_path = commands[2];

    // exract file extension
    char *file_extension = strrchr(filename, '.');
    if (file_extension == NULL)
    {
      strcpy(response, "Invalid file extension");
      return;
    }

    // if file extension is not .txt or .c or .pdf print error message
    if (strcmp(file_extension, ".txt") != 0 && strcmp(file_extension, ".c") != 0 && strcmp(file_extension, ".pdf") != 0)
    {
      strcpy(response, "Invalid file extension\nSupported file extensions: .txt, .c, .pdf");
      return;
    }

    // Send the file to the server
    if (send_file(socket, filename, destination_path) <= 0)
    {
      printf("Failed to send file\n");
      return;
    }

    // Receive the server response
    if (recv(socket, response, BUFFER_SIZE, 0) < 0)
    {
      printf("Failed to receive server response\n");
      return;
    }
  }
  else if (strcmp(command, "dfile") == 0)
  {
    if (count != 2)
    {
      strcpy(response, "Invalid Usage \n Usage: dfile filename");
      return;
    }

    char *filename = commands[1];

    // add null terminator to file path
    char *file_path_with_null = malloc(strlen(filename) + 1);
    strcpy(file_path_with_null, filename);
    file_path_with_null[strlen(filename)] = '\0';

    // send the command and file path to server separated by a space
    char message[BUFFER_SIZE];
    sprintf(message, "dfile %s", filename);
    char ack_message[BUFFER_SIZE];
    if (send_with_acknowledgement(socket, message, ack_message) <= 0)
    {
      return -1;
    }

    // Send the file to the server
    download_file(socket, filename);

    // Receive the server response
    if (recv(socket, response, BUFFER_SIZE, 0) < 0)
    {
      return;
    }
  }
  else if (strcmp(command, "rmfile") == 0)
  {
    if (count != 2)
    {
      strcpy(response, "Invalid Usage \n Usage: rmfile filename");
      return;
    }

    char *filename = commands[1];

    // exract file extension
    char *file_extension = strrchr(filename, '.');
    if (file_extension == NULL)
    {
      strcpy(response, "Invalid file path");
      return;
    }

    // remove file from the server
    remove_file(socket, filename);

    // Receive the server response
    if (recv(socket, response, BUFFER_SIZE, 0) < 0)
    {
      return;
    }
  }
  else if (strcmp(command, "dtar") == 0)
  {
    if (count != 2)
    {
      strcpy(response, "Invalid Usage \n Usage: dtar filetype");
      return;
    }

    char *filetype = commands[1];

    // add null terminator to filetype
    char *file_type = malloc(strlen(filetype) + 1);
    strcpy(file_type, filetype);
    file_type[strlen(filetype)] = '\0';

    // send the command and file path to server separated by a space
    char message[BUFFER_SIZE];
    sprintf(message, "dtar %s", file_type);
    char ack_message[BUFFER_SIZE];
    if (send_with_acknowledgement(socket, message, ack_message) <= 0)
    {
      return -1;
    }

    // create tar file name
    char tar_file_name[BUFFER_SIZE];
    sprintf(tar_file_name, "./%s.tar", file_type);

    // download tar file from the server
    download_file(socket, tar_file_name);

    // Receive the server response
    if (recv(socket, response, BUFFER_SIZE, 0) < 0)
    {
      return;
    }
  }
  else if (strcmp(command, "display") == 0)
  {
    if (count != 2)
    {
      strcpy(response, "Invalid Usage \n Usage: display path");
      return;
    }

    char *path = commands[1];

    // Send the file to the server
    display_files(socket, path);

    // Receive the server response
    if (recv(socket, response, BUFFER_SIZE, 0) < 0)
    {
      return;
    }
  }
  else
  {
    strcpy(response, "Invalid command");
  }
}

int send_file(int server_socket, const char *file_path, const char *destination_path)
{
  FILE *file = fopen(file_path, "rb");
  if (file == NULL)
  {
    perror("Failed to open file");
    return -1;
  }

  char buffer[BUFFER_SIZE];
  size_t bytes_read;

  // get file name from file path
  char *file_name = strrchr(file_path, '/');
  if (file_name == NULL)
    file_name = (char *)file_path;
  else
    file_name++;

  printf("File name: %s\n", file_name);

  fseek(file, 0, SEEK_END);
  size_t file_size = ftell(file);
  fseek(file, 0, SEEK_SET);

  // send the command, fileName, fileSize and destination path to server separated by a space
  char message[BUFFER_SIZE];
  sprintf(message, "ufile %s %ld %s", file_name, file_size, destination_path);

  char ack_message[BUFFER_SIZE];
  if (send_with_acknowledgement(server_socket, message, ack_message) <= 0)
  {
    fclose(file);
    return -1;
  }

  // Read file and send its contents
  while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, file)) > 0)
  {
    // send without acknowledgement file content to server
    if (send(server_socket, buffer, bytes_read, 0) < 0)
    {
      perror("Failed to send file");
      fclose(file);
      return -1;
    }

    // Calculate and print the percentage of file sent
    double percentage_sent = (double)ftell(file) / file_size * 100;
    printf("\rPercentage of file sent: %.2f%%", percentage_sent);
    fflush(stdout);

    // Add a delay
    usleep(1000); // Delay for 1 milliseconds
  }

  // receive acknowledgement from server
  char response[BUFFER_SIZE];
  if (recv(server_socket, response, BUFFER_SIZE, 0) < 0)
  {
    perror("Failed to receive acknowledgement");
    fclose(file);
    return -1;
  }

  printf("\n");

  fclose(file);
  return 1;
}

int download_file(int server_socket, const char *file_path)
{
  // Receive the file size from the server
  char response[BUFFER_SIZE];
  if (receive_with_acknowledgement(server_socket, "ack", response) == NULL)
  {
    perror("Failed to receive file size");
    return -1;
  }
  size_t file_size = atoi(response);
  if (DEBUG)
    printf("File size: %ld\n", file_size);

  if (file_size == -1)
  {
    printf("File not found\n");
    return -1;
  }

  // exract file name from file path
  char *file_name = strrchr(file_path, '/');
  if (file_name == NULL)
    file_name = (char *)file_path;
  else
    file_name++;

  printf("File name: %s\n", file_name);

  // Receive the file content from the server
  FILE *file = fopen(file_name, "wb");
  if (file == NULL)
  {
    perror("Failed to open file");
    return -1;
  }

  size_t total_bytes_received = 0;

  while (total_bytes_received < file_size)
  {
    char response[BUFFER_SIZE];
    // receive without acknowledgement file content from server
    int bytes_read = recv(server_socket, response, BUFFER_SIZE, 0);
    if (bytes_read < 0)
    {
      perror("Failed to receive file");
      fclose(file);
      return -1;
    }

    total_bytes_received += bytes_read;

    if (fwrite(response, 1, bytes_read, file) < 0)
    {
      perror("Failed to write to file");
      fclose(file);
      return -1;
    }

    // Calculate and print the percentage of file received
    double percentage_received = (double)total_bytes_received / file_size * 100;
    printf("\rPercentage of file received: %.2f%%", percentage_received);
    fflush(stdout);

    usleep(1000); // Delay for 1 milliseconds
  }

  if (DEBUG)
    printf("\nFile received, sending acknowledgement\n");

  // send acknowledgement to server
  if (send(server_socket, "ack", 3, 0) < 0)
  {
    perror("Failed to send acknowledgement");
    fclose(file);
    return -1;
  }

  printf("\n");

  fclose(file);
  return 0;
}

int remove_file(int server_socket, const char *file_path)
{
  // send the command and file path to server separated by a space
  char message[BUFFER_SIZE];
  sprintf(message, "rmfile %s", file_path);
  char ack_message[BUFFER_SIZE];
  if (send_with_acknowledgement(server_socket, message, ack_message) <= 0)
  {
    return -1;
  }

  // receive acknowledgement from server
  char response[BUFFER_SIZE];
  if (recv(server_socket, response, BUFFER_SIZE, 0) < 0)
  {
    perror("Failed to receive acknowledgement");
    return -1;
  }

  return 1;
}

int display_files(int server_socket, const char *file_path)
{
  // add null terminator to file path
  char *file_path_with_null = malloc(strlen(file_path) + 1);
  strcpy(file_path_with_null, file_path);
  file_path_with_null[strlen(file_path)] = '\0';

  // send the command and file path to server separated by a space
  char message[BUFFER_SIZE];
  sprintf(message, "display %s", file_path);
  char ack_message[BUFFER_SIZE];
  if (send_with_acknowledgement(server_socket, message, ack_message) <= 0)
  {
    return -1;
  }

  // Receive the file size from the server
  char response[BUFFER_SIZE];
  if (receive_with_acknowledgement(server_socket, "ack", response) == NULL)
  {
    perror("Failed to receive file size");
    return -1;
  }
  size_t file_size = atoi(response);
  printf("File size: %ld\n", file_size);

  if (file_size == 0)
  {
    printf("Directory does not exist\n");
    return -1;
  }

  char *file_name = "display.txt";

  printf("File name: %s\n", file_name);

  // Receive the file content from the server
  FILE *file = fopen(file_name, "wb");
  if (file == NULL)
  {
    perror("Failed to open file");
    return -1;
  }

  size_t total_bytes_received = 0;

  while (total_bytes_received < file_size)
  {
    char response[BUFFER_SIZE];
    // receive without acknowledgement file content from server
    int bytes_read = recv(server_socket, response, BUFFER_SIZE, 0);
    if (bytes_read < 0)
    {
      perror("Failed to receive file");
      fclose(file);
      return -1;
    }

    total_bytes_received += bytes_read;

    if (fwrite(response, 1, bytes_read, file) < 0)
    {
      perror("Failed to write to file");
      fclose(file);
      return -1;
    }

    // Calculate and print the percentage of file received
    double percentage_received = (double)total_bytes_received / file_size * 100;
    printf("\rPercentage of file received: %.2f%%", percentage_received);
    fflush(stdout);

    usleep(1000); // Delay for 1 milliseconds
  }

  if (DEBUG)
    printf("\nFile received, sending acknowledgement\n");

  // send acknowledgement to server
  if (send(server_socket, "ack", 3, 0) < 0)
  {
    perror("Failed to send acknowledgement");
    fclose(file);
    return -1;
  }

  printf("\n");

  fclose(file);
  return 0;
}

/* UTILITY FUNCTIONS */

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

  // Receive the acknowledgement message from the socket
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
  // Attempt to receive a message from the socket
  int bytes_received = recv(socket, message, BUFFER_SIZE, 0);

  // Check if the receive operation failed
  if (bytes_received < 0)
  {
    // Print an error message to stderr
    perror("Failed to receive message");
    // Return 0 to indicate failure
    return 0;
  }

  // Null-terminate the received message
  message[bytes_received] = '\0';

  // Attempt to send an acknowledgement message back through the socket
  if (send(socket, ack_message, strlen(ack_message), 0) < 0)
  {
    // Print an error message to stderr
    perror("Failed to send acknowledgement");
    // Return 0 to indicate failure
    return 0;
  }

  // Return the number of bytes successfully received
  return bytes_received;
}