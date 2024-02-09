#ifndef HEADER_FILE
#define HEADER_FILE

#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define YEL   "\x1B[33m"
#define BLU   "\x1B[34m"
#define MAG   "\x1B[35m"
#define CYN   "\x1B[36m"
#define WHT   "\x1B[37m"
#define RESET "\x1B[0m"

char *col(int attr, int fg, int bg){
    char* string=NULL;
    sprintf(string,"%c[%d;%d;%dm", 0x1B, attr, fg + 30, bg + 40);
    return string;
}

#endif
