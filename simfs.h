#include <stdio.h>
#include "simfstypes.h"

/* File system operations */
void printfs(char *);
void initfs(char *);
void createfile(char *, char *);
void deletefile(char *, char *);
void readfile(char *, char *, int , int);
void writefile(char *, char *, int , int);
void info(char *, char *);

/* Internal functions */
FILE *openfs(char *filename, char *mode);
void closefs(FILE *fp);
void loadfiles(FILE *data, fentry files[MAXFILES]);
void loadnodes(FILE *data, fnode fnodes[MAXBLOCKS]);
void writefiles(FILE *data, fentry files[MAXFILES]);
void writenodes(FILE *data, fnode fnodes[MAXBLOCKS]);
int getAvalibleBlock(fnode fnodes[MAXBLOCKS]);
