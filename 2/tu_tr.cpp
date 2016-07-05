/*
  Custom version of standard 'tr' command
  Author: Rains Jordan
  Version: 1.0
  
  Only supports basic 'tr' features. Two arguments are accepted as input. Each
  character from the first argument will be overwritten by the character at
  the corresponding position in the second argument. ASCII only.
  
  If strlen(arg1) > strlen(arg2), the last character of arg2 is used for all
  unmatched chars from arg1.
  If strlen(arg1) < strlen(arg2), the unmatched chars from arg2 are ignored.
*/

#include <iostream>
#include <cstring>
#include <cstdio>
#include <algorithm>

using std::cerr;
using std::endl;
using std::fill_n;

const int ARGS_USED_NUM = 2;
const int ASCII_MAX = 256;
const char* PROG_NAME = "tu_tr";
const char NO_MATCH = -1;

int main(int argc, char** argv) {
    if (argc != ARGS_USED_NUM + 1) {
        cerr << "Usage: ./tu_tr ARG1 ARG2" << endl
             << "(Each instance of a character from ARG1 "
             << "will be overwritten by the character at the corresponding "
             << "position in ARG2.)" << endl;
        return 1;
    }
    
    char* arg1 = argv[1];
    char* arg2 = argv[2];
    
    //Create a lookup table in which the Xth element will represent the
    //character that the character X should be transformed into.
    //Note that this table will include standard ASCII key values from
    //[0, 256], as well as -1 values, indicating that no valid mapping exists
    //for the input character.
    //If using -1 is not desired, the program design could easily be altered
    //to, for example, use a separate map of bools. Looking a character up in
    //this map would indicate whether it has a mapping or not.
    char map[ASCII_MAX];
    fill_n(map, sizeof(map) / sizeof(char), NO_MATCH);
    
    int len1 = strlen(arg1);
    int arg2MaxValidPos = strlen(arg2) - 1;
    
    //Note: If we run out of arg1 characters before we run out of arg2
    //characters, we're done setting up the transformation mapping. If we
    //run out of arg2 characters before we run out of arg1 characters, we'll
    //keep mapping the last arg2 character to all remaining arg1 characters.
    for (int pos1 = 0, pos2 = 0; pos1 < len1; ++pos1) {
        map[(int)arg1[pos1]] = arg2[pos2];
        
        if (pos2 < arg2MaxValidPos)
            ++pos2;
    }
    
    char ch = getchar();
    
    while (ch != EOF) {
        if (ch >= ASCII_MAX) {
            cerr << "Error: Unexpected input." << endl;
            return 1;
        }
        
        if (map[(int)ch] != NO_MATCH)
            putchar(map[(int)ch]);
        else
            putchar(ch);
        
        ch = getchar();
    }
    
    if (ferror(stdin)) {
        perror("Read error");
    }
    
    return 0;
}
