/* This file contains functions that are not part of the visible "interface".
 * They are essentially helper functions.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "simfs.h"

/* Internal helper functions first.
 */

FILE *
openfs(char *filename, char *mode)
{
    FILE *fp;
    if((fp = fopen(filename, mode)) == NULL) {
        perror("openfs");
        exit(1);
    }
    return fp;
}

void
closefs(FILE *fp)
{
    if(fclose(fp) != 0) {
        perror("closefs");
        exit(1);
    }
}

void
loadfiles(FILE *data, fentry files[MAXFILES])
{
    fread(files, sizeof(fentry), MAXFILES, data);
}

void
loadnodes(FILE *data, fnode fnodes[MAXBLOCKS])
{
    fread(fnodes, sizeof(fnode), MAXBLOCKS, data);
}

void
writefiles(FILE *data, fentry files[MAXFILES])
{
    int error = 0;
    error = fwrite(files, sizeof(fentry), MAXFILES, data);
    if (error != MAXFILES){
        fprintf(stderr, "Error: Could Not Write Data Succesfully\n");
        exit(1);
    }
}

void
writenodes(FILE *data, fnode fnodes[MAXBLOCKS])
{
    int error = 0;
    error = fwrite(fnodes, sizeof(fnode), MAXBLOCKS, data);
    if (error != MAXBLOCKS){
        fprintf(stderr, "Error: Could Not Write Data Succesfully\n");
        exit(1);
    }
}

int
getAvalibleBlock(fnode fnodes[MAXBLOCKS])
{
    for(int i = 0; i < MAXBLOCKS;i++){
        if (fnodes[i].blockindex < 0){
            return i;
        }
    }
    return -1;
}

/* File system operations: creating, deleting, reading, and writing to files.
 */

void
createfile(char *fs, char filename[12])
{   
    FILE *database;
    fentry files[MAXFILES];
    fnode fnodes[MAXBLOCKS];
    
    int space = -1; // Intialized to no availible space

    // Check Appropiate size of String
    if (strlen(filename) > 11){
        fprintf(stderr, "Error: Invalid Filename\n");
        exit(1);
    }

    // Opens Database and load files to create a file

    database = openfs(fs, "r+");
    if (database == NULL) {
        fprintf(stderr, "Error: Could not open Database\n");
        exit(1);
    }

    loadfiles(database, files);
    loadnodes(database, fnodes);

    // Check Space Avaiblity
    for(int i = 0; i < MAXFILES;i++){
        if(strcmp(files[i].name, "") == 0){
            space = 0;
        }
    }
    if (space == -1){
        fprintf(stderr, "Error: No Avaibile Space\n");
        exit(1);
    }

    // Check if file already exists
    bool fileExits = false;

    for(int i = 0; i < MAXFILES;i++){
        if(strcmp(files[i].name, filename) == 0){
            fileExits = true;
        }
    }
    if (fileExits){
        fprintf(stderr, "Error: File Already Exists\n");
        exit(1);
    }

    // Make File
    for(int i = 0; i < MAXFILES;i++){
        if(strcmp(files[i].name, "") == 0){
            strcpy(files[i].name, filename);
            i = MAXFILES;
        }
    }
    rewind(database);

    rewind(database);
    writefiles(database, files);
    writenodes(database,fnodes);
    rewind(database);
    closefs(database);
}

