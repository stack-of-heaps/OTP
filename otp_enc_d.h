#ifndef otp_enc_d
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>

void sigchld_handler(int s);
int openSocket(char* port);
int newRecvSocket(int listenSocket);
void recvMsg(int socketFD, char* buffer);
//SHARED
int ASCIItoOrdinal(char letter);
int newConnection(char* port);

//OTP_ENC
int newSocket(char* port);

//OTP_ENC_D
int encodeSend(char* msg, char* cipher, int socket);
void verifyConnection(int socketFD);
int encode(int msg, int key);
void getFilenames(char* plaintext, char* cipher, int socketFD);

//OTP_DEC_D
void badFileresponse(int socketFD);
int decode(int msg, int key);
int modulo(int a, int b);

#endif