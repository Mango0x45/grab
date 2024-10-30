#ifndef GRAB2_FLAGS_H
#define GRAB2_FLAGS_H

typedef struct {
	bool c : 1;
	bool i : 1;
	bool l : 1;
	bool p : 1;
	bool s : 1;
	bool U : 1;
	bool z : 1;

#if !GIT_GRAB
	bool do_header : 1;
#endif
} flags_t;

#if !MAIN_C
extern
#endif
flags_t flags;

#endif /* !GRAB2_FLAGS_H */