void
deletefile(char *fs, char filename[12])
{   
    FILE *database;
    fentry files[MAXFILES];
    fnode fnodes[MAXBLOCKS];

    // Check Appropiate size of String
    if (strlen(filename) > 11){
        fprintf(stderr, "Error: Invalid Filename\n");
        exit(1);
    }

    // Opens Database and load files and Nodes to create a file

    database = openfs(fs, "r+");
    if (database == NULL) {
        fprintf(stderr, "Error: Could not open Database\n");
        exit(1);
    }

    loadfiles(database, files);
    loadnodes(database, fnodes);
    // Check if file exists
    bool fileExits = false;
    for(int i = 0; i < MAXFILES;i++){
        if(strcmp(files[i].name, filename) == 0){
            fileExits = true;
        }
    }
    if (!fileExits){
        fprintf(stderr, "Error: File Doesnt Exist\n");
        exit(1);
    }

    int blockindex;
    // Deletes the file from system
    for(int i = 0; i < MAXFILES;i++){
        if(strcmp(files[i].name, filename) == 0){
            blockindex = files[i].firstblock;
            strcpy(files[i].name, "");
            files[i].size = 0;
            files[i].firstblock = -1;
            i = MAXFILES;
        }
    }
    rewind(database);

     // Free possible blocks occupied by the files.
    int zero[BLOCKSIZE] = {0};
    int prev = 0;
     while (!(blockindex < 0)){
         fseek(database, BLOCKSIZE * blockindex, SEEK_SET); // Move to the Block
         fwrite(zero, sizeof(int), BLOCKSIZE, database); // Write Zeros in the block to cover all 128 bytes
         prev = blockindex;
         fnodes[prev].blockindex = fnodes[prev].blockindex * -1; // Reset Values to indicate deletion or nonexistence
         blockindex = fnodes[prev].nextblock;  // Move to next Block if any
         fnodes[prev].nextblock = -1; // Reset Values to indicate deletionn or onexistence
     }

    rewind(database);
    writefiles(database, files); // Writes from Current
    // Also write the fnodes back
    writenodes(database,fnodes);

    rewind(database);

    closefs(database);
}

void
writefile(char *fs, char filename[12], int start, int length)
{   
    FILE *database;
    fentry files[MAXFILES];
    fnode fnodes[MAXBLOCKS];

    // Check Appropiate size of String
    if (strlen(filename) > 11){
        fprintf(stderr, "Error: Invalid Filename\n");
        exit(1);
    }
    // Opens Database and load files and Nodes to create a file

    database = openfs(fs, "r+");
    if (database == NULL) {
        fprintf(stderr, "Error: Could not open Database\n");
        exit(1);
    }

    loadfiles(database, files);
    loadnodes(database, fnodes);
    rewind(database);

    // Check if file exists
    bool fileExits = false;
    for(int i = 0; i < MAXFILES;i++){
        if(strcmp(files[i].name, filename) == 0){
            fileExits = true;
        }
    }
    if (!fileExits){
        fprintf(stderr, "Error: File Doesnt Exist\n");
        exit(1);
    }
    // Check Valid Start
    for(int i = 0; i < MAXFILES;i++){
        if(strcmp(files[i].name, filename) == 0){
            if (start > files[i].size){
                fprintf(stderr, "Error: Invalid Start Input\n");
                exit(1);
            }
        }
    }
    if (length <= 0){
        fprintf(stderr, "Error: Invalid Length Input\n");
        exit(1);
    }


    // Check if there is enough room in the file space blocks to write the data given. If there isnt submit an error.
    // Check if there is space in the current occupying blocks of the file.

    rewind(database);
    int blockindex;
    int firstBlock;
    int zeroBuffer[BLOCKSIZE] = {0};
    int fileIndex;
    int fileSize;
    
    for(int i = 0; i < MAXFILES;i++){
        if(strcmp(files[i].name, filename) == 0){
            blockindex = files[i].firstblock;
            fileSize = files[i].size;
            fileIndex = i;
            i = MAXFILES;
        }
    }
    firstBlock = blockindex;
 
    int bytesFreeInLastBlock = BLOCKSIZE - (fileSize % BLOCKSIZE);

    int spaceFreeTotal = 0;
    int overwrittenSpace = (fileSize - start);

    spaceFreeTotal += overwrittenSpace;
    spaceFreeTotal += bytesFreeInLastBlock;
    // Blocks Completey Empty
    int freeBlocks = 0;
    for(int i = 0; i < MAXBLOCKS; i++){
        if (fnodes[i].blockindex < 0){
            freeBlocks++;
        }
    }
    spaceFreeTotal += (BLOCKSIZE * freeBlocks);

    if (spaceFreeTotal < length){
            fprintf(stderr, "Error: No Free Space\n");
            exit(1);
    }

    // Read the input
    int readInput = 0;
    char input[length];
    readInput = fread(input, sizeof(char), length, stdin);
    if (readInput != length){
        fprintf(stderr, "Error: Unable to read from stdin\n");
        exit(1);
    }

    if (start + length > fileSize){
        files[fileIndex].size += (length - (fileSize - start));
    }

    // int position = start;
    int written = 0;

    // Writing for an empty file.
    int prev;
    int offSet = start;
    if (blockindex < 0){

        blockindex = getAvalibleBlock(fnodes);
        fseek(database, BLOCKSIZE * blockindex, SEEK_SET);
        fwrite(zeroBuffer, sizeof(char), BLOCKSIZE, database);

        files[fileIndex].firstblock = blockindex;
        fnodes[blockindex].blockindex = fnodes[blockindex].blockindex * -1;

        fseek(database, BLOCKSIZE * blockindex, SEEK_SET);
        while (written < length){
                 
            if ((length - written) <= BLOCKSIZE){
                fwrite(&input[written], sizeof(char), (length - written), database);
                written += length;
            }
            else if (length > BLOCKSIZE){
                fwrite(&input[written], sizeof(char), BLOCKSIZE, database);
                written += BLOCKSIZE;
            }
            if ((written > 0) && (written % 128 == 0) && (written < length)){
                prev = blockindex;
                blockindex = getAvalibleBlock(fnodes);
                fseek(database, BLOCKSIZE * blockindex, SEEK_SET);
                fwrite(zeroBuffer, sizeof(char), BLOCKSIZE, database);
                fnodes[blockindex].blockindex = fnodes[blockindex].blockindex * -1;
                fnodes[prev].nextblock = blockindex;
                fseek(database, BLOCKSIZE * blockindex, SEEK_SET);
            }
        
        } 
    }
    else { // If the file Already has data inside.

        while (offSet > BLOCKSIZE){
           offSet -= BLOCKSIZE;
           blockindex = fnodes[blockindex].nextblock;
        }
        fseek(database, BLOCKSIZE * blockindex + offSet, SEEK_SET);

        if (start + length <= fileSize && (offSet + length <= BLOCKSIZE)){
            fwrite(input, sizeof(char), length, database);
        }
        else if (start + length <= fileSize && (offSet + length > BLOCKSIZE)){
            while (written < length ){
                if (written == 0 && length > (BLOCKSIZE-offSet)){
                    fwrite(input, sizeof(char), (BLOCKSIZE-offSet), database);
                    written += (BLOCKSIZE-offSet);
                    blockindex = fnodes[blockindex].nextblock;
                    offSet = 0;
                }
                else if (written > 0 && (length - written) >= (BLOCKSIZE)){
                    fseek(database, BLOCKSIZE * blockindex, SEEK_SET);
                    fwrite(&input[written], sizeof(char),BLOCKSIZE, database);
                    written += BLOCKSIZE;
                    blockindex = fnodes[blockindex].nextblock;
                    offSet = 0;
                }
                else if (written > 0 && (length - written) < (BLOCKSIZE)){
                    fseek(database, BLOCKSIZE * blockindex, SEEK_SET);
                    fwrite(&input[written], sizeof(char),(length - written), database);
                    written += (length - written);
                    blockindex = fnodes[blockindex].nextblock;
                    offSet = 0;
                }
            }
        }
        else if (start == fileSize && (offSet + length <= BLOCKSIZE)){
            fwrite(input, sizeof(char), length, database);
        }
        else if (start == fileSize && (offSet + length > BLOCKSIZE)){
            while (written < length ){
                if (written == 0 && length > (BLOCKSIZE-offSet)){
                    fwrite(input, sizeof(char), (BLOCKSIZE-offSet), database);
                    written += (BLOCKSIZE-offSet);
                    offSet = 0;
                }
                else if (written > 0 && (length - written) >= (BLOCKSIZE)){
                    prev = blockindex;
                    blockindex = getAvalibleBlock(fnodes);
                    fseek(database, BLOCKSIZE * blockindex, SEEK_SET);
                    fwrite(zeroBuffer, sizeof(char), BLOCKSIZE, database);
                    fnodes[blockindex].blockindex = fnodes[blockindex].blockindex * -1;
                    fnodes[prev].nextblock = blockindex;

                    fseek(database, BLOCKSIZE * blockindex, SEEK_SET);
                    fwrite(&input[written], sizeof(char),BLOCKSIZE, database);
                    written += BLOCKSIZE;
                    offSet = 0;
                }
                else if (written > 0 && (length - written) < (BLOCKSIZE)){
                    prev = blockindex;
                    blockindex = getAvalibleBlock(fnodes);
                    fseek(database, BLOCKSIZE * blockindex, SEEK_SET);
                    fwrite(zeroBuffer, sizeof(char), BLOCKSIZE, database);
                    fnodes[blockindex].blockindex = fnodes[blockindex].blockindex * -1;
                    fnodes[prev].nextblock = blockindex;

                    fseek(database, BLOCKSIZE * blockindex, SEEK_SET);
                    fwrite(&input[written], sizeof(char),(length - written), database);
                    written += (length - written);
                    offSet = 0;
                }
            }
        } 
        else if (start + length > fileSize && (offSet + length <= BLOCKSIZE)){
            fwrite(input, sizeof(char), length, database);
        }

        else if (start + length > fileSize && (offSet + length > BLOCKSIZE))
        {
            // Write as much as data possible in the file by overwritting
            int overwrite = fileSize - start;
            while (written < overwrite){
                    if (written == 0 && overwrite > (BLOCKSIZE-offSet)){
                        fwrite(input, sizeof(char), (BLOCKSIZE-offSet), database);
                        written += (BLOCKSIZE-offSet);
                        blockindex = fnodes[blockindex].nextblock;
                        offSet = 0;
                    }
                    else if (written == 0 && overwrite <= (BLOCKSIZE-offSet)){
                        fwrite(input, sizeof(char), overwrite, database);
                        written += overwrite;
                        blockindex = fnodes[blockindex].nextblock;
                        offSet = 0;
                    }
                    else if (written > 0 && (overwrite - written) >= (BLOCKSIZE)){
                        fseek(database, BLOCKSIZE * blockindex, SEEK_SET);
                        fwrite(&input[written], sizeof(char),BLOCKSIZE, database);
                        written += BLOCKSIZE;
                        blockindex = fnodes[blockindex].nextblock;
                        offSet = 0;
                    }
                    else if (written > 0 && (overwrite - written) < (BLOCKSIZE)){
                        fseek(database, BLOCKSIZE * blockindex, SEEK_SET);
                        fwrite(&input[written], sizeof(char),(overwrite - written), database);
                        written += (overwrite - written);
                        blockindex = fnodes[blockindex].nextblock;
                        offSet = 0;
                    }
                }

             // Now append all the remaining data to the end.
            offSet = fileSize;
            blockindex = firstBlock;
            int writtenEnd = 0;
            int lengthRemaining = (length - (fileSize - start));
            
            while (offSet > BLOCKSIZE){
                offSet -= BLOCKSIZE;
                blockindex = fnodes[blockindex].nextblock;
            }
            fseek(database, BLOCKSIZE * blockindex + offSet, SEEK_SET);
            
             while (writtenEnd < lengthRemaining){

                if (writtenEnd == 0 && lengthRemaining > (BLOCKSIZE-offSet)){
                    fwrite(&input[written], sizeof(char), (BLOCKSIZE-offSet), database);
                    written += (BLOCKSIZE-offSet);
                    writtenEnd += (BLOCKSIZE-offSet);
                    offSet = 0;
                }
                else if (writtenEnd == 0 && lengthRemaining <= (BLOCKSIZE-offSet)){
                    fwrite(&input[written], sizeof(char), lengthRemaining, database);
                    written += lengthRemaining;
                    writtenEnd += lengthRemaining;
                    offSet = 0;
                }
                else if (writtenEnd > 0 && (lengthRemaining - writtenEnd) >= (BLOCKSIZE)){
                    prev = blockindex;
                    blockindex = getAvalibleBlock(fnodes);
                    fseek(database, BLOCKSIZE * blockindex, SEEK_SET);
                    fwrite(zeroBuffer, sizeof(char), BLOCKSIZE, database);
                    fnodes[blockindex].blockindex = fnodes[blockindex].blockindex * -1;
                    fnodes[prev].nextblock = blockindex;

                    fseek(database, BLOCKSIZE * blockindex, SEEK_SET);
                    fwrite(&input[written], sizeof(char),BLOCKSIZE, database);
                    written += BLOCKSIZE;
                    writtenEnd += BLOCKSIZE;
                    offSet = 0;
                }
                else if (written > 0 && (lengthRemaining - writtenEnd) < (BLOCKSIZE)){
                    prev = blockindex;
                    blockindex = getAvalibleBlock(fnodes);
                    fseek(database, BLOCKSIZE * blockindex, SEEK_SET);
                    fwrite(zeroBuffer, sizeof(char), BLOCKSIZE, database);

                    fnodes[prev].nextblock = blockindex;
                    
                    fnodes[blockindex].blockindex = fnodes[blockindex].blockindex * -1;
                    fseek(database, BLOCKSIZE * blockindex, SEEK_SET);
                    
                    fwrite(&input[written], sizeof(char),(lengthRemaining - writtenEnd), database);
                    written += (lengthRemaining - writtenEnd);
                    writtenEnd += (lengthRemaining - writtenEnd);
                    offSet = 0;
                }
            }

        }
    }
    rewind(database);
    writefiles(database, files); // Writes from Current
    writenodes(database,fnodes);
    rewind(database);

    closefs(database);
}

