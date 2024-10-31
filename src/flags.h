#ifndef GRAB_FLAGS_H
#define GRAB_FLAGS_H

typedef struct {
	bool b : 1;
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

#endif /* !GRAB_FLAGS_H */
