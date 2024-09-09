/* This program simulates a file system within an actual file.  The
 * simulated file system has only one directory, and two types of
 * metadata structures defined in simfstypes.h.
 */

/* Simfs is run as:
 * simfs -f myfs command args
 * where the -f option specifies the actual file that the simulated file system
 * occupies, the command is the command to be run on the simulated file system,
 * and args represents a list of arguments for the command.  Note that
 * different commands take different numbers of arguments.
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "simfs.h"

// We use the ops array to match the file system command entered by the user.
#define MAXOPS  7
char *ops[MAXOPS] = {"initfs", "printfs", "createfile", "readfile",
                     "writefile", "deletefile", "info"};
int find_command(char *);

int
main(int argc, char **argv)
{
    int oc;       /* option character */
    char *cmd;    /* command to run on the file system */
    char *fsname; /* name of the simulated file system file */

    char *usage_string = "Usage: simfs -f file cmd arg1 arg2 ...\n";

    /* Get and check the arguments */
    if(argc < 4) {
        fputs(usage_string, stderr);
        exit(1);
    }

    while((oc = getopt(argc, argv, "f:")) != -1) {
        switch(oc) {
        case 'f' :
            fsname = optarg;
            break;
        default:
            fputs(usage_string, stderr);
            exit(1);
        }
    }

    /* Get the command name */
    cmd = argv[optind];
    optind++;

    switch((find_command(cmd))) {
    case 0: /* initfs */
        initfs(fsname);
        break;
    case 1: /* printfs */
        printfs(fsname);
        break;
    case 2: /* createfile */
        if(argc < 5) {
            fputs(usage_string, stderr);
            exit(1);
        }
        createfile(fsname, argv[4]);
        break;
    case 3: /* readfile */
        if(argc < 7) {
            fputs(usage_string, stderr);
            exit(1);
        }
        int start = atoi(argv[5]);
        int length = atoi(argv[6]);
        readfile(fsname, argv[4], start ,length);
        break;
    case 4: /* writefile */
        if(argc < 7) {
            fputs(usage_string, stderr);
            exit(1);
        }
        int startw = atoi(argv[5]);
        int lengthw = atoi(argv[6]);
        writefile(fsname, argv[4], startw ,lengthw);
        break;
    case 5: /* deletefile */
        // fprintf(stderr, "Error: deletefile not yet implemented\n");
        if(argc < 5) {
            fputs(usage_string, stderr);
            exit(1);
        }
        deletefile(fsname, argv[4]);
        break;
    case 6: /* info */
        if(argc < 4) {
            fputs(usage_string, stderr);
            exit(1);
        }
        info(fsname, argv[4]);
        break;
    default:
        fprintf(stderr, "Error: Invalid command\n");
        exit(1);
    }

    return 0;
}

/* Returns a integer corresponding to the file system command that
 * is to be executed
 */
int
find_command(char *cmd)
{
    int i;
    for(i = 0; i < MAXOPS; i++) {
        if ((strncmp(cmd, ops[i], strlen(ops[i]))) == 0) {
            return i;
        }
    }
    fprintf(stderr, "Error: Command %s not found\n", cmd);
    return -1;
}
