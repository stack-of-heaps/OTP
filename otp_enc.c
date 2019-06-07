#include "otp_enc_d.h"

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

int main(int argc, char** argv) {

    if (argc != 4) {
        fprintf(stderr, "%s", "OTP_ENC usage: <text> <cipher> <port>\n");
        exit(2);
    }
    else {

        //Check if message and cipher files specified in cmd line are of same size.
        //If not, quit.
       long msgSize = getFileSize(argv[1]);
       long cipherSize = getFileSize(argv[2]);

       if (msgSize != cipherSize) {
           fprintf(stderr, "%s", "MESSAGE and CIPHER are not the same size. Must be same size.\n");
           exit(2);
       }

       char recvBuffer[512];
       memset(recvBuffer, '\0', 512);
       char* port = argv[3];
       char* secretMessage = "twobits\n";

       int talkSocket = newConnection(port);

       int bytesSent = send(talkSocket, secretMessage, strlen(secretMessage), 0);

       if (bytesSent < 0) {
           perror("Send: ");
           exit(2);
       }

       printf("bytessent: %d\n", bytesSent);

       int bytesReceived = recv(talkSocket, recvBuffer, sizeof(recvBuffer), 0);

       if (bytesReceived < 0) {
           perror("Receive: ");
           exit(2);
       }
       
       printf("recvbuffer: %s\n", recvBuffer);

       if (strcmp(recvBuffer, "wecool") != 0) {
           exit(2);
       }
       else {

            char sendBuffer[512];
            memset(sendBuffer, '\0', 512);
            char recvBuffer[512];
            memset(recvBuffer, '\0', 512);
            char* plaintext = argv[1];
            char* cipher = argv[2];

            strcat(sendBuffer, plaintext);
            strcat(sendBuffer, ";");
            strcat(sendBuffer, cipher);
            strcat(sendBuffer, ";");

            bytesSent = 0;
            bytesSent = send(talkSocket, sendBuffer, strlen(sendBuffer), 0);

            if (bytesSent < 0) {
                perror("Send: ");
                exit(2);
            }
            close(talkSocket);
       }
    }


    return 0;
}