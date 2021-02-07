/*
 * calmwm - the calm window manager
 *
 * Copyright (c) 2008 Owain G. Ainsworth <oga@openbsd.org>
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
#include "config.hxx"
#include "queue.hxx"

#include <algorithm>
#include <cctype>
#include <cerrno>
#include <climits>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <err.h>
#include <sys/types.h>
#include <unistd.h>

static constexpr auto MENUMASK {MOUSEMASK | ButtonMotionMask | KeyPressMask | ExposureMask};
static constexpr auto MENUGRABMASK {MOUSEMASK | ButtonMotionMask | StructureNotifyMask};

enum class Ctltype { none = -1, eraseone = 0, wipe, up, down, ret, tab, abort, all };

struct Menu_ctx {
	Screen_ctx* sc;
	Window win;
	XftDraw* xftdraw;
	Geom geom;
	char searchstr[MENU_MAXENTRY + 1];
	char dispstr[MENU_MAXENTRY * 2 + 1];
	char promptstr[MENU_MAXENTRY + 1];
	int list;
	int listing;
	int changed;
	int prev;
	int entry;
	int num;
	int flags;
	void (*match)(struct menu_q*, struct menu_q*, char*);
	void (*print)(Menu*, int);
};

namespace {
Menu* menu_complete_path(Menu_ctx* mc)
{
	Screen_ctx* sc = mc->sc;
	Menu *mi, *mr;
	struct menu_q menuq;
	int mflags = (CWM_MENU_DUMMY);

	mr = (Menu*)calloc(1, sizeof(*mr));

	TAILQ_INIT(&menuq);

	if ((mi = menu_filter(sc,
	                      &menuq,
	                      mc->searchstr,
	                      nullptr,
	                      mflags,
	                      search_match_path,
	                      search_print_text))
	    != nullptr) {
		mr->abort = mi->abort;
		mr->dummy = mi->dummy;
		if (mi->text[0] != '\0')
			snprintf(mr->text, sizeof(mr->text), "%s \"%s\"", mc->searchstr, mi->text);
		else if (!mr->abort)
			strlcpy(mr->text, mc->searchstr, sizeof(mr->text));
	}

	menuq_clear(&menuq);

	return mr;
}

int menu_keycode(XKeyEvent* ev, Ctltype* ctl, char* chr)
{
	KeySym ks;

	*ctl = Ctltype::none;
	chr[0] = '\0';

	ks = XkbKeycodeToKeysym(X_Dpy, ev->keycode, 0, (ev->state & ShiftMask) ? 1 : 0);

	/* Look for control characters. */
	switch (ks) {
	case XK_BackSpace: *ctl = Ctltype::eraseone; break;
	case XK_KP_Enter:
	case XK_Return: *ctl = Ctltype::ret; break;
	case XK_Tab: *ctl = Ctltype::tab; break;
	case XK_Up: *ctl = Ctltype::up; break;
	case XK_Down: *ctl = Ctltype::down; break;
	case XK_Escape: *ctl = Ctltype::abort; break;
	}

	if (*ctl == Ctltype::none && (ev->state & ControlMask)) {
		switch (ks) {
		case XK_s:
		case XK_S:
			/* Emacs "next" */
			*ctl = Ctltype::down;
			break;
		case XK_r:
		case XK_R:
			/* Emacs "previous" */
			*ctl = Ctltype::up;
			break;
		case XK_u:
		case XK_U: *ctl = Ctltype::wipe; break;
		case XK_h:
		case XK_H: *ctl = Ctltype::eraseone; break;
		case XK_a:
		case XK_A: *ctl = Ctltype::all; break;
		case XK_bracketleft: *ctl = Ctltype::abort; break;
		}
	}

	if (*ctl == Ctltype::none && (ev->state & Mod1Mask)) {
		switch (ks) {
		case XK_j:
		case XK_J:
			/* Vi "down" */
			*ctl = Ctltype::down;
			break;
		case XK_k:
		case XK_K:
			/* Vi "up" */
			*ctl = Ctltype::up;
			break;
		}
	}

	if (*ctl != Ctltype::none) return 0;

	if (XLookupString(ev, chr, 32, &ks, nullptr) < 0) return -1;

	return 0;
}

