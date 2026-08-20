/*
 * Copyright © 2024 Red Hat, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include "config.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

static inline void
backtrace_print(FILE *fp)
{
#if HAVE_GSTACK
	pid_t parent, child;
	int pipefd[2];

	if (pipe(pipefd) == -1)
		return;

	parent = getpid();
	child = fork();

	if (child == 0) {
		char pid[8];

		close(pipefd[0]);
		dup2(pipefd[1], STDOUT_FILENO);

		sprintf(pid, "%d", parent);

		execlp("gstack", "gstack", pid, NULL);
		exit(errno);
	}

	/* parent */
	char buf[1024];
	int status, nread;

	close(pipefd[1]);
	waitpid(child, &status, 0);

	status = WEXITSTATUS(status);
	if (status != 0) {
		fprintf(fp, "ERROR: gstack failed, no backtrace available: %s\n",
			   strerror(status));
	} else {
		fprintf(fp, "\nBacktrace:\n");
		while ((nread = read(pipefd[0], buf, sizeof(buf) - 1)) > 0) {
			buf[nread] = '\0';
			fprintf(stderr, "%s", buf);
		}
		fprintf(fp, "\n");
	}
	close(pipefd[0]);
#endif
}