void
readfile(char *fs, char filename[12], int start, int length)
{   
    FILE *database;
    fentry files[MAXFILES];
    fnode fnodes[MAXBLOCKS];

    // Check Appropiate size of String
    if (strlen(filename) > 11){
        fprintf(stderr, "Error: Invalid Filename\n");
        exit(1);
    }
    // Opens Database and load files and Nodes to create a file

    database = openfs(fs, "r+");
    if (database == NULL) {
        fprintf(stderr, "Error: Could not open Database\n");
        exit(1);
    }

    loadfiles(database, files);
    loadnodes(database, fnodes);
    rewind(database);

    // Check if file exists
    bool fileExits = false;

    for(int i = 0; i < MAXFILES;i++){
        if(strcmp(files[i].name, filename) == 0){
            fileExits = true;
        }
    }
    if (!fileExits){
        fprintf(stderr, "Error: File Doesnt Exist\n");
        exit(1);
    }

    int sizeOfFile = 0;
    // Check if size is greater than 0
    for(int i = 0; i < MAXFILES;i++){
        if(strcmp(files[i].name, filename) == 0){
            sizeOfFile = files[i].size;
            if (files[i].size == 0){
                fprintf(stderr, "Error: Nothing to read from file\n");
                exit(1);
            }
        }
    }

    // Check Start is greater than size of file

    for(int i = 0; i < MAXFILES;i++){
        if(strcmp(files[i].name, filename) == 0){
            if (start > files[i].size){
                fprintf(stderr, "Error: Invalid Start Input\n");
                exit(1);
            }
        }
    }

    if (length <= 0){
        fprintf(stderr, "Error: Invalid Length Input\n");
        exit(1);
    }

    int blockindex;
    // Gets first block index
    for(int i = 0; i < MAXFILES;i++){
        if(strcmp(files[i].name, filename) == 0){
            blockindex = files[i].firstblock;
            i = MAXFILES;
        }
    }

     // Read the whole file into a char array.
     char buffer[sizeOfFile];
     int bytesRead = 0;
     int tempSize = sizeOfFile;

    while (!(blockindex < 0)) {
        fseek(database, BLOCKSIZE * blockindex, SEEK_SET);
        if(tempSize > BLOCKSIZE){
            fread(&buffer[bytesRead], sizeof(char), BLOCKSIZE, database); // Write Zeros in the block to cover all 128 bytes
            bytesRead += BLOCKSIZE;
            tempSize -= BLOCKSIZE;   
        }
        else{
            fread(&buffer[bytesRead], sizeof(char), tempSize, database); // Write Zeros in the block to cover all 128 bytes
        }
        blockindex = fnodes[blockindex].nextblock; 
    }
    rewind(database);
    if (!((start + length) <= sizeOfFile)){
        fprintf(stderr, "Error: Invalid Length\n");
        exit(1);
    }
    
    int charsRead = 0;
    for(int i = start; i < sizeOfFile; i++){
        printf("%c", buffer[i]);
        charsRead++;
        if (charsRead == length){
            i = sizeOfFile;
        }
    }
}

void
info(char *fs, char filename[12]){

    FILE *database;
    fentry files[MAXFILES];
    fnode fnodes[MAXBLOCKS];

    // Check Appropiate size of String
    if (strlen(filename) > 11){
        fprintf(stderr, "Error: Invalid Filename\n");
        exit(1);
    }
    // Opens Database and load files and Nodes to create a file
    database = openfs(fs, "r+");
    if (database == NULL) {
        fprintf(stderr, "Error: Could not open Database\n");
        exit(1);
    }

    loadfiles(database, files);
    loadnodes(database, fnodes);
    rewind(database);

    // Check if file exists
    bool fileExits = false;

    for(int i = 0; i < MAXFILES;i++){
        if(strcmp(files[i].name, filename) == 0){
            fileExits = true;
        }
    }
    if (!fileExits){
        fprintf(stderr, "Error: File Doesnt Exist\n");
        exit(1);
    }

    int sizeOfFile = 0;
    // Check if size is greater than 0
    for(int i = 0; i < MAXFILES;i++){
        if(strcmp(files[i].name, filename) == 0){
            sizeOfFile = files[i].size;
            if (files[i].size == 0){
                printf("EMPTY");
                exit(1);
            }
        }
    }

    int blockindex;
    // Gets first block index
    for(int i = 0; i < MAXFILES;i++){
        if(strcmp(files[i].name, filename) == 0){
            blockindex = files[i].firstblock;
            i = MAXFILES;
        }
    }

     // Read the whole file into a char array.
     char buffer[sizeOfFile];
     int bytesRead = 0;
     int tempSize = sizeOfFile;

    while (!(blockindex < 0)) {
        fseek(database, BLOCKSIZE * blockindex, SEEK_SET);
        if(tempSize > BLOCKSIZE){
            fread(&buffer[bytesRead], sizeof(char), BLOCKSIZE, database); // Write Zeros in the block to cover all 128 bytes
            bytesRead += BLOCKSIZE;
            tempSize -= BLOCKSIZE;   
        }
        else{
            fread(&buffer[bytesRead], sizeof(char), tempSize, database); // Write Zeros in the block to cover all 128 bytes
        }
        blockindex = fnodes[blockindex].nextblock; 
    }
    rewind(database);

    int accumulator[256] = {0};

    int maximumCount = 0;
    char target;

    for (int i = 0; i < sizeOfFile; i++){
        accumulator[(int)buffer[i]] = accumulator[(int)buffer[i]] + 1;
    }
    for (int i = 0; i < 256; i++){
        if (maximumCount < accumulator[i]){
            maximumCount = accumulator[i];
        }   
    }
    for (int i = 0; i < 256; i++){
        if (maximumCount == accumulator[i]){
            target = (char)i; 
        }   
    }
    printf("%d | %c",maximumCount, target);
}

// Signatures omitted; design as you wish.

