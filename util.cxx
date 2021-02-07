/*
 * calmwm - the calm window manager
 *
 * Copyright (c) 2004 Marius Aamodt Eriksen <marius@monkey.org>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * $OpenBSD$
 */

#include "calmwm.hxx"
#include "queue.hxx"

#include <cerrno>
#include <climits>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <err.h>
#include <sys/types.h>
#include <unistd.h>

static void log_msg(char const*, va_list);

void u_spawn(char* argstr)
{
	switch (fork()) {
	case 0: u_exec(argstr); exit(1);
	case -1: warn("fork");
	default: break;
	}
}

void u_exec(char* argstr)
{
#define MAXARGLEN 20
	char *args[MAXARGLEN], **ap = args;
	char **end = &args[MAXARGLEN - 2], *tmp;
	char* s = argstr;

	while (ap < end && (*ap = strsep(&argstr, " \t")) != nullptr) {
		if (**ap == '\0') continue;
		ap++;
		if (argstr != nullptr) {
			/* deal with quoted strings */
			switch (argstr[0]) {
			case '"':
			case '\'':
				if ((tmp = strchr(argstr + 1, argstr[0])) != nullptr) {
					*(tmp++) = '\0';
					*(ap++) = ++argstr;
					argstr = tmp;
				}
				break;
			default: break;
			}
		}
	}
	*ap = nullptr;

	(void)setsid();
	(void)execvp(args[0], args);
	warn("%s", s);
}

char* u_argv(char* const* argv)
{
	size_t siz = 0;
	int i;
	char* p;

	if (argv == nullptr) return nullptr;

	for (i = 0; argv[i]; i++) siz += strlen(argv[i]) + 1;
	if (siz == 0) return nullptr;

	p = (char*)std::malloc(siz);
	strlcpy(p, argv[0], siz);
	for (i = 1; argv[i]; i++) {
		strlcat(p, " ", siz);
		strlcat(p, argv[i], siz);
	}
	return p;
}

static void log_msg(char const* msg, va_list ap)
{
	char* fmt;

	if (asprintf(&fmt, "%s\n", msg) == -1) {
		vfprintf(stderr, msg, ap);
		fprintf(stderr, "\n");
	} else {
		vfprintf(stderr, fmt, ap);
		free(fmt);
	}
	fflush(stderr);
}

void log_debug(int level, char const* func, char const* msg, ...)
{
	char* fmt;
	va_list ap;

	if (conf->debug < level) return;

	va_start(ap, msg);
	asprintf(&fmt, "debug%d: %s: %s", level, func, msg);
	log_msg(fmt, ap);
	free(fmt);
	va_end(ap);
}
