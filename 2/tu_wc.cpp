/*
  Custom version of standard 'wc' command
  Author: Rains Jordan
  Version: 1.0
  
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
  -Does not work on Windows, due to the use of POSIX stat() to determine byte
   counts for each file. These are used to compute the maximum possible width
   for each column in the output.
  -This program counts lines and words slightly differently than standard 'wc'
   does. This is mainly noticeable when reading binary files.
  -Unlike standard 'wc', this program doesn't print out stats for a file which
   could not be read (say, a directory).
*/

#include <iostream>
#include <cstdio>
#include <cstring>
#include <algorithm>
#include <iomanip>
#include <getopt.h>
#include <sys/stat.h>

using std::cout;
using std::cerr;
using std::endl;
using std::fill_n;

//For our counting metrics, use the same data type that POSIX size() uses to
//store file sizes.
typedef off_t Count;

const char INVALID_CHAR = -1;

//Each column must contain at least 1 character of whitespace and 1 character
//of actual content.
const int MIN_COLUMN_WIDTH = 2;

bool ProcessFile(const char* filename, FILE* file, bool bUseLines,
                 bool bUseWords, bool bUseBytes, Count* pTotalLineNum,
                 Count* pTotalWordNum, Count* pTotalByteNum, int columnWidth);

void PrintStats(bool bUseLines, bool bUseWords, bool bUseBytes,
                Count* pLineNum, Count* pWordNum, Count* pByteNum,
                const char* label, int columnWidth);

int main(int argc, char** argv) {
	//Follow the traditional method of creating a struct of values for use in
	//getopt_long().
	static struct option longOptions[] = {
		{"lines", no_argument, 0, 'l'},
		{"words", no_argument, 0, 'w'},
		{"bytes", no_argument, 0, 'c'},
		{0,       0,           0, 0}
	};
    
    bool bUseLines = false, bUseWords = false, bUseBytes = false;
    
    int opt;
    
    //Get the values associated with named command-line parameters.
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
    
    if (!bUseLines && !bUseWords && !bUseBytes)
        bUseLines = bUseWords = bUseBytes = true;
    
    Count totalLineNum = 0, totalWordNum = 0, totalByteNum = 0;
    
    //Process the non-option arguments (which have been automatically
    //rearranged by getopt_long() to follow the option-related arguments).
    
    const int fileNum = argc - optind;
    
    //In order to pretty-print the data, we need to know the maximum width
    //of any column. To do this, we'll use stat() to precompute the total
    //byte num. From that, we can get the largest possible column width.
    //We'll use this width even if the user chooses not to display byte
    //information. The standard 'wc' command seems to do something similar.
    struct stat st;
    int columnWidth = MIN_COLUMN_WIDTH;
    Count tempTotalByteNum = 0;
    
    const char* totalLabel = "";
    bool bUsingStdin = false;
    
    if (fileNum > 0) {
        for (int i = optind; i < argc; ++i) {
            if (stat(argv[i], &st) == 0) {
                tempTotalByteNum += st.st_size;
            }
            else {
                fprintf(stderr, "%s: ", argv[i]);
                perror("Unable to get stat() data");
                return 1;
            }
        }
        
        while (tempTotalByteNum >= 10) {
            tempTotalByteNum /= 10;
            ++columnWidth;
        }
        
        for (int i = optind; i < argc; ++i) {
            FILE* file = fopen(argv[i], "r");
            
            //Question: Do you prefer 'nullptr' even in old-school C-style
            //situations like this and the other usage with getopt_long above?
            
            if (file == NULL) {
                cerr << "No such file or directory: '" << argv[i] << "'"
                     << endl;
            }
            else {
                ProcessFile(argv[i], file, bUseLines, bUseWords, bUseBytes,
                            &totalLineNum, &totalWordNum, &totalByteNum,
                            columnWidth);
            }
        }
        
        totalLabel = "total";
    }
    else {
        ProcessFile("", stdin, bUseLines, bUseWords, bUseBytes, &totalLineNum,
                    &totalWordNum, &totalByteNum, columnWidth);
        
        tempTotalByteNum = totalByteNum;
        
        while (tempTotalByteNum >= 10) {
            tempTotalByteNum /= 10;
            ++columnWidth;
        }
        
        bUsingStdin = true;
    }
    
    //Only bother printing a total if there are multiple files.
    if (bUsingStdin || fileNum > 1) {
        PrintStats(bUseLines, bUseWords, bUseBytes,
                   &totalLineNum, &totalWordNum, &totalByteNum,
                   totalLabel, columnWidth);
        
        if (bUsingStdin)
            cout << endl;
    }
    
    return 0;
}

