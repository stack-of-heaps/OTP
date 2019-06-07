#include "otp_enc_d.h"

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
	server.ai_flags = AI_PASSIVE;
    getaddrinfo("localhost", port, &server, &res);

    //Create our socket
    int newSocket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

    if (newSocket == -1) {
        fprintf(stderr, "%s", "OTP_ENC_D: Socket Error\n");
        exit(1);
    }

    //Bind the socket
    if (bind(newSocket, res->ai_addr, res->ai_addrlen) != 0) {
        fprintf(stderr, "%s", "OTP_ENC_D: Socket Binding\n");
        exit(1);
    }




    //Listen for connections
    if (listen(newSocket, 5) != 0) {
        perror("Listen: ");
        exit(1);
    }

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
        perror("Response socket error\n ");
        return 0;
    }

    return socketIn;
}

 /*******************************************************************************
 *
 * Upon accepting a connection, our server needs to verify that whoever is contacting
 * us should be allowed to. It analyzes the incoming message, which should include a port.
 * If it rejects the message, it will respond with "bye". If it accepts, it will say "wecool,"
 * and return the new "response socket" which we will use to send the encoded message back.
 *  
 ********************************************************************************/
void verifyConnection(int socketFD) {
    char* code = "twobits\n";
    char* okay = "wecool\n";
    char* bye = "bye\n";
    char knockknock[512];
    memset(knockknock, '\0', 512);

    int bytesReceived = recv(socketFD, knockknock, 512, 0);

    if (bytesReceived <= 0) {
        fprintf(stderr, "%s", "OTP_ENC_D VERIFYCONNECTION: No message received. Exiting.\n");
        return;
    }

    char* handshake;
    handshake = strtok(knockknock, ";");
    printf("knockknock: %s\n", knockknock);
    printf("handshake: %s\n", handshake);

    //If we don't receive the message "twobits", we don't accept the connection
    if (strcmp(handshake, code) != 0) {
        int bytesSent = send(socketFD, bye, sizeof(bye), 0);

        if (bytesSent < 0) {
            fprintf(stderr, "%s", "OTP_ENC_D: Unable to respond on port.\n");
            return;
        }
        close(socketFD);
        return;
    }
    else {
        int bytesSent = send(socketFD, okay, sizeof(okay), 0);
        if (bytesSent < 0) {
            fprintf(stderr, "%s", "OTP_ENC_D: Unable to respond. Exiting.\n");
            return;
        }
    }
}

//Helper function: After otp_enc connection verified, we receive
//the filenames here. Parse them into variables. Then copy that
//into the plaintext and cipher parameters passed into function.
void getFilenames(char* plaintext, char* cipher, int socketFD) {
    char buffer[512];
    memset(buffer, '\0', 512);
    char* tempText;
    char* tempCipher;

    int bytesReceived = recv(socketFD, buffer, 512, 0);
    printf("getfilenames received: %s\n", buffer);

    if (bytesReceived <= 0) {
        fprintf(stderr, "%s", "OTP_ENC_D GETFILENAMES: No message received. Exiting.\n");
        return;
    }

    tempText = strtok(buffer, ";");
    tempCipher = strtok(NULL, ";");

    strcpy(plaintext, tempText);
    strcpy(cipher, tempCipher);

}

int ASCIItoOrdinal(char letter) {

    switch (letter) {
        case 'A':
            return 1;
        case 'B':
            return 2;
        case 'C':
            return 3;
        case 'D':
            return 4;
        case 'E':
            return 5;
        case 'F': 
            return 6;
        case 'G':
            return 7;
        case 'H':
            return 8;
        case 'I':
            return 9;
        case 'J':
            return 10;
        case 'K':
            return 11;
        case 'L':
            return 12;
        case 'M':
            return 13;
        case 'N':
            return 14;
        case 'O':
            return 15;
        case 'P':
            return 16;
        case 'Q':
            return 17;
        case 'R':
            return 18;
        case 'S':
            return 19;
        case 'T':
            return 20;
        case 'U':
            return 21;
        case 'V':
            return 22;
        case 'W':
            return 23;
        case 'X':
            return 24;
        case 'Y':
            return 25;
        case 'Z':
            return 26;
        case ' ':
            return 27;
    }
}

int encode(int msg, int key) {

    int convert = 0;
    int msgVal = ASCIItoOrdinal(msg);
    int keyVal = ASCIItoOrdinal(key);

    convert = (msgVal + keyVal) % 27;
    convert += 64; 

    if (convert == 27) {
        return 32;
    }
    else {
        return convert;
    }
}

//Helper function to determine size of file.
//Used with OTP_ENC to determine if MESSAGE and CIPHER files are
//of the same length. If not, they are invalid and the program quits.
//Function returns filesize in LONG form
long getFileSize(char* file) {

    long fSize;
    FILE* filePtr = fopen(file, "r");
    fseek(filePtr, 0L, SEEK_END);
    fSize = ftell(filePtr);
    fclose(filePtr);
    return fSize;

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
int encodeSend(char* plaintext, char* cipher, int socket) {

    //Get filesize
    long fSize = getFileSize(plaintext);

    FILE* text = fopen(plaintext, "r");
    FILE* code = fopen(cipher, "r");
    char t[1];          //Holds char from text
    char c[1];          //Holds char from code
    char buffer[512];   //Holds current block of encrypted text
    memset(t, 0, 1);
    memset(c, 0, 1);
    memset(buffer, '\0', 512);
    int bytesRead = 0;
    int totalBytesSent = 0;
    int flagEOF = 0;    //Flip this when we hit EOF

    while (flagEOF != 1) { //Outer loop in charge of sending ENTIRE converted file
        while (bytesRead < 512) {
            if ((t[0] = fgetc(text)) == EOF) {
                flagEOF = 1;
                break; //Inner loop: convert 512 chars at time, then send
            }
            else {
                c[0] = fgetc(code);
                if (t[0] == '\n') {
                    buffer[bytesRead] = '\n';
                    continue;
                }
                else {
                buffer[bytesRead] = encode(t[0], c[0]);
                bytesRead++;
                printf("send buffer: %s\n", buffer);
                }
            }
        }

        //When we've finished with our conversion or we've hit 512 chars, 
        //send the buffer
        size_t bytesSent = 0;
        int bytesRemaining = bytesRead;
        printf("Sending encrypted text...\n");
        while (bytesSent < bytesRead) {
            bytesSent += send(socket, buffer, bytesRemaining, 0);
            if (bytesSent < 0) {
                perror("Error sending converted file.");
                fclose(text);
                fclose(code);
                return 0;
            }
            bytesRemaining -= bytesSent;
        }
        totalBytesSent += bytesSent;
        memset(buffer, '0', 512);
    }
    //Done sending; close filestreams
    printf("Transmission complete. Sent %d bytes.\n", totalBytesSent);
    fclose(text);
    fclose(code);
}

int main(int argc, char** argv) {

    //Verify we have proper command line invocation
    if (argc != 2) {
        printf("Usage: otp_enc_d <port>\n");
        exit(1);
    }

    //Establish the server. Listen on port provided by user.
    char* serverPort = argv[1];
    int listenSocket = openSocket(serverPort);

    while(1) {
        //We will accept connections in the parent process before passing off to the child.
        int recvSocket = accept(listenSocket, NULL, NULL);
        if (recvSocket == -1) {
            fprintf(stderr, "%s", "Otp_Enc_D Accept: \n");
            continue;
        }

        pid_t childPID = fork();

        switch(childPID) {
            case -1: ;
                perror("Error forking.\n");
                exit(1);
                break;

            case 0: ;
                //Verify that incoming connection is valid (otp_enc only)
                verifyConnection(recvSocket);

                printf("Connection verified...\n");

                char plaintext[128];    //For filename
                char cipher[128];       //For filename
                memset(plaintext, 0, 128);
                memset(cipher, 0, 128);

                printf("Get filenames...\n");
                getFilenames(plaintext, cipher, recvSocket);
                printf("Plaintext: %s\n", plaintext);
                printf("Cipher: %s\n", cipher);
                printf("Encode and send...\n");
                encodeSend(plaintext, cipher, recvSocket);
                break;
                
            default: ;
                close(recvSocket);
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
