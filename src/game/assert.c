#include <ultra64.h>

#include "n64_defs.h"
#include "debug.h"

void __n64_assert(char *fl, int line, char *fmt, void *arg) {
	sprintf(_n64assert_BUF, fmt, arg);

	__n64Assert_dispatch(fl, line, _n64assert_BUF);
}

void __n64_assertClassic(char *fl, int line, char *msg) {
	__n64Assert_dispatch(fl, line, msg);
}

