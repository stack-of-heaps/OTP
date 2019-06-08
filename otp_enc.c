//#include "otp_enc_d.h"
#include "magicsauce.h"
/*
NAME: KYLE KARTHAUSER
DATE: 6/7/19
COURSE: CS344-400
DESCRIPTION:  OTP_ENC.c facilitates communication between OTP_ENC_D.c. It's jobs are:
1) Communicate with OTP_ENC_D server. 
2) Verify that files passed in to command line are of the same size.
3) Verify connection with OTP_ENC.
4) Transmit filenames of MESSAGE and KEY to OTP_ENC_D.
5) Receive final encrypted message, outputting to stdout.
*/

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

       int bytesReceived = recv(talkSocket, recvBuffer, sizeof(recvBuffer), 0);
       if (bytesReceived < 0) {
           perror("Receive: ");
           exit(2);
       }

       if (strcmp(recvBuffer, "wecool\n") != 0) {
           exit(2);
       }
        //If we are verified by otp_enc_d, send filenames 
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

            recvMessage(talkSocket);

            close(talkSocket);
       }
    }


    return 0;
}