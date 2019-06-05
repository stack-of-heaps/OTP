#include "otp_enc_d.h";

 /*******************************************************************************
  
 * Courtesy of BEEJ'S GUIDE, https://beej.us/guide/bgnet/html/single/bgnet.html
 * 
 * This function is called after the fork() process in main() in order to kill off
 * zombie processes periodically.
 * 
 ********************************************************************************/

void sigchld_handler(int s)
{
    waitpid(-1, NULL, WNOHANG) > 0;
}

 /*******************************************************************************
  
 * Courtesy of BEEJ'S GUIDE, use getaddrinfo() to populate the structs above with
 * information necessary to create a connection.
 * https://beej.us/guide/bgnet/html/single/bgnet.html
 * 
 * Creates a new connection using getaddrinfo to populate an addrinfo struct.
 * Creates a socket file descriptor.
 * Binds the socket.
 * Returns the new socket.
 * 
 * Takes in a port (int)
  
 ********************************************************************************/

int openSocket(char* port) {
    
    //Our "main" server, which will perpetually listen for connections
    struct addrinfo server, *res;      
    int serverPort;

    memset(&server, 0, sizeof server);
    server.ai_family = AF_INET;          //Setup for IPv4
    server.ai_socktype = SOCK_STREAM; //TCP
    
	//'Bind' our server to the current FLIP server	
	server.ai_flags = AI_PASSIVE;
	char hostbuffer[256];
	gethostname(hostbuffer, sizeof(hostbuffer));

    getaddrinfo(hostbuffer, port, &server, &res);

    int newSocket;
    //Create our socket
    newSocket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

    if (newSocket == -1) {
        printf("Socket error");
        fflush(stdout);
        exit(1);
    }

    //Bind the socket
    if (bind(newSocket, res->ai_addr, res->ai_addrlen) != 0) {
        perror("Socket binding: ");
        exit(1);
    }

    //Listen for connections
    if (listen(newSocket, 5) != 0) {
        perror("Listen: ");
        exit(1);
    }

    printf("Listening on %s\n", port);
    fflush(stdout);

    return newSocket;
}   


 /*******************************************************************************
  
 * Courtesy of BEEJ'S GUIDE, use getaddrinfo() to populate the structs above with
 * information necessary to create a connection.
 * https://beej.us/guide/bgnet/html/single/bgnet.html
 * 
 * The function above creates a socket for LISTENING. This creates a socket to read or write from.
 *  
 ********************************************************************************/
int newRecvSocket(int listenSocket) {

    int socketIn;
    struct sockaddr_storage incoming_addr;
    socklen_t addr_size;
    addr_size = sizeof incoming_addr;
    
    int option = 1; 

    socketIn = accept(listenSocket, (struct sockaddr*)&incoming_addr, &addr_size);

    if (socketIn == -1) {
        printf("Response socket error\n ");
        exit(1);
    }

    return socketIn;
}


 /*******************************************************************************
 *
 * simple helper function to receive initial request from client.
 * Reads message into buffer from socket passed in to function
 *  
 ********************************************************************************/
void recvMsg(int socketFD, char* buffer) {

    memset(buffer, '\0', 300);
    int bytes = recv(socketFD, buffer, 299, 0);
    
    if (bytes < 0) {
        perror("Socket read: ");
    }

    return;
}

 /*******************************************************************************
 *
 * When the client requests the pwd contents, this writes them to a buffer.
 * The buffer is initialized and passed in outside the function. this function
 * will not prevent buffer overruns.
 *  
 ********************************************************************************/
void getDirContents(char* buffer) {
   //Open current directory
   DIR* dir = opendir(".");
   struct dirent* dirContents = readdir(dir);

   char* fileIterator;
   strcpy(buffer, "Available files: ");

    //Iterate through all entities in the directory, excluding . and ..
    while (dirContents != NULL) {
    fileIterator = dirContents -> d_name;

    if (strcmp(fileIterator, ".") != 0 && strcmp(fileIterator, "..") != 0) {
        strcat(buffer, fileIterator);
        strcat(buffer, ", ");
    }
    dirContents = readdir(dir);
    fileIterator = dirContents -> d_name;
   }

   //Remove comma at the end of string.
   int length = strlen(buffer);
   buffer[length - 2] = '.';
}
 
 /*******************************************************************************
 *
 * If the client sends a command unrecognized by the server, the server sends this response.
 * The response is sent to the socket passed in to the function.
 *  
 ********************************************************************************/

void badCmdResponse(int socketFD) {
    char* badCmd = "Invalid command. Please try again with -g or -l.\n";
    int bytesSent = send(socketFD, badCmd, strlen(badCmd), 0);

    if (bytesSent < 0) {
        printf("Error sending badCmd message\n");
    }
}

 /*******************************************************************************
 *
 * If the client requests a file which is not found in the pwd, the server sends this response.
 * The response is sent to the socket passed in to the function.
 *  
 ********************************************************************************/
void badFileResponse(int socketFD) {
    char* badCmd = "FILE NOT FOUND.\n";
    int bytesSent = send(socketFD, badCmd, strlen(badCmd), 0);
}

void sendResponse(int socketFD, char* msg) {

    int bytesSent = send(socketFD, msg, strlen(msg), 0);

    if (bytesSent < 0) {
        printf("Error sending badCmd message\n");
    }
}


 /*******************************************************************************
 *
 * When the client requests a file, this function confirms that the file exists.
 * If it does not, it sends an error FILE NOT FOUND.
 * If file is found, server sends the file to the socket passed in to the function.
 *  
 * //https://stackoverflow.com/questions/238603/how-can-i-get-a-files-size-in-c
 * //https://stackoverflow.com/questions/24259640/writing-a-full-buffer-using-write-system-call
 ********************************************************************************/
