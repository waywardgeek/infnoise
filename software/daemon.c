// Functions used when running in the background

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>

#include "infnoise.h"

// Write af PID in a file
static bool writePid(int32_t pid, char *fileName) {
    FILE *pidFile;
    int ret;
    pidFile = fopen(fileName,"w");
    if(pidFile == NULL) {
        return errno;
    }
    ret = fprintf(pidFile, "%d\n", pid);
    if (ret < 0) {
        return false;
    }
    fclose(pidFile);
    return true;
}

void startDaemon(bool daemon, bool pidFile, char *fileName) {
	if(!daemon) {
		// No backgrounding, optionslly write current PID
		if(pidFile) {
			writePid(getpid(), fileName);
		}
		return;
	}
	int32_t pid = fork();
	if(pid < 0) {
		fputs("fork() failed\n", stderr);
		exit(1);
	} else if(pid > 0) {
		// Parent
		if(pidFile) {
			if(!writePid(pid, fileName)) {
				exit(1);
			}
		}
		exit(0);
	}
	// Child
}

bool isSuperUser(void) {
	return (geteuid() == 0);
}
