# Project Overview
This project consists of a client-server architecture with multiple server components handling different types of files and commands. The main components of the project are:

- **Smain Server**: The main server that coordinates with other servers and handles client requests.
- **Spdf Server**: A server dedicated to handling PDF files.
- **Stext Server**: A server dedicated to handling text files.
- **Client**: The client application that communicates with the Smain server to perform various file operations.

## File Descriptions

### Smain.c
This file contains the implementation of the main server (**Smain Server**). It includes functions to process client commands, handle file uploads and downloads, remove files, display files, and create tar archives. Key functions include:

- `process_command(int socket, char *command_str)`: Processes commands received from the client.
- `process_ufile(int socket, char *commands[])`: Handles file uploads from the client.
- `process_dfile(int socket, char *commands[])`: Handles file downloads to the client.
- `process_rmfile(int socket, char *commands[])`: Removes files on the server.
- `process_display(int socket, char *commands[])`: Displays files in a directory.
- `process_dtar(int socket, char *commands[])`: Creates and sends tar archives.
- `send_file(int socket, const char *file_path)`: Sends a file to the client.
- `receive_file(int client_socket, const char *dir_path, const char *file_name, int file_size)`: Receives a file from the client.

### Spdf.c
This file contains the implementation of the PDF server (**Spdf Server**). It includes functions similar to those in Smain.c but specifically for handling PDF files. Key functions include:

- `prcclient(int client_socket)`: Handles communication with a client.
- `process_command(int socket, char *command_str)`: Processes commands received from the client.
- `process_ufile(int socket, char *commands[])`: Handles file uploads from the client.
- `process_dfile(int socket, char *commands[])`: Handles file downloads to the client.
- `process_rmfile(int socket, char *commands[])`: Removes files on the server.
- `process_display(int socket, char *commands[])`: Displays files in a directory.
- `process_dtar(int socket, char *commands[])`: Creates and sends tar archives.
- `send_file(int socket, const char *file_path)`: Sends a file to the client.
- `receive_file(int client_socket, const char *dir_path, const char *file_name, int file_size)`: Receives a file from the client.

### Stext.c
This file contains the implementation of the text server (**Stext Server**). It includes functions similar to those in Smain.c but specifically for handling text files. Key functions include:

- `prcclient(int client_socket)`: Handles communication with a client.
- `process_command(int socket, char *command_str)`: Processes commands received from the client.
- `process_ufile(int socket, char *commands[])`: Handles file uploads from the client.
- `process_dfile(int socket, char *commands[])`: Handles file downloads to the client.
- `process_rmfile(int socket, char *commands[])`: Removes files on the server.
- `process_display(int socket, char *commands[])`: Displays files in a directory.
- `process_dtar(int socket, char *commands[])`: Creates and sends tar archives.
- `send_file(int socket, const char *file_path)`: Sends a file to the client.
- `receive_file(int client_socket, const char *dir_path, const char *file_name, int file_size)`: Receives a file from the client.

### client24s.c
This file contains the implementation of the client application. It includes functions to communicate with the Smain server, send commands, and handle file operations. Key functions include:

- `communicate_with_server(int socket)`: Manages communication with the server.
- `process_command(int socket, char *cmd_str, char *response)`: Processes commands by sending them to the server and receiving responses.
- `send_file(int server_socket, const char *file_path, const char *destination_path)`: Sends a file to the server.
- `download_file(int server_socket, const char *file_path)`: Downloads a file from the server.
- `remove_file(int server_socket, const char *file_path)`: Removes a file on the server.
- `display_files(int server_socket, const char *file_path)`: Displays files in a directory on the server.

## Compilation and Execution

### Compiling the Servers
To compile the servers, use the following commands:
```bash
gcc -o smain Smain.c
gcc -o spdf Spdf.c
gcc -o stext Stext.c
```

### Compiling the Client
To compile the client, use the following command:
```bash
gcc -o client24s client24s.c
```

### Running the Servers
To run the servers, use the following commands:
```bash
./spdf
./stext
./smain
```

### Running the Client
To run the client, use the following command:
```bash
./client24s
```

## Usage
1. Start the servers (`smain`, `spdf`, and `stext`).
2. Run the client (`client24s`).
3. Use the client to send commands to the smain server, which will coordinate with the spdf and stext servers as needed.

## Contributing
Contributions are welcome! Please open an issue or submit a pull request for any improvements or bug fixes.

## Authors
 - Fwaad Ahmad
