#ifndef GRAB_WORK_H
#define GRAB_WORK_H

#include <pcre2.h>

typedef struct {
	char c;
	pcre2_code *re;
#if DEBUG
	bool free_me;
#endif
} op_t;

void process_file(const char *filename, unsigned char **buf);

#endif /* !GRAB_WORK_H */
