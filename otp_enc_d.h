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
int newRecvSocket(int listenSocket)m
void recvMsg(int socketFD, char* buffer);
int parseMsg(char* bufferIn, char* fileName, char* port);
void getDirContents(char* buffer);
int getSendFile(char* fileName, int socket);
void badCmdresponse(int socketFD);
void badFileresponse(int socketFD);
int ASCIItoOrdinal(char letter);
int encode(int msg, int key);
int decode(int msg, int key);
int modulo(int a, int b);

#endif