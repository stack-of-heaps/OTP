#include <stdio.h>
#include <stdlib.h>

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

int decode(int msg, int key) {

    int convert = 0;
    int msgVal = ASCIItoOrdinal(msg);
    int keyVal = ASCIItoOrdinal(key);

    convert = (msgVal - keyVal) % 27;

    if (convert == 0) {
        return 32;
    }
    else {
        convert += 64; 
        return convert;
    }
}

int main() {

    char one = 'K';
    char two = 'T';
    char a = ASCIItoOrdinal(one);
    char b = ASCIItoOrdinal(two);
    char garble = encode(one, two);
    printf("One: %c\n", ASCIItoOrdinal(one) + 64);
    printf("Two: %d\n", ASCIItoOrdinal(two));
    printf("Garble: %c\n", garble);
    printf("Garble: %d\n", garble);
    printf("a: %c\n", a);
    printf("b: %c\n", b);

    return 0;
}