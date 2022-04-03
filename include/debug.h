#pragma once

extern void __n64_assert(char *, int, char *, void *);
extern void __n64_assertClassic(char *, int, char *);

#define assert(cond, msg, arg) do {\
	if (!(cond)) __n64_assert(__FILE__, __LINE__, msg, arg);\
} while (0)

#define _assert(cond, msg) do {\
	if (!(cond)) __n64_assertClassic(__FILE__, __LINE__, msg);\
} while (0)

extern char *fresults[];
#define ERRCK(statement) do {\
	res = statement;\
	assert(res == FR_OK, "readdir error %s", fresults[res]);\
} while (0);

extern char *_n64assert_FILE;
extern int _n64assert_LINE;
extern char *_n64assert_MSG;
extern char _n64assert_BUF[100];
