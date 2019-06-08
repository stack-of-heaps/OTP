#include "magicsauce.h"
#define SIZE 27

            ////////////////////////////////////
            /*                                */
            /*          SERVER-SIDE           */
            /*                                */
            ////////////////////////////////////

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

 /*******************************************************************************
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

 /*******************************************************************************
 * Courtesy of BEEJ'S GUIDE, https://beej.us/guide/bgnet/html/single/bgnet.html
 * 
 * This function is called after the fork() process in main() in order to kill off
 * zombie processes periodically.
 ********************************************************************************/
void sigchld_handler(int s)
{
    waitpid(-1, NULL, WNOHANG) > 0;
}

            ////////////////////////////////////
            /*                                */
            /*          CLIENT-SIDE           */
            /*                                */
            ////////////////////////////////////

//Simple function to receive encrypted message in 512-byte chunks.
//Loops until we receive no more data. 
//Closes the socket, prints message to stdout;
void recvMessage(int socketFD) {
    printf("in recv message\n");
    char completeMessage[65536];
    char rcvBuffer[512];
    memset(completeMessage, '\0', 65536);
    memset(rcvBuffer, '\0', 512);
    int totalBytesReceived = 0;
    int bytesReceived = 1;

    while(1) {
        bytesReceived = recv(socketFD, rcvBuffer, 512, 0);
        if (bytesReceived <= 0) {
            break;
        }
        else {
            printf("recvbuffer: %s\n", rcvBuffer);
            memset(rcvBuffer, '\0', 512);
            totalBytesReceived += bytesReceived;
        }
    }

    printf("%s", completeMessage);
}
 
 /*******************************************************************************
 * Courtesy of BEEJ'S GUIDE
 * https://beej.us/guide/bgnet/html/single/bgnet.html
 * 
 * Unlike openConnection(), this function is there for the CLIENT, not the host. 
 * This creates a socket in order to CONNECT. Does not BIND OR LISTEN.
 * 
 * Takes in a port (char*). Returns the socket.
 ********************************************************************************/
int newConnection(char* port) {

    int newSocket;
    struct addrinfo server, *res;      
    memset(&server, 0, sizeof server);

    server.ai_family = AF_INET;          //Setup for IPv4
    server.ai_socktype = SOCK_STREAM; //TCP
	server.ai_flags = AI_PASSIVE;
    
    getaddrinfo("localhost", port, &server, &res);

    //Create our socket
    newSocket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

    if (newSocket == -1) {
        perror("Socket creation: ");
        exit(1);
    }

    if (connect(newSocket, res->ai_addr, res->ai_addrlen) < 0) {
        perror("Connect: ");
        exit(2);
    }

    return newSocket;
}
            
            //////////////////////////////////////////
            /*                                      */
            /*          ENCODING/DECODING           */
            /*                                      */
            //////////////////////////////////////////

//Trying to get the ASCII values of A-Z to play nicely with that of SPACE
//and have it all survive encoding/decoding was difficult. This helper
//function is used to translate A to int val of 1, Z to 26, and SPACE to 27.
//As a result, when we % 27 and get 0, that is a SPACE.
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

//This is the function which performs all the encoding from
//'plaintext' to encoded text. Uses ASCIItoOrdinal() to translate
//ascii values into our controlled environment of 1-27.
//Used to convert ONE CHAR at a time. Returns that char as its
//ascii value.
int encode(int msg, int key) {

    int convert = 0;
    int msgVal = ASCIItoOrdinal(msg);
    int keyVal = ASCIItoOrdinal(key);

    //If we have 0 after modulo, it's a space.
    convert = (msgVal + keyVal) % SIZE;
    convert += 64; 

    if (convert == 27) {
        return 32;
    }
    else {
        return convert;
    }
}

//The counterpart of encode. Does the exact same thing, using the same
//ASCII helper function. Instead of using the key file to "increase" the 
//CHAR value and therefore mask it, the key is used to "decrease" the char value
//and returns the "original" letter passed into the encode() function.
//Used to DECODE ONE CHAR AT A TIME.
int decode(int msg, int key) {

    int convert = 0;
    int offset;
    int msgVal = ASCIItoOrdinal(msg);
    int keyVal = ASCIItoOrdinal(key);

    offset = (msgVal - keyVal);
    convert = modulo(offset, SIZE);

    //If we have 0 after modulo, it's a space.
    if (convert == 0) {
        return 32;
    }
    else {
        convert += 64; 
        return convert;
    }
}

//Needed this in order to ACTUALLY do modulo calculation in C.
//C's % operator is actually remainder and does not working correctly
//for modulo with negative modulo.
//https://stackoverflow.com/questions/11720656/modulo-operation-with-negative-numbers/42131603
int modulo(int a, int b) {
    return (a % b + b) % b;
}