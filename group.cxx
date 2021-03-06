/*
 * calmwm - the calm window manager
 *
 * Copyright (c) 2004 Andy Adamson <dros@monkey.org>
 * Copyright (c) 2004,2005 Marius Aamodt Eriksen <marius@monkey.org>
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

#include <algorithm>
#include <cerrno>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <err.h>
#include <sys/types.h>
#include <unistd.h>

namespace {
void group_restack(Group_ctx* gc)
{
	Screen_ctx* sc = gc->sc;
	Client_ctx* cc;
	Window* winlist;
	int i, lastempty = -1;
	int nwins = 0, highstack = 0;

	TAILQ_FOREACH(cc, &sc->clientq, entry)
	{
		if (cc->gc != gc) continue;
		if (cc->stackingorder > highstack) highstack = cc->stackingorder;
	}
	winlist = (Window*)reallocarray(nullptr, (highstack + 1), sizeof(*winlist));

	/* Invert the stacking order for XRestackWindows(). */
	TAILQ_FOREACH(cc, &sc->clientq, entry)
	{
		if (cc->gc != gc) continue;
		winlist[highstack - cc->stackingorder] = cc->win;
		nwins++;
	}

	/* Un-sparseify */
	for (i = 0; i <= highstack; i++) {
		if (!winlist[i] && lastempty == -1)
			lastempty = i;
		else if (winlist[i] && lastempty != -1) {
			winlist[lastempty] = winlist[i];
			if (++lastempty == i) lastempty = -1;
		}
	}

	XRestackWindows(X_Dpy, winlist, nwins);
	std::free(winlist);
}

void group_set_active(Group_ctx* gc)
{
	Screen_ctx* sc = gc->sc;
	sc->group_active = gc;
	xu_ewmh_net_current_desktop(sc);
}

Group_ctx* group_next(Group_ctx* gc)
{
	Screen_ctx* sc = gc->sc;
	Group_ctx* newgc;

	return (((newgc = TAILQ_NEXT(gc, entry)) != nullptr) ? newgc : TAILQ_FIRST(&sc->groupq));
}

Group_ctx* group_prev(Group_ctx* gc)
{
	Screen_ctx* sc = gc->sc;
	Group_ctx* newgc;

	return (((newgc = TAILQ_PREV(gc, group_q, entry)) != nullptr)
	            ? newgc
	            : TAILQ_LAST(&sc->groupq, group_q));
}
}

void group_assign(Group_ctx* gc, Client_ctx* cc)
{
	if ((gc != nullptr) && (gc->num == 0)) gc = nullptr;

	cc->gc = gc;

	xu_ewmh_set_net_wm_desktop(cc);
}

void group_hide(Group_ctx* gc)
{
	Screen_ctx* sc = gc->sc;
	Client_ctx* cc;

	screen_updatestackingorder(gc->sc);

	TAILQ_FOREACH(cc, &sc->clientq, entry)
	{
		if (cc->gc != gc) continue;
		if (!(cc->flags & Client_ctx::sticky) && !(cc->flags & Client_ctx::hidden)) client_hide(cc);
	}
}

void group_show(Group_ctx* gc)
{
	Screen_ctx* sc = gc->sc;
	Client_ctx* cc;

	TAILQ_FOREACH(cc, &sc->clientq, entry)
	{
		if (cc->gc != gc) continue;
		if (!(cc->flags & Client_ctx::sticky) && (cc->flags & Client_ctx::hidden)) client_show(cc);
	}
	group_restack(gc);
	group_set_active(gc);
}

void group_init(Screen_ctx* sc, int num, char const* name)
{
	Group_ctx* gc;

	gc = (Group_ctx*)std::malloc(sizeof(*gc));
	gc->sc = sc;
	gc->name = strdup(name);
	gc->num = num;
	TAILQ_INSERT_TAIL(&sc->groupq, gc, entry);

	if (num == 1) group_set_active(gc);
}

void group_movetogroup(Client_ctx* cc, int idx)
{
	Screen_ctx* sc = cc->sc;
	Group_ctx* gc;

	TAILQ_FOREACH(gc, &sc->groupq, entry)
	{
		if (gc->num == idx) {
			if (cc->gc == gc) return;
			if (gc->num != 0 && group_holds_only_hidden(gc)) client_hide(cc);
			group_assign(gc, cc);
		}
	}
}