Menu* menu_handle_key(XEvent* e, Menu_ctx* mc, struct menu_q* menuq, struct menu_q* resultq)
{
	Menu* mi;
	Ctltype ctl;
	char chr[32];
	size_t len;
	int clen, i;
	wchar_t wc;

	if (menu_keycode(&e->xkey, &ctl, chr) < 0) return nullptr;

	switch (ctl) {
	case Ctltype::eraseone:
		if ((len = strlen(mc->searchstr)) > 0) {
			clen = 1;
			while (mbtowc(&wc, &mc->searchstr[len - clen], MB_CUR_MAX) == -1) clen++;
			for (i = 1; i <= clen; i++) mc->searchstr[len - i] = '\0';
			mc->changed = 1;
		}
		break;
	case Ctltype::up:
		mi = TAILQ_LAST(resultq, menu_q);
		if (mi == nullptr) break;

		TAILQ_REMOVE(resultq, mi, resultentry);
		TAILQ_INSERT_HEAD(resultq, mi, resultentry);
		break;
	case Ctltype::down:
		mi = TAILQ_FIRST(resultq);
		if (mi == nullptr) break;

		TAILQ_REMOVE(resultq, mi, resultentry);
		TAILQ_INSERT_TAIL(resultq, mi, resultentry);
		break;
	case Ctltype::ret:
		/*
		 * Return whatever the cursor is currently on. Else
		 * even if dummy is zero, we need to return something.
		 */
		if ((mi = TAILQ_FIRST(resultq)) == nullptr) {
			mi = (Menu*)std::malloc(sizeof(*mi));
			(void)strlcpy(mi->text, mc->searchstr, sizeof(mi->text));
			mi->dummy = true;
		}
		mi->abort = false;
		return mi;
	case Ctltype::wipe:
		mc->searchstr[0] = '\0';
		mc->changed = 1;
		break;
	case Ctltype::tab:
		if ((mi = TAILQ_FIRST(resultq)) != nullptr) {
			/*
			 * - We are in exec_path menu mode
			 * - It is equal to the input
			 * We got a command, launch the file menu
			 */
			if ((mc->flags & CWM_MENU_FILE)
			    && (strncmp(mc->searchstr, mi->text, strlen(mi->text))) == 0)
				return menu_complete_path(mc);

			/*
			 * Put common prefix of the results into searchstr
			 */
			(void)strlcpy(mc->searchstr, mi->text, sizeof(mc->searchstr));
			while ((mi = TAILQ_NEXT(mi, resultentry)) != nullptr) {
				i = 0;
				while (tolower(mc->searchstr[i]) == tolower(mi->text[i])) i++;
				mc->searchstr[i] = '\0';
			}
			mc->changed = 1;
		}
		break;
	case Ctltype::all: mc->list = !mc->list; break;
	case Ctltype::abort:
		mi = (Menu*)std::malloc(sizeof(*mi));
		mi->text[0] = '\0';
		mi->dummy = true;
		mi->abort = true;
		return mi;
	default: break;
	}

	if (chr[0] != '\0') {
		mc->changed = 1;
		strlcat(mc->searchstr, chr, sizeof(mc->searchstr));
	}

	if (mc->changed) {
		if (mc->searchstr[0] != '\0') (*mc->match)(menuq, resultq, mc->searchstr);
	} else if (!mc->list && mc->listing) {
		TAILQ_INIT(resultq);
		mc->listing = 0;
	}

	return nullptr;
}

void menu_draw_entry(Menu_ctx* mc, struct menu_q* resultq, int entry, int active)
{
	Screen_ctx* sc = mc->sc;
	Menu* mi;
	int color, i = 1;

	TAILQ_FOREACH(mi, resultq, resultentry)
	if (entry == i++) break;
	if (mi == nullptr) return;

	color = (active) ? CWM_COLOR_MENU_FG : CWM_COLOR_MENU_BG;
	XftDrawRect(mc->xftdraw,
	            &sc->xftcolor[color],
	            0,
	            (sc->xftfont->height + 1) * entry,
	            mc->geom.w,
	            (sc->xftfont->height + 1) + sc->xftfont->descent);
	color = (active) ? CWM_COLOR_MENU_FONT_SEL : CWM_COLOR_MENU_FONT;
	XftDrawStringUtf8(mc->xftdraw,
	                  &sc->xftcolor[color],
	                  sc->xftfont,
	                  0,
	                  (sc->xftfont->height + 1) * entry + sc->xftfont->ascent + 1,
	                  (const FcChar8*)mi->print,
	                  strlen(mi->print));
}

int menu_calc_entry(Menu_ctx* mc, int x, int y)
{
	Screen_ctx* sc = mc->sc;
	int entry;

	entry = y / (sc->xftfont->height + 1);

	/* in bounds? */
	if (x < 0 || x > mc->geom.w || y < 0 || y > (sc->xftfont->height + 1) * mc->num || entry < 0
	    || entry >= mc->num)
		entry = -1;

	if (entry == 0) entry = -1;

	return entry;
}

