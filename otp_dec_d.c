#include "magicsauce.h"

/*
NAME: KYLE KARTHAUSER
DATE: 6/7/19
COURSE: CS344-400
DESCRIPTION:  OTP_ENC_D.c is the "decoding server" in this batch of files. It's job is to:
1) Run in the background, waiting for OTP_DEC.c to connect
2) Verify any attempted connections are legitimate. prevent connection if illegitimate.
3) Receive names of files containing ENCRYPTED MESSAGE and KEY.
4) Fetch files from current directory, decode MESSAGE using KEY.
5) Transmit the DECRYPTED MESSAGE back to OTP_DEC_C.c
*/

int main(int argc, char** argv) {

    //Verify we have proper command line invocation
    if (argc != 2) {
        printf("Usage: otp_dec_d <port>\n");
        exit(1);
    }

    //Establish the server. Listen on port provided by user.
    char* serverPort = argv[1];
    int listenSocket = openSocket(serverPort);

    while(1) {
        //We will accept connections in the parent process before passing off to the child.
        int recvSocket = accept(listenSocket, NULL, NULL);
        if (recvSocket == -1) {
            fprintf(stderr, "%s", "Otp_Dec_D Accept: \n");
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
                verifyConnectionDEC(recvSocket);

                char plaintext[128];    //For filename
                char cipher[128];       //For filename
                memset(plaintext, 0, 128);
                memset(cipher, 0, 128);

                getFilenames(plaintext, cipher, recvSocket);
                decodeSend(plaintext, cipher, recvSocket);
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