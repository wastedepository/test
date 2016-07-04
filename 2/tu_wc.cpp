/*
  Custom version of standard 'wc' command
  Author: Rains Jordan
  
  Supported options:
  -l: print number of newline characters
  -w: print number of words
  -c: print number of bytes
  These options are also printed, in the above order, if no options are given.
  
  Any extra arguments will be treated as input filenames. The statistics from
  each file will be accumulated together. If no input files are given, stdin
  will be used.
  
  Notes:
  -Each word consists of non-whitespace characters, separated by whitespace
  -\r\n will be counted as a single newline character
  -Does not work on Windows, due to the use of stat() to determine byte counts
   for each file. These are used to compute the maximum possible width for each
   column in the output.
*/

#include <iostream>
#include <cstdio>
#include <getopt.h>

using std::cout;
using std::endl;

const char INVALID_CHAR = -1;

bool ProcessFile(FILE* file, bool bUseLines, bool bUseWords, bool bUseBytes,
                 int* pTotalLineNum, int* pTotalWordNum, int* pTotalByteNum);

int main(int argc, char** argv) {
	//Follow the traditional method of creating a struct of values for use in
	//getopt_long(). Each command-line parameter will have a short, one-char
	//version, and a longer, full-word version.
	static struct option longOptions[] = {
		{"lines", no_argument, 0, 'l'},
		{"words", no_argument, 0, 'w'},
		{"bytes", no_argument, 0, 'c'},
		{0,       0,           0, 0}
	};
    
    bool bUseLines = false, bUseWords = false, bUseBytes = false;
    
    int opt;
    
    //Get the values associated with each of several named command-line
	//parameters.
    while ((opt = getopt_long(argc, argv, "lwc", longOptions,
	                             NULL)) != -1) {
		switch (opt) {
			case 'l' :
				bUseLines = true;
				break;
			case 'w' :
				bUseWords = true;
				break;
			case 'c' :
				bUseBytes = true;
				break;
			default:
				break;
		}
	}
    
    int lineNum = 0, wordNum = 0, byteNum = 0;
    
    if (optind < argc) {
        //Process the non-option arguments (which have been automatically
        //rearranged by getopt_long() to follow the option-related arguments).
        for (int i = optind; i < argc; ++i) {
            FILE* file = fopen(argv[i], "r");
            
            //Question: Do you prefer 'nullptr' even in old-school C-style
            //situations like this and the other usage with getopt_long above?
            
            if (file == NULL) {
                cout << "No such file or directory: '" << argv[i] << "'"
                     << endl;
            }
            else {
                if (!ProcessFile(file, bUseLines, bUseWords, bUseBytes,
                                 &lineNum, &wordNum, &byteNum)) {
                    cout << "Unable to read file '" << argv[i] << "'" << endl;
                }
            }
        }
    }
    else {
        ProcessFile(stdin, bUseLines, bUseWords, bUseBytes,
                    &lineNum, &wordNum, &byteNum);
    }
    
    //TODO print totals
    
    return 0;
}

//Scan a text file and gather various statistics from the data in it.
//Returns false if failed to process file, true otherwise.
bool ProcessFile(FILE* file, bool bUseLines, bool bUseWords, bool bUseBytes,
                 int* pTotalLineNum, int* pTotalWordNum, int* pTotalByteNum) {
    int lineNum = 0, wordNum = 0, byteNum = 0;
    
    char prevCh = INVALID_CHAR;
    char ch = getc(file);
    
    //TODO count words
    
    while (ch != EOF) {
        cout << ">" << ch << "<" << endl;
        
        if (ch == '\r')
            ++lineNum;
        else if (ch == '\n') {
            if (prevCh != '\r')
                ++lineNum;
        }
        
        ++byteNum;
        
        prevCh = ch;
        ch = getc(file);
    }
    
    if (ferror(file))
        return false;
    
    cout << lineNum << " " << wordNum << " " << byteNum << endl;
    //TODO format output, using LWB column widths which were pre-computed
    //using stat(), formatted like this:
    //  182313  1033089 46488579 total
    
    (*pTotalLineNum) += lineNum;
    (*pTotalWordNum) += wordNum;
    (*pTotalByteNum) += byteNum;
    
    return true;
}