//Scan a text file and gather various statistics from the data in it.
//Returns false if failed to process file, true otherwise.
bool ProcessFile(const char* filename, FILE* file, bool bUseLines,
                 bool bUseWords, bool bUseBytes, Count* pTotalLineNum,
                 Count* pTotalWordNum, Count* pTotalByteNum, int columnWidth) {
    Count lineNum = 0, wordNum = 0, byteNum = 0;
    
    char prevCh = INVALID_CHAR;
    char ch = getc(file);
    
    //For word-counting purposes, we'll pretend the start of the input was
    //preceded by whitespace.
    bool bPrevWasSpace = true;
    
    bool bCurrentIsSpace;
    
    //Count statistics. To avoid constant comparisons during the loop, we'll
    //accumulate all data, regardless of whether we print it.
    
    //Note: When reading certain binary files, getc() can return EOF even
    //though neither feof or ferror has been set for the file. I'm not sure
    //why.
    while (ch != EOF || (!feof(file) && !ferror(file))) {
        if (ch == '\r')
            ++lineNum;
        else if (ch == '\n') {
            if (prevCh != '\r')
                ++lineNum;
        }
        
        bCurrentIsSpace = isspace(ch);
        
        if (!bCurrentIsSpace && bPrevWasSpace)
            ++wordNum;
        
        //Note: If we're dealing with a file rather than stdin, we're
        //recomputing this data, since we already precached file sizes.
        //I've done things this way to avoid checking if we're dealing with
        //stdin or a file here, each loop iteration. Then again, maybe those
        //checks would be optimized out by the compiler... so as a TODO, I
        //could change this behavior to reuse the previously obtained data.
        ++byteNum;
        
        prevCh = ch;
        bPrevWasSpace = bCurrentIsSpace;
        ch = getc(file);
    }
    
    if (ferror(file)) {
        if (strlen(filename) > 0)
            fprintf(stderr, "%s: ", filename);
        perror("Read error");
        
        return false;
    }
    
    (*pTotalLineNum) += lineNum;
    (*pTotalWordNum) += wordNum;
    (*pTotalByteNum) += byteNum;
    
    if (strlen(filename) > 0) {
        PrintStats(bUseLines, bUseWords, bUseBytes,
                   &lineNum, &wordNum, &byteNum, filename, columnWidth);
    }
    
    return true;
}

//Print stats about a file that has been read (or stdin).
void PrintStats(bool bUseLines, bool bUseWords, bool bUseBytes,
                Count* pLineNum, Count* pWordNum, Count* pByteNum,
                const char* label, int columnWidth) {
    //Streams are used rather than printf() since they don't need to know
    //Count's internal type for formatting purposes.
    
    if (bUseLines)
        cout << std::right << std::setw(columnWidth) << *pLineNum;
    
    if (bUseWords)
        cout << std::right << std::setw(columnWidth) << *pWordNum;
    
    if (bUseBytes)
        cout << std::right << std::setw(columnWidth) << *pByteNum;
    
    //Note: The program logic is arranged such that SOMETHING has been printed.
    if (strlen(label) > 0)
        cout << " " << label << endl;
}