void group_toggle_membership(Client_ctx* cc)
{
	Screen_ctx* sc = cc->sc;
	Group_ctx* gc = sc->group_active;

	if (cc->gc == gc) {
		group_assign(nullptr, cc);
		cc->flags |= Client_ctx::ungroup;
	} else {
		group_assign(gc, cc);
		cc->flags |= Client_ctx::group;
	}
	client_draw_border(cc);
}

int group_holds_only_sticky(Group_ctx* gc)
{
	Screen_ctx* sc = gc->sc;
	Client_ctx* cc;

	TAILQ_FOREACH(cc, &sc->clientq, entry)
	{
		if (cc->gc != gc) continue;
		if (!(cc->flags & Client_ctx::sticky)) return 0;
	}
	return 1;
}

int group_holds_only_hidden(Group_ctx* gc)
{
	Screen_ctx* sc = gc->sc;
	Client_ctx* cc;

	TAILQ_FOREACH(cc, &sc->clientq, entry)
	{
		if (cc->gc != gc) continue;
		if (!(cc->flags & (Client_ctx::hidden | Client_ctx::sticky))) return 0;
	}
	return 1;
}

void group_only(Screen_ctx* sc, int idx)
{
	Group_ctx* gc;

	TAILQ_FOREACH(gc, &sc->groupq, entry)
	{
		if (gc->num == idx)
			group_show(gc);
		else
			group_hide(gc);
	}
}

void group_toggle(Screen_ctx* sc, int idx)
{
	Group_ctx* gc;

	TAILQ_FOREACH(gc, &sc->groupq, entry)
	{
		if (gc->num == idx) {
			if (group_holds_only_hidden(gc))
				group_show(gc);
			else
				group_hide(gc);
		}
	}
}

void group_toggle_all(Screen_ctx* sc)
{
	Group_ctx* gc;

	TAILQ_FOREACH(gc, &sc->groupq, entry)
	{
		if (sc->hideall)
			group_show(gc);
		else
			group_hide(gc);
	}
	sc->hideall = !sc->hideall;
}

void group_close(Screen_ctx* sc, int idx)
{
	Group_ctx* gc;
	Client_ctx* cc;

	TAILQ_FOREACH(gc, &sc->groupq, entry)
	{
		if (gc->num == idx) {
			TAILQ_FOREACH(cc, &sc->clientq, entry)
			{
				if (cc->gc != gc) continue;
				cc->close();
			}
		}
	}
}

void group_cycle(Screen_ctx* sc, int flags)
{
	Group_ctx *newgc, *oldgc, *showgroup = nullptr;

	oldgc = sc->group_active;

	newgc = oldgc;
	for (;;) {
		newgc = (flags & CWM_CYCLE_REVERSE) ? group_prev(newgc) : group_next(newgc);

		if (newgc == oldgc) break;

		if (!group_holds_only_sticky(newgc) && showgroup == nullptr)
			showgroup = newgc;
		else if (!group_holds_only_hidden(newgc))
			group_hide(newgc);
	}
	if (showgroup == nullptr) return;

	group_hide(oldgc);

	if (group_holds_only_hidden(showgroup))
		group_show(showgroup);
	else
		group_set_active(showgroup);
}

int group_restore(Client_ctx* cc)
{
	Screen_ctx* sc = cc->sc;
	Group_ctx* gc;
	int num;
	long grpnum;

	if (!xu_ewmh_get_net_wm_desktop(cc, &grpnum)) return 0;

	num = (grpnum == -1) ? 0 : grpnum;
	num = std::min(num, (conf->ngroups - 1));

	TAILQ_FOREACH(gc, &sc->groupq, entry)
	{
		if (gc->num == num) {
			group_assign(gc, cc);
			return 1;
		}
	}
	return 0;
}

int group_autogroup(Client_ctx* cc)
{
	Screen_ctx* sc = cc->sc;
	Autogroup* ag;
	Group_ctx* gc;
	int num = -1, both_match = 0;

	if (cc->res_class == nullptr || cc->res_name == nullptr) return 0;

	TAILQ_FOREACH(ag, &conf->autogroupq, entry)
	{
		if (strcmp(ag->wclass, cc->res_class) == 0) {
			if ((ag->name != nullptr) && (strcmp(ag->name, cc->res_name) == 0)) {
				num = ag->num;
				both_match = 1;
			} else if (ag->name == nullptr && !both_match)
				num = ag->num;
		}
	}

	TAILQ_FOREACH(gc, &sc->groupq, entry)
	{
		if (gc->num == num) {
			group_assign(gc, cc);
			return 1;
		}
	}
	return 0;
}