int getSendFile(char* fileName, int socket) {
    int foundFileFlag = 0;
   //Open current directory
   DIR* dir = opendir(".");
   struct dirent* dirContents = readdir(dir);

   char* fileIterator;
    //Iterate through all entities in the directory, excluding . and ..
    //If we find the file, set a flag and break.
    while (dirContents != NULL) {

        fileIterator = dirContents -> d_name;

        if (strcmp(fileIterator, fileName) == 0) {
            foundFileFlag = 1;
            break;
        }
        dirContents = readdir(dir);
    }

    long fSize = 0;
    if (foundFileFlag == 1) {
        //Get file size for when we write
        FILE* filePtr = fopen(fileName, "r");
        fseek(filePtr, 0L, SEEK_END);
        fSize = ftell(filePtr);
        fclose(filePtr);
    
        char buffer[4096];
        //Read contents of file into buffer
        int file = open(fileName, O_RDONLY);
        size_t bytesSent = 0;
        int bytesRead = 1;
        printf("Sending file...\n");
        while (bytesRead > 0) {
            bytesRead = read(file, buffer, fSize);
            bytesSent = write(socket, buffer, bytesRead);
            //fSize -= bytesSent;
            memset(buffer, 0, 4096);
        }
    printf("File transfer complete.\n");
    close(file);
    return 0;
    }
    else {
        printf("Unable to find file %s\n", fileName);
        badFileResponse(socket);
        return -5;
    }
}

int main(int argc, char** argv) {

    //Verify we have proper command line invocation
    if (argc != 2) {
        printf("Usage: ftserver.c <port>\n");
        exit(1);
    }

    char rcvBuffer[65536];        //For incoming data

    char* serverPort = argv[1];

    int listenSocket = openSocket(serverPort);

    while(1) {
        //We will receive data on this socket (courtesy of Beej's guide)
        int recvSocket;
        recvSocket = accept(listenSocket, NULL, NULL);
        if (recvSocket == -1) {
            printf("Error on main server port\n");
            continue;
        }

        pid_t childPID = fork();

        switch(childPID) {
            //case -1: ;
                //printf("Error forking.\n");
                //exit(1);
                //break;

            case 0: ;
                //Receive command string from client
                char buffer[1024];
                memset(buffer, 0, 1024);
                recvMsg(recvSocket, buffer);

                //Parse command string
                //If bad command, we return -1 from function and send "bad cmd" message
                //If "-l", we return 3 and send directory
                //If "-g", we return 6, get filename, and send file
                //In all cases we get a port for the response
                char* cmd = NULL;
                char* port = NULL;
                char* fileName = NULL;

                cmd = strtok(buffer, ";");
                
                if (cmd[1] == 'l') {
                    printf("Client requests server send directory contents.\n");
                    port = strtok(NULL, ";");
                    //In this case, token2 is port
                    //Establish our response connection using port sent by client
                    int secondConnection;
                    //secondConnection = openSocket(port);
                    listenSocket = openSocket(port);

                    struct sockaddr_storage inc_addr;
                    socklen_t addr_sz;
                    addr_sz = sizeof inc_addr;
                    int sendSocket;

                    //sendSocket = accept(secondConnection, (struct sockaddr*)&inc_addr, &addr_sz);
                    sendSocket = accept(listenSocket, (struct sockaddr*)&inc_addr, &addr_sz);
                    memset(buffer, 0, 1024);
                    getDirContents(buffer);
                    sendResponse(sendSocket, buffer);
                    close(sendSocket);
                    printf("Closing data connection\n");
                }
                
                //File request
                else if (cmd[1] == 'g') {
                    printf("Client requests server send file.\n");
                    //In this case, token2 is filename, token3 is port
                    fileName = strtok(NULL, ";");
                    port = strtok(NULL, ";");
                    int secondConnection;
                    secondConnection = openSocket(port);

                    struct sockaddr_storage inc_addr;
                    socklen_t addr_sz;
                    addr_sz = sizeof inc_addr;
                    int sendSocket;
                    sendSocket = accept(secondConnection, (struct sockaddr*)&inc_addr, &addr_sz);

                    getSendFile(fileName, sendSocket); 
                    printf("Closing data connection\n");
                    close(sendSocket);
                    close(secondConnection);
                    }

                //Bad command
                else {
                    printf("Received bad command. Sending response\n");
                    
                    port = strtok(NULL, ";");
                    int secondConnection;
                    secondConnection = openSocket(port);

                    struct sockaddr_storage inc_addr;
                    socklen_t addr_sz;
                    addr_sz = sizeof inc_addr;
                    int sendSocket;

                    sendSocket = accept(secondConnection, (struct sockaddr*)&inc_addr, &addr_sz);
                    badCmdResponse(sendSocket);
                    printf("Closing data connection\n");
                }
                
                default: ;
                struct sigaction sa;
                sa.sa_handler = sigchld_handler; // reap all dead processes
                sigemptyset(&sa.sa_mask);
                sa.sa_flags = SA_RESTART;
                if (sigaction(SIGCHLD, &sa, NULL) == -1) {
                    perror("sigaction");
                    exit(1);
                }
            }
        }

    return 0;
}
