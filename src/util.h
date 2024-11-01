#ifndef GRAB_UTIL_H
#define GRAB_UTIL_H

int getenv_posnum(const char *ev, int fallback);
void pcre2_bitch_and_die(int ec, const char *fmt, ...);

#endif /* !GRAB_UTIL_H */