#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "otp_enc_d.h"

#define ROT_MIN 'A'
#define ROT_MAX 'Z'
#define ROT_SIZE (ROT_MAX - ROT_MIN + 1)
#define ROT_HALF (ROT_SIZE / 2)
#define MIN 64 
#define MAX 90
#define SIZE 27

/*

Conversion code inspired by:
https://chromium.googlesource.com/chromiumos/platform/flimflam/+/1eab803b27c6819d89a6f0f8db62d3d802daad1c/tools/rot47-test.c

*/

//Since we are annoyingly including spaces, I built this helper function to 
//accomodate that. 
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

    if (convert == 0) {
        return 32;
    }
    else {
        convert += 64; 
        return convert;
    }
}

int decode(int msg, int key) {

    int convert = 0;
    int offset;
    int msgVal = ASCIItoOrdinal(msg);
    int keyVal = ASCIItoOrdinal(key);

    offset = (msgVal - keyVal);
    convert = modulo(offset, SIZE);

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

int main(int argc, char** argv){

    FILE* msg = fopen(argv[1], "r");
    FILE* cipher = fopen(argv[2], "r");
    char msgLengthTest[1024];
    char cipherLengthTest[1024];
    memset(msgLengthTest, '\0', 1024);
    memset(cipherLengthTest, '\0', 1024);

    fgets(msgLengthTest, 1024, msg);
    fgets(cipherLengthTest, 1024, cipher);


    if (strlen(msgLengthTest) != strlen(cipherLengthTest)) {
        fprintf(stderr, "%s", "MESSAGE and CIPHER files are not the same length.\n");
        exit(1);
    }

    char f_msg[1];
    char f_cipher[1];
    char garble[1024];
    memset(garble, '\0', 1024);

    int m, c, g;               //m = message, c = cipher, g = garble
    int counter = 0;         //will increment each time we read a character

    //We want to stop reading at newline character (ASCII 10, as we know
    //all of our input files we terminate with that)
    while ( (m = fgetc(msg)) != 10) {
        f_msg[0] = m;
        c = fgetc(cipher);
        f_cipher[0] = c;
        g = encode(m, c);
        garble[counter] = g;
        counter++;
    printf("Message: %c\n", f_msg[0]);
    printf("Cipher: %c\n", f_cipher[0]);
    printf("Garble: %c\n", g);
    
    }

    fclose(msg);
    fclose(cipher);


    printf("Garbled: %s\n", garble);

    FILE* key = fopen(argv[2], "r");

    char s_msg[1];
    char s_cipher[1];
    char answer[1024];
    memset(answer, '\0', 1024);

    int m2, c2, g2;               //m = message, c = cipher, g = garble
    counter = 0;         //will increment each time we read a character

    //We want to stop reading at newline character (ASCII 10, as we know
    //all of our input files we terminate with that)
    while ( (m2 = fgetc(key)) != 10) {
        s_msg[0] = m2;
        c2 = garble[counter];
        g2 = decode(c2, m2);
        answer[counter] = g2;

    printf("Message: %c\n", garble[counter]);
    printf("Cipher: %c\n", m2);
    printf("Answer: %c\n", answer[counter]);
        counter++;
    }
    printf("Final Answer: %s\n", answer);

   return 0; 
}