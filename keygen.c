#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>


int main(int argc, char** argv) {

    //Validate command line input 
    if (argc != 2) {
        fprintf(stderr, "%s", "Keygen usage: keygen <length of key>\n");
        exit(1);
    }
    int keyLength = atoi(argv[1]);
    if (keyLength < 1) {
        fprintf(stderr, "%s", "Key length must be greater than 0.");
        exit(1);
    }
    //allocate memory
    char key[keyLength + 1];
    memset(key, 0, keyLength + 1);

   time_t t;
//A = 65
//Z = 90
//space = 32
   int upper_limit = 90;
   int lower_limit = 64; //If we generate random number 64, we will convert it to a space

   /* Intializes random number generator */
   srand((unsigned) time(&t));

   /* Print 5 random numbers from 0 to 49 */
   int randChar;
   int i;
   for ( i = 0; i < keyLength; i++ ) {
        randChar = ((rand()  % (upper_limit - lower_limit + 1) + lower_limit));
       if (randChar == 64) {
           key[i] = 32;
       }
       else {
           key[i] = randChar;
       }
   }

   strcat(key, "\n");

   printf("%s", key);
   return(0);
}