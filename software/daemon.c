/* Functions used when running in the background
 */

#include <stdio.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>

#include "infnoise.h"

int writePid(pid_t pid, char *fileName) {
	FILE *pidFile;
	int ret;
	pidFile = fopen(fileName,"w");
	if(pidFile == NULL)
		return errno;
	ret = fprintf(pidFile, "%d\n", pid);
	if (ret < 0)
		return ret;
	return fclose(pidFile);
}