void menu_handle_move(Menu_ctx* mc, struct menu_q* resultq, int x, int y)
{
	mc->prev = mc->entry;
	mc->entry = menu_calc_entry(mc, x, y);

	if (mc->prev == mc->entry) return;

	if (mc->prev != -1) menu_draw_entry(mc, resultq, mc->prev, 0);
	if (mc->entry != -1) {
		XChangeActivePointerGrab(X_Dpy, MENUGRABMASK, conf->cursor[CF_NORMAL], CurrentTime);
		menu_draw_entry(mc, resultq, mc->entry, 1);
	}
}

Menu* menu_handle_release(Menu_ctx* mc, struct menu_q* resultq, int x, int y)
{
	Menu* mi;
	int entry, i = 1;

	entry = menu_calc_entry(mc, x, y);

	TAILQ_FOREACH(mi, resultq, resultentry)
	if (entry == i++) break;
	if (mi == nullptr) {
		mi = (Menu*)std::malloc(sizeof(*mi));
		mi->text[0] = '\0';
		mi->dummy = true;
	}
	return mi;
}

void menu_draw(Menu_ctx* mc, struct menu_q* menuq, struct menu_q* resultq)
{
	Screen_ctx* sc = mc->sc;
	Menu* mi;
	Geom area;
	int n, xsave, ysave;
	XGlyphInfo extents;

	if (mc->list) {
		if (TAILQ_EMPTY(resultq)) {
			/* Copy them all over. */
			TAILQ_FOREACH(mi, menuq, entry)
			TAILQ_INSERT_TAIL(resultq, mi, resultentry);

			mc->listing = 1;
		} else if (mc->changed)
			mc->listing = 0;
	}

	snprintf(mc->dispstr,
	         sizeof(mc->dispstr),
	         "%s%s%s%s",
	         mc->promptstr,
	         prompt_schar,
	         mc->searchstr,
	         prompt_echar);
	XftTextExtentsUtf8(X_Dpy,
	                   sc->xftfont,
	                   (const FcChar8*)mc->dispstr,
	                   strlen(mc->dispstr),
	                   &extents);
	mc->geom.w = extents.xOff;
	mc->geom.h = sc->xftfont->height + 1;
	mc->num = 1;

	TAILQ_FOREACH(mi, resultq, resultentry)
	{
		(*mc->print)(mi, mc->listing);
		XftTextExtentsUtf8(X_Dpy,
		                   sc->xftfont,
		                   (const FcChar8*)mi->print,
		                   std::min(strlen(mi->print), MENU_MAXENTRY),
		                   &extents);
		mc->geom.w = std::max(mc->geom.w, static_cast<int>(extents.xOff));
		mc->geom.h += sc->xftfont->height + 1;
		mc->num++;
	}

	area = screen_area(sc, mc->geom.x, mc->geom.y, 1);
	area.w += area.x - conf->bwidth * 2;
	area.h += area.y - conf->bwidth * 2;

	xsave = mc->geom.x;
	ysave = mc->geom.y;

	/* Never hide the top, or left side, of the menu. */
	if (mc->geom.x + mc->geom.w >= area.w) mc->geom.x = area.w - mc->geom.w;
	if (mc->geom.x < area.x) {
		mc->geom.x = area.x;
		mc->geom.w = std::min(mc->geom.w, (area.w - area.x));
	}
	if (mc->geom.y + mc->geom.h >= area.h) mc->geom.y = area.h - mc->geom.h;
	if (mc->geom.y < area.y) {
		mc->geom.y = area.y;
		mc->geom.h = std::min(mc->geom.h, (area.h - area.y));
	}

	if (mc->geom.x != xsave || mc->geom.y != ysave) xu_ptr_set(sc->rootwin, mc->geom.x, mc->geom.y);

	XClearWindow(X_Dpy, mc->win);
	XMoveResizeWindow(X_Dpy, mc->win, mc->geom.x, mc->geom.y, mc->geom.w, mc->geom.h);

	n = 1;
	XftDrawStringUtf8(mc->xftdraw,
	                  &sc->xftcolor[CWM_COLOR_MENU_FONT],
	                  sc->xftfont,
	                  0,
	                  sc->xftfont->ascent,
	                  (const FcChar8*)mc->dispstr,
	                  strlen(mc->dispstr));

	TAILQ_FOREACH(mi, resultq, resultentry)
	{
		int y = n * (sc->xftfont->height + 1) + sc->xftfont->ascent + 1;

		/* Stop drawing when menu doesn't fit inside the screen. */
		if (mc->geom.y + y > area.h) break;

		XftDrawStringUtf8(mc->xftdraw,
		                  &sc->xftcolor[CWM_COLOR_MENU_FONT],
		                  sc->xftfont,
		                  0,
		                  y,
		                  (const FcChar8*)mi->print,
		                  strlen(mi->print));
		n++;
	}
	if (n > 1) menu_draw_entry(mc, resultq, 1, 1);
}
}

