#ifndef GRAB_UTIL_H
#define GRAB_UTIL_H

#include <mbstring.h>

int getenv_posnum(const char *ev, int fallback);
void pcre2_bitch_and_die(int ec, const char *fmt, ...);
bool islbrk(u8view_t g);

#endif /* !GRAB_UTIL_H */