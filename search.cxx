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

#include <array>
#include <cerrno>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <err.h>
#include <fnmatch.h>
#include <glob.h>
#include <string_view>
#include <sys/types.h>
#include <unistd.h>

#define PATH_ANY 0x0001
#define PATH_EXEC 0x0002

static void match_path_type(struct menu_q*, char*, int);

static bool match_substr(std::string_view sub, std::string_view str, bool zeroidx = false)
{
	if (sub.empty() || str.empty()) return false;
	if (sub.length() > str.length()) return false;

	auto flen {zeroidx ? 0 : str.length() - sub.length()};
	for (decltype(flen) n {0}; n <= flen; n++)
		if (strncasecmp(sub.data(), str.substr(n).data(), sub.length()) == 0) return true;

	return false;
}

void search_match_client(struct menu_q* menuq, struct menu_q* resultq, char* search)
{
	std::array<Menu*, 3> tierp {0, 0, 0};
	Menu *mi, *before = nullptr;
	Client_ctx* cc;
	Winname* wn;

	TAILQ_INIT(resultq);
	TAILQ_FOREACH(mi, menuq, entry)
	{
		int tier = -1, t;
		cc = (Client_ctx*)mi->ctx;

		/* Match on label. */
		if (match_substr(search, cc->label)) tier = 0;

		/* Match on window name history, from present to past. */
		if (tier < 0) {
			TAILQ_FOREACH_REVERSE(wn, &cc->nameq, name_q, entry)
			if (match_substr(search, wn->name)) {
				tier = 1;
				break;
			}
		}

		/* Match on window resource class. */
		if ((tier < 0) && match_substr(search, cc->res_class)) tier = 2;

		if (tier < 0) continue;

		/* Current window is ranked down. */
		if ((tier < tierp.size() - 1) && (cc->flags & CLIENT_ACTIVE)) tier++;

		/* Hidden window is ranked up. */
		if ((tier > 0) && (cc->flags & CLIENT_HIDDEN)) tier--;

		/*
		 * If you have a tierp, insert after it, and make it
		 * the new tierp.  If you don't have a tierp, find the
		 * first nonzero tierp above you, insert after it.
		 * Always make your current tierp the newly inserted
		 * entry.
		 */
		for (t = tier; t >= 0 && ((before = tierp[t]) == nullptr); t--)
			;

		if (before == nullptr)
			TAILQ_INSERT_HEAD(resultq, mi, resultentry);
		else
			TAILQ_INSERT_AFTER(resultq, before, mi, resultentry);

		tierp[tier] = mi;
	}
}

void search_match_cmd(struct menu_q* menuq, struct menu_q* resultq, char* search)
{
	Menu* mi;
	Cmd_ctx* cmd;

	TAILQ_INIT(resultq);
	TAILQ_FOREACH(mi, menuq, entry)
	{
		cmd = (Cmd_ctx*)mi->ctx;
		if (match_substr(search, cmd->name)) TAILQ_INSERT_TAIL(resultq, mi, resultentry);
	}
}

void search_match_group(struct menu_q* menuq, struct menu_q* resultq, char* search)
{
	Menu* mi;
	Group_ctx* gc;
	char* s;

	TAILQ_INIT(resultq);
	TAILQ_FOREACH(mi, menuq, entry)
	{
		gc = (Group_ctx*)mi->ctx;
		xasprintf(&s, "%d %s", gc->num, gc->name);
		if (match_substr(search, s)) TAILQ_INSERT_TAIL(resultq, mi, resultentry);
		free(s);
	}
}

static void match_path_type(struct menu_q* resultq, char* search, int flag)
{
	Menu* mi;
	char* pattern;
	glob_t g;
	int i;

	xasprintf(&pattern, "%s*", search);
	if (glob(pattern, GLOB_MARK, nullptr, &g) != 0) return;
	for (i = 0; i < g.gl_pathc; i++) {
		if ((flag & PATH_EXEC) && access(g.gl_pathv[i], X_OK)) continue;
		mi = (Menu*)xcalloc(1, sizeof(*mi));
		(void)strlcpy(mi->text, g.gl_pathv[i], sizeof(mi->text));
		TAILQ_INSERT_TAIL(resultq, mi, resultentry);
	}
	globfree(&g);
	free(pattern);
}

void search_match_exec(struct menu_q* menuq, struct menu_q* resultq, char* search)
{
	Menu *mi, *mj;
	int r;

	TAILQ_INIT(resultq);
	TAILQ_FOREACH(mi, menuq, entry)
	{
		if (match_substr(search, mi->text, true) == 0
		    && fnmatch(search, mi->text, 0) == FNM_NOMATCH)
			continue;
		TAILQ_FOREACH(mj, resultq, resultentry)
		{
			r = strcmp(mi->text, mj->text);
			if (r < 0) TAILQ_INSERT_BEFORE(mj, mi, resultentry);
			if (r <= 0) break;
		}
		if (mj == nullptr) TAILQ_INSERT_TAIL(resultq, mi, resultentry);
	}
	if (TAILQ_EMPTY(resultq)) match_path_type(resultq, search, PATH_EXEC);
}

void search_match_path(struct menu_q* menuq, struct menu_q* resultq, char* search)
{
	TAILQ_INIT(resultq);
	match_path_type(resultq, search, PATH_ANY);
}

void search_match_text(struct menu_q* menuq, struct menu_q* resultq, char* search)
{
	Menu* mi;

	TAILQ_INIT(resultq);
	TAILQ_FOREACH(mi, menuq, entry)
	{
		if (match_substr(search, mi->text)) TAILQ_INSERT_TAIL(resultq, mi, resultentry);
	}
}

void search_match_wm(struct menu_q* menuq, struct menu_q* resultq, char* search)
{
	Menu* mi;
	Cmd_ctx* wm;

	TAILQ_INIT(resultq);
	TAILQ_FOREACH(mi, menuq, entry)
	{
		wm = (Cmd_ctx*)mi->ctx;
		if ((match_substr(search, wm->name)) || (match_substr(search, wm->path)))
			TAILQ_INSERT_TAIL(resultq, mi, resultentry);
	}
}

void search_print_client(Menu* mi, int listing)
{
	Client_ctx* cc = (Client_ctx*)mi->ctx;
	char flag = ' ';

	if (cc->flags & CLIENT_ACTIVE)
		flag = '!';
	else if (cc->flags & CLIENT_HIDDEN)
		flag = '&';

	(void)snprintf(mi->print,
	               sizeof(mi->print),
	               "(%d) %c[%s] %s",
	               (cc->gc) ? cc->gc->num : 0,
	               flag,
	               (cc->label) ? cc->label : "",
	               cc->name);
}

void search_print_cmd(Menu* mi, int listing)
{
	Cmd_ctx* cmd = (Cmd_ctx*)mi->ctx;

	snprintf(mi->print, sizeof(mi->print), "%s", cmd->name);
}

void search_print_group(Menu* mi, int listing)
{
	Group_ctx* gc = (Group_ctx*)mi->ctx;

	(void)snprintf(mi->print,
	               sizeof(mi->print),
	               (group_holds_only_hidden(gc)) ? "%d: [%s]" : "%d: %s",
	               gc->num,
	               gc->name);
}

void search_print_text(Menu* mi, int listing)
{
	snprintf(mi->print, sizeof(mi->print), "%s", mi->text);
}

void search_print_wm(Menu* mi, int listing)
{
	Cmd_ctx* wm = (Cmd_ctx*)mi->ctx;

	snprintf(mi->print, sizeof(mi->print), "%s [%s]", wm->name, wm->path);
}