Menu* menu_filter(Screen_ctx* sc,
                  struct menu_q* menuq,
                  char const* prompt,
                  char const* initial,
                  int flags,
                  void (*match)(struct menu_q*, struct menu_q*, char*),
                  void (*print)(Menu*, int))
{
	Menu_ctx mc {};
	menu_q resultq {};
	Menu* mi = nullptr;
	XEvent e;
	Window focuswin;
	int focusrevert, xsave, ysave, xcur, ycur;

	TAILQ_INIT(&resultq);

	xu_ptr_get(sc->rootwin, &xsave, &ysave);

	(void)memset(&mc, 0, sizeof(mc));
	mc.sc = sc;
	mc.flags = flags;
	mc.match = match;
	mc.print = print;
	mc.entry = mc.prev = -1;
	mc.geom.x = xsave;
	mc.geom.y = ysave;

	if (mc.flags & CWM_MENU_LIST) mc.list = 1;

	(void)strlcpy(mc.promptstr, prompt, sizeof(mc.promptstr));
	if (initial != nullptr)
		(void)strlcpy(mc.searchstr, initial, sizeof(mc.searchstr));
	else
		mc.searchstr[0] = '\0';

	mc.win = XCreateSimpleWindow(X_Dpy,
	                             sc->rootwin,
	                             0,
	                             0,
	                             1,
	                             1,
	                             conf->bwidth,
	                             sc->xftcolor[CWM_COLOR_MENU_FG].pixel,
	                             sc->xftcolor[CWM_COLOR_MENU_BG].pixel);
	mc.xftdraw = XftDrawCreate(X_Dpy, mc.win, sc->visual, sc->colormap);

	XSelectInput(X_Dpy, mc.win, MENUMASK);
	XMapRaised(X_Dpy, mc.win);

	if (XGrabPointer(X_Dpy,
	                 mc.win,
	                 False,
	                 MENUGRABMASK,
	                 GrabModeAsync,
	                 GrabModeAsync,
	                 None,
	                 conf->cursor[CF_QUESTION],
	                 CurrentTime)
	    != GrabSuccess) {
		XftDrawDestroy(mc.xftdraw);
		XDestroyWindow(X_Dpy, mc.win);
		return nullptr;
	}

	XGetInputFocus(X_Dpy, &focuswin, &focusrevert);
	XSetInputFocus(X_Dpy, mc.win, RevertToPointerRoot, CurrentTime);

	/* make sure keybindings don't remove keys from the menu stream */
	XGrabKeyboard(X_Dpy, mc.win, True, GrabModeAsync, GrabModeAsync, CurrentTime);

	for (;;) {
		mc.changed = 0;

		XWindowEvent(X_Dpy, mc.win, MENUMASK, &e);

		switch (e.type) {
		case KeyPress:
			if ((mi = menu_handle_key(&e, &mc, menuq, &resultq)) != nullptr) goto out;
			[[fallthrough]];
		case Expose: menu_draw(&mc, menuq, &resultq); break;
		case MotionNotify: menu_handle_move(&mc, &resultq, e.xbutton.x, e.xbutton.y); break;
		case ButtonRelease:
			if ((mi = menu_handle_release(&mc, &resultq, e.xbutton.x, e.xbutton.y)) != nullptr)
				goto out;
			break;
		default: break;
		}
	}
out:
	if ((mc.flags & CWM_MENU_DUMMY) == 0 && mi->dummy) {
		/* no mouse based match */
		free(mi);
		mi = nullptr;
	}

	XftDrawDestroy(mc.xftdraw);
	XDestroyWindow(X_Dpy, mc.win);

	XSetInputFocus(X_Dpy, focuswin, focusrevert, CurrentTime);
	/* restore if user didn't move */
	xu_ptr_get(sc->rootwin, &xcur, &ycur);
	if (xcur == mc.geom.x && ycur == mc.geom.y) xu_ptr_set(sc->rootwin, xsave, ysave);

	XUngrabPointer(X_Dpy, CurrentTime);
	XUngrabKeyboard(X_Dpy, CurrentTime);

	return mi;
}

void menuq_add(struct menu_q* mq, void* ctx, char const* fmt, ...)
{
	va_list ap;
	Menu* mi;

	mi = (Menu*)calloc(1, sizeof(*mi));
	mi->ctx = ctx;

	va_start(ap, fmt);
	if (fmt != nullptr)
		(void)vsnprintf(mi->text, sizeof(mi->text), fmt, ap);
	else
		mi->text[0] = '\0';
	va_end(ap);

	TAILQ_INSERT_TAIL(mq, mi, entry);
}

void menuq_clear(struct menu_q* mq)
{
	Menu* mi;

	while ((mi = TAILQ_FIRST(mq)) != nullptr) {
		TAILQ_REMOVE(mq, mi, entry);
		free(mi);
	}
}
