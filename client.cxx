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

#include <algorithm>
#include <cerrno>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <err.h>
#include <sys/types.h>
#include <unistd.h>

static void client_class_hint(Client_ctx*);
static void client_placement(Client_ctx*);
static void client_mwm_hints(Client_ctx*);
static void client_wm_protocols(Client_ctx*);

Client_ctx* client_init(Window win, Screen_ctx* sc)
{
	Client_ctx* cc;
	XWindowAttributes wattr;
	int mapped;
	long state;

	if (win == None) return nullptr;
	if (!XGetWindowAttributes(X_Dpy, win, &wattr)) return nullptr;

	if (sc == nullptr) {
		if ((sc = screen_find(wattr.root)) == nullptr) return nullptr;
		mapped = 1;
	} else {
		if (wattr.override_redirect || wattr.map_state != IsViewable) return nullptr;
		mapped = wattr.map_state != IsUnmapped;
	}

	XGrabServer(X_Dpy);

	cc = (Client_ctx*)xmalloc(sizeof(*cc));
	cc->sc = sc;
	cc->win = win;
	cc->name = nullptr;
	cc->label = nullptr;
	cc->gc = nullptr;
	cc->res_class = nullptr;
	cc->res_name = nullptr;
	cc->flags = 0;
	cc->stackingorder = 0;
	cc->initial_state = 0;
	memset(&cc->hint, 0, sizeof(cc->hint));
	TAILQ_INIT(&cc->nameq);

	cc->geom.x = wattr.x;
	cc->geom.y = wattr.y;
	cc->geom.w = wattr.width;
	cc->geom.h = wattr.height;
	cc->colormap = wattr.colormap;
	cc->obwidth = wattr.border_width;
	cc->bwidth = conf.bwidth;

	client_set_name(cc);
	conf_client(cc);

	client_wm_hints(cc);
	client_class_hint(cc);
	client_wm_protocols(cc);
	client_get_sizehints(cc);
	client_transient(cc);
	client_mwm_hints(cc);

	if ((cc->flags & CLIENT_IGNORE)) cc->bwidth = 0;
	cc->dim.w = (cc->geom.w - cc->hint.basew) / cc->hint.incw;
	cc->dim.h = (cc->geom.h - cc->hint.baseh) / cc->hint.inch;
	cc->ptr.x = cc->geom.w / 2;
	cc->ptr.y = cc->geom.h / 2;

	if (wattr.map_state != IsViewable) {
		client_placement(cc);
		client_resize(cc, 0);
		if (cc->initial_state) xu_set_wm_state(cc->win, cc->initial_state);
	}

	XSelectInput(X_Dpy, cc->win, EnterWindowMask | PropertyChangeMask | KeyReleaseMask);

	XAddToSaveSet(X_Dpy, cc->win);

	/* Notify client of its configuration. */
	client_config(cc);

	TAILQ_INSERT_TAIL(&sc->clientq, cc, entry);

	xu_ewmh_net_client_list(sc);
	xu_ewmh_net_client_list_stacking(sc);
	xu_ewmh_restore_net_wm_state(cc);

	xu_get_wm_state(cc->win, &state);
	if (state == IconicState)
		client_hide(cc);
	else
		client_show(cc);

	if (mapped) {
		if (cc->gc) {
			group_movetogroup(cc, cc->gc->num);
			goto out;
		}
		if (group_restore(cc)) goto out;
		if (group_autogroup(cc)) goto out;
		if (conf.stickygroups)
			group_assign(sc->group_active, cc);
		else
			group_assign(nullptr, cc);
	}
out:
	XSync(X_Dpy, False);
	XUngrabServer(X_Dpy);

	return cc;
}

Client_ctx* client_current(Screen_ctx* sc)
{
	Screen_ctx* _sc;
	Client_ctx* cc;

	if (sc) {
		TAILQ_FOREACH(cc, &sc->clientq, entry)
		{
			if (cc->flags & CLIENT_ACTIVE) return cc;
		}
	} else {
		TAILQ_FOREACH(_sc, &Screenq, entry)
		{
			TAILQ_FOREACH(cc, &_sc->clientq, entry)
			{
				if (cc->flags & CLIENT_ACTIVE) return cc;
			}
		}
	}
	return nullptr;
}

Client_ctx* client_find(Window win)
{
	Screen_ctx* sc;
	Client_ctx* cc;

	TAILQ_FOREACH(sc, &Screenq, entry)
	{
		TAILQ_FOREACH(cc, &sc->clientq, entry)
		{
			if (cc->win == win) return cc;
		}
	}
	return nullptr;
}

Client_ctx* client_next(Client_ctx* cc)
{
	Screen_ctx* sc = cc->sc;
	Client_ctx* newcc;

	return (((newcc = TAILQ_NEXT(cc, entry)) != nullptr) ? newcc : TAILQ_FIRST(&sc->clientq));
}

Client_ctx* client_prev(Client_ctx* cc)
{
	Screen_ctx* sc = cc->sc;
	Client_ctx* newcc;

	return (((newcc = TAILQ_PREV(cc, client_q, entry)) != nullptr)
	            ? newcc
	            : TAILQ_LAST(&sc->clientq, client_q));
}

void client_remove(Client_ctx* cc)
{
	Screen_ctx* sc = cc->sc;
	Winname* wn;

	TAILQ_REMOVE(&sc->clientq, cc, entry);

	xu_ewmh_net_client_list(sc);
	xu_ewmh_net_client_list_stacking(sc);

	if (cc->flags & CLIENT_ACTIVE) xu_ewmh_net_active_window(sc, None);

	while ((wn = TAILQ_FIRST(&cc->nameq)) != nullptr) {
		TAILQ_REMOVE(&cc->nameq, wn, entry);
		free(wn->name);
		free(wn);
	}

	free(cc->name);
	free(cc->label);
	free(cc->res_class);
	free(cc->res_name);
	free(cc);
}

void client_set_active(Client_ctx* cc)
{
	Screen_ctx* sc = cc->sc;
	Client_ctx* oldcc;

	if (cc->flags & CLIENT_HIDDEN) return;

	XInstallColormap(X_Dpy, cc->colormap);

	if ((cc->flags & CLIENT_INPUT) || (!(cc->flags & CLIENT_WM_TAKE_FOCUS))) {
		XSetInputFocus(X_Dpy, cc->win, RevertToPointerRoot, CurrentTime);
	}
	if (cc->flags & CLIENT_WM_TAKE_FOCUS)
		xu_send_clientmsg(cc->win, cwmh[WM_TAKE_FOCUS], Last_Event_Time);

	if ((oldcc = client_current(sc)) != nullptr) {
		oldcc->flags &= ~CLIENT_ACTIVE;
		client_draw_border(oldcc);
	}

	/* If we're in the middle of cycling, don't change the order. */
	if (!sc->cycling) client_mtf(cc);

	cc->flags |= CLIENT_ACTIVE;
	cc->flags &= ~CLIENT_URGENCY;
	client_draw_border(cc);
	conf_grab_mouse(cc->win);
	xu_ewmh_net_active_window(sc, cc->win);
}

void client_toggle_freeze(Client_ctx* cc)
{
	if (cc->flags & CLIENT_FULLSCREEN) return;

	cc->flags ^= CLIENT_FREEZE;
	xu_ewmh_set_net_wm_state(cc);
}

void client_toggle_hidden(Client_ctx* cc)
{
	cc->flags ^= CLIENT_HIDDEN;
	xu_ewmh_set_net_wm_state(cc);
}

void client_toggle_skip_pager(Client_ctx* cc)
{
	cc->flags ^= CLIENT_SKIP_PAGER;
	xu_ewmh_set_net_wm_state(cc);
}

void client_toggle_skip_taskbar(Client_ctx* cc)
{
	cc->flags ^= CLIENT_SKIP_TASKBAR;
	xu_ewmh_set_net_wm_state(cc);
}

void client_toggle_sticky(Client_ctx* cc)
{
	cc->flags ^= CLIENT_STICKY;
	xu_ewmh_set_net_wm_state(cc);
}

void client_toggle_fullscreen(Client_ctx* cc)
{
	Screen_ctx* sc = cc->sc;
	Geom area;

	if ((cc->flags & CLIENT_FREEZE) && !(cc->flags & CLIENT_FULLSCREEN)) return;

	if (cc->flags & CLIENT_FULLSCREEN) {
		if (!(cc->flags & CLIENT_IGNORE)) cc->bwidth = conf.bwidth;
		cc->geom = cc->fullgeom;
		cc->flags &= ~(CLIENT_FULLSCREEN | CLIENT_FREEZE);
		goto resize;
	}

	cc->fullgeom = cc->geom;

	area = screen_area(sc, cc->geom.x + cc->geom.w / 2, cc->geom.y + cc->geom.h / 2, 0);

	cc->bwidth = 0;
	cc->geom = area;
	cc->flags |= (CLIENT_FULLSCREEN | CLIENT_FREEZE);

resize:
	client_resize(cc, 0);
	xu_ewmh_set_net_wm_state(cc);
}

void client_toggle_maximize(Client_ctx* cc)
{
	Screen_ctx* sc = cc->sc;
	Geom area;

	if (cc->flags & CLIENT_FREEZE) return;

	if ((cc->flags & CLIENT_MAXFLAGS) == CLIENT_MAXIMIZED) {
		cc->geom = cc->savegeom;
		cc->flags &= ~CLIENT_MAXIMIZED;
		goto resize;
	}

	if (!(cc->flags & CLIENT_VMAXIMIZED)) {
		cc->savegeom.h = cc->geom.h;
		cc->savegeom.y = cc->geom.y;
	}

	if (!(cc->flags & CLIENT_HMAXIMIZED)) {
		cc->savegeom.w = cc->geom.w;
		cc->savegeom.x = cc->geom.x;
	}

	area = screen_area(sc, cc->geom.x + cc->geom.w / 2, cc->geom.y + cc->geom.h / 2, 1);

	cc->geom.x = area.x;
	cc->geom.y = area.y;
	cc->geom.w = area.w - (cc->bwidth * 2);
	cc->geom.h = area.h - (cc->bwidth * 2);
	cc->flags |= CLIENT_MAXIMIZED;

resize:
	client_resize(cc, 0);
	xu_ewmh_set_net_wm_state(cc);
}

void client_toggle_vmaximize(Client_ctx* cc)
{
	Screen_ctx* sc = cc->sc;
	Geom area;

	if (cc->flags & CLIENT_FREEZE) return;

	if (cc->flags & CLIENT_VMAXIMIZED) {
		cc->geom.y = cc->savegeom.y;
		cc->geom.h = cc->savegeom.h;
		cc->flags &= ~CLIENT_VMAXIMIZED;
		goto resize;
	}

	cc->savegeom.y = cc->geom.y;
	cc->savegeom.h = cc->geom.h;

	area = screen_area(sc, cc->geom.x + cc->geom.w / 2, cc->geom.y + cc->geom.h / 2, 1);

	cc->geom.y = area.y;
	cc->geom.h = area.h - (cc->bwidth * 2);
	cc->flags |= CLIENT_VMAXIMIZED;

resize:
	client_resize(cc, 0);
	xu_ewmh_set_net_wm_state(cc);
}

void client_toggle_hmaximize(Client_ctx* cc)
{
	Screen_ctx* sc = cc->sc;
	Geom area;

	if (cc->flags & CLIENT_FREEZE) return;

	if (cc->flags & CLIENT_HMAXIMIZED) {
		cc->geom.x = cc->savegeom.x;
		cc->geom.w = cc->savegeom.w;
		cc->flags &= ~CLIENT_HMAXIMIZED;
		goto resize;
	}

	cc->savegeom.x = cc->geom.x;
	cc->savegeom.w = cc->geom.w;

	area = screen_area(sc, cc->geom.x + cc->geom.w / 2, cc->geom.y + cc->geom.h / 2, 1);

	cc->geom.x = area.x;
	cc->geom.w = area.w - (cc->bwidth * 2);
	cc->flags |= CLIENT_HMAXIMIZED;

resize:
	client_resize(cc, 0);
	xu_ewmh_set_net_wm_state(cc);
}

void client_resize(Client_ctx* cc, int reset)
{
	if (reset) {
		cc->flags &= ~CLIENT_MAXIMIZED;
		xu_ewmh_set_net_wm_state(cc);
	}

	client_draw_border(cc);

	XMoveResizeWindow(X_Dpy, cc->win, cc->geom.x, cc->geom.y, cc->geom.w, cc->geom.h);
	cc->dim.w = (cc->geom.w - cc->hint.basew) / cc->hint.incw;
	cc->dim.h = (cc->geom.h - cc->hint.baseh) / cc->hint.inch;
	client_config(cc);
}

void client_move(Client_ctx* cc)
{
	XMoveWindow(X_Dpy, cc->win, cc->geom.x, cc->geom.y);
	client_config(cc);
}

void client_lower(Client_ctx* cc)
{
	XLowerWindow(X_Dpy, cc->win);
}

void client_raise(Client_ctx* cc)
{
	XRaiseWindow(X_Dpy, cc->win);
}

void client_config(Client_ctx* cc)
{
	XConfigureEvent cn;

	(void)memset(&cn, 0, sizeof(cn));
	cn.type = ConfigureNotify;
	cn.event = cc->win;
	cn.window = cc->win;
	cn.x = cc->geom.x;
	cn.y = cc->geom.y;
	cn.width = cc->geom.w;
	cn.height = cc->geom.h;
	cn.border_width = cc->bwidth;
	cn.above = None;
	cn.override_redirect = 0;

	XSendEvent(X_Dpy, cc->win, False, StructureNotifyMask, (XEvent*)&cn);
}

void client_ptr_inbound(Client_ctx* cc, int getpos)
{
	if (getpos) xu_ptr_get(cc->win, &cc->ptr.x, &cc->ptr.y);

	if (cc->ptr.x < 0)
		cc->ptr.x = 0;
	else if (cc->ptr.x > cc->geom.w - 1)
		cc->ptr.x = cc->geom.w - 1;
	if (cc->ptr.y < 0)
		cc->ptr.y = 0;
	else if (cc->ptr.y > cc->geom.h - 1)
		cc->ptr.y = cc->geom.h - 1;

	client_ptr_warp(cc);
}

void client_ptr_warp(Client_ctx* cc)
{
	xu_ptr_set(cc->win, cc->ptr.x, cc->ptr.y);
}

void client_ptr_save(Client_ctx* cc)
{
	int x, y;

	xu_ptr_get(cc->win, &x, &y);
	if (client_inbound(cc, x, y)) {
		cc->ptr.x = x;
		cc->ptr.y = y;
	} else {
		cc->ptr.x = cc->geom.w / 2;
		cc->ptr.y = cc->geom.h / 2;
	}
}

void client_hide(Client_ctx* cc)
{
	XUnmapWindow(X_Dpy, cc->win);

	if (cc->flags & CLIENT_ACTIVE) {
		cc->flags &= ~CLIENT_ACTIVE;
		xu_ewmh_net_active_window(cc->sc, None);
	}
	cc->flags |= CLIENT_HIDDEN;
	xu_set_wm_state(cc->win, IconicState);
}

void client_show(Client_ctx* cc)
{
	XMapRaised(X_Dpy, cc->win);

	cc->flags &= ~CLIENT_HIDDEN;
	xu_set_wm_state(cc->win, NormalState);
	client_draw_border(cc);
}

void client_urgency(Client_ctx* cc)
{
	if (!(cc->flags & CLIENT_ACTIVE)) cc->flags |= CLIENT_URGENCY;
}

void client_draw_border(Client_ctx* cc)
{
	Screen_ctx* sc = cc->sc;
	unsigned long pixel;

	if (cc->flags & CLIENT_ACTIVE) switch (cc->flags & CLIENT_HIGHLIGHT) {
		case CLIENT_GROUP: pixel = sc->xftcolor[CWM_COLOR_BORDER_GROUP].pixel; break;
		case CLIENT_UNGROUP: pixel = sc->xftcolor[CWM_COLOR_BORDER_UNGROUP].pixel; break;
		default: pixel = sc->xftcolor[CWM_COLOR_BORDER_ACTIVE].pixel; break;
		}
	else
		pixel = sc->xftcolor[CWM_COLOR_BORDER_INACTIVE].pixel;

	if (cc->flags & CLIENT_URGENCY) pixel = sc->xftcolor[CWM_COLOR_BORDER_URGENCY].pixel;

	XSetWindowBorderWidth(X_Dpy, cc->win, (unsigned int)cc->bwidth);
	XSetWindowBorder(X_Dpy, cc->win, pixel);
}

static void client_class_hint(Client_ctx* cc)
{
	XClassHint ch;

	if (XGetClassHint(X_Dpy, cc->win, &ch)) {
		if (ch.res_class) {
			cc->res_class = xstrdup(ch.res_class);
			XFree(ch.res_class);
		}
		if (ch.res_name) {
			cc->res_name = xstrdup(ch.res_name);
			XFree(ch.res_name);
		}
	}
}

static void client_wm_protocols(Client_ctx* cc)
{
	Atom* p;
	int j;

	if (XGetWMProtocols(X_Dpy, cc->win, &p, &j)) {
		for (int i {0}; i < j; ++i) {
			if (p[i] == cwmh[WM_DELETE_WINDOW])
				cc->flags |= CLIENT_WM_DELETE_WINDOW;
			else if (p[i] == cwmh[WM_TAKE_FOCUS])
				cc->flags |= CLIENT_WM_TAKE_FOCUS;
		}
		XFree(p);
	}
}

void client_wm_hints(Client_ctx* cc)
{
	XWMHints* wmh;

	if ((wmh = XGetWMHints(X_Dpy, cc->win)) != nullptr) {
		if ((wmh->flags & InputHint) && (wmh->input)) cc->flags |= CLIENT_INPUT;
		if ((wmh->flags & XUrgencyHint)) client_urgency(cc);
		if ((wmh->flags & StateHint)) cc->initial_state = wmh->initial_state;
		XFree(wmh);
	}
}

void Client_ctx::close() const noexcept
{
	if (flags & CLIENT_WM_DELETE_WINDOW)
		xu_send_clientmsg(win, cwmh[WM_DELETE_WINDOW], CurrentTime);
	else
		XKillClient(X_Dpy, win);
}

void client_set_name(Client_ctx* cc)
{
	Winname *wn, *wnnxt;
	int i = 0;

	free(cc->name);
	if (!xu_get_strprop(cc->win, ewmh[_NET_WM_NAME], &cc->name))
		if (!xu_get_strprop(cc->win, XA_WM_NAME, &cc->name)) cc->name = xstrdup("");

	TAILQ_FOREACH_SAFE(wn, &cc->nameq, entry, wnnxt)
	{
		if (strcmp(wn->name, cc->name) == 0) {
			TAILQ_REMOVE(&cc->nameq, wn, entry);
			free(wn->name);
			free(wn);
		}
		i++;
	}
	wn = (Winname*)xmalloc(sizeof(*wn));
	wn->name = xstrdup(cc->name);
	TAILQ_INSERT_TAIL(&cc->nameq, wn, entry);

	/* Garbage collection. */
	if ((i + 1) > conf.nameqlen) {
		wn = TAILQ_FIRST(&cc->nameq);
		TAILQ_REMOVE(&cc->nameq, wn, entry);
		free(wn->name);
		free(wn);
	}
}

static void client_placement(Client_ctx* cc)
{
	Screen_ctx* sc = cc->sc;

	if (cc->hint.flags & (USPosition | PPosition)) {
		if (cc->geom.x >= sc->view.w) cc->geom.x = sc->view.w - cc->bwidth - 1;
		if (cc->geom.x + cc->geom.w + cc->bwidth <= 0) cc->geom.x = -(cc->geom.w + cc->bwidth - 1);
		if (cc->geom.y >= sc->view.h) cc->geom.x = sc->view.h - cc->bwidth - 1;
		if (cc->geom.y + cc->geom.h + cc->bwidth <= 0) cc->geom.y = -(cc->geom.h + cc->bwidth - 1);
		if (cc->flags & CLIENT_IGNORE) {
			if (((cc->obwidth * 2) + cc->geom.x + cc->geom.w) == sc->view.w)
				cc->geom.x += cc->obwidth * 2;
			if (((cc->obwidth * 2) + cc->geom.y + cc->geom.h) == sc->view.h)
				cc->geom.y += cc->obwidth * 2;
		}
	} else {
		Geom area;
		int xmouse, ymouse, xslack, yslack;

		xu_ptr_get(sc->rootwin, &xmouse, &ymouse);
		area = screen_area(sc, xmouse, ymouse, 1);

		xmouse = std::max(std::max(xmouse, area.x) - cc->geom.w / 2, area.x);
		ymouse = std::max(std::max(ymouse, area.y) - cc->geom.h / 2, area.y);

		xslack = area.x + area.w - cc->geom.w - cc->bwidth * 2;
		yslack = area.y + area.h - cc->geom.h - cc->bwidth * 2;

		if (xslack >= area.x) {
			cc->geom.x = std::max(std::min(xmouse, xslack), area.x);
		} else {
			cc->geom.x = area.x;
			cc->geom.w = area.x + area.w;
		}
		if (yslack >= area.y) {
			cc->geom.y = std::max(std::min(ymouse, yslack), area.y);
		} else {
			cc->geom.y = area.y;
			cc->geom.h = area.y + area.h;
		}
	}
}

void client_mtf(Client_ctx* cc)
{
	Screen_ctx* sc = cc->sc;

	TAILQ_REMOVE(&sc->clientq, cc, entry);
	TAILQ_INSERT_HEAD(&sc->clientq, cc, entry);
}

void client_get_sizehints(Client_ctx* cc)
{
	long tmp;
	XSizeHints size;

	if (!XGetWMNormalHints(X_Dpy, cc->win, &size, &tmp)) size.flags = 0;

	cc->hint.flags = size.flags;

	if (size.flags & PBaseSize) {
		cc->hint.basew = size.base_width;
		cc->hint.baseh = size.base_height;
	} else if (size.flags & PMinSize) {
		cc->hint.basew = size.min_width;
		cc->hint.baseh = size.min_height;
	}
	if (size.flags & PMinSize) {
		cc->hint.minw = size.min_width;
		cc->hint.minh = size.min_height;
	} else if (size.flags & PBaseSize) {
		cc->hint.minw = size.base_width;
		cc->hint.minh = size.base_height;
	}
	if (size.flags & PMaxSize) {
		cc->hint.maxw = size.max_width;
		cc->hint.maxh = size.max_height;
	}
	if (size.flags & PResizeInc) {
		cc->hint.incw = size.width_inc;
		cc->hint.inch = size.height_inc;
	}
	cc->hint.incw = std::max(1, cc->hint.incw);
	cc->hint.inch = std::max(1, cc->hint.inch);
	cc->hint.minw = std::max(1, cc->hint.minw);
	cc->hint.minh = std::max(1, cc->hint.minh);

	if (size.flags & PAspect) {
		if (size.min_aspect.x > 0)
			cc->hint.mina = static_cast<float>(size.min_aspect.y) / size.min_aspect.x;
		if (size.max_aspect.y > 0)
			cc->hint.maxa = static_cast<float>(size.max_aspect.x) / size.max_aspect.y;
	}
}

void Client_ctx::apply_sizehints() noexcept
{
	auto baseismin = (hint.basew == hint.minw) && (hint.baseh == hint.minh);

	/* temporarily remove base dimensions, ICCCM 4.1.2.3 */
	if (!baseismin) {
		geom.w -= hint.basew;
		geom.h -= hint.baseh;
	}

	/* adjust for aspect limits */
	if (hint.mina && hint.maxa) {
		if (hint.maxa < static_cast<float>(geom.w) / geom.h)
			geom.w = geom.h * hint.maxa;
		else if (hint.mina < static_cast<float>(geom.h) / geom.w)
			geom.h = geom.w * hint.mina;
	}

	/* remove base dimensions for increment */
	if (baseismin) {
		geom.w -= hint.basew;
		geom.h -= hint.baseh;
	}

	/* adjust for increment value */
	geom.w -= geom.w % hint.incw;
	geom.h -= geom.h % hint.inch;

	/* restore base dimensions */
	geom.w += hint.basew;
	geom.h += hint.baseh;

	/* adjust for min width/height */
	geom.w = std::max(geom.w, hint.minw);
	geom.h = std::max(geom.h, hint.minh);

	/* adjust for max width/height */
	if (hint.maxw) geom.w = std::min(geom.w, hint.maxw);
	if (hint.maxh) geom.h = std::min(geom.h, hint.maxh);
}

static void client_mwm_hints(Client_ctx* cc)
{
	Mwm_hints* mwmh;

	if (xu_get_prop(cc->win,
	                cwmh[_MOTIF_WM_HINTS],
	                cwmh[_MOTIF_WM_HINTS],
	                MWM_HINTS_ELEMENTS,
	                (unsigned char**)&mwmh)
	    == MWM_HINTS_ELEMENTS) {
		if (mwmh->flags & MWM_FLAGS_DECORATIONS && !(mwmh->decorations & MWM_DECOR_ALL)
		    && !(mwmh->decorations & MWM_DECOR_BORDER))
			cc->bwidth = 0;
		XFree(mwmh);
	}
}

void client_transient(Client_ctx* cc)
{
	Client_ctx* tc;
	Window trans;

	if (XGetTransientForHint(X_Dpy, cc->win, &trans)) {
		if ((tc = client_find(trans)) != nullptr) {
			if (tc->flags & CLIENT_IGNORE) {
				cc->flags |= CLIENT_IGNORE;
				cc->bwidth = tc->bwidth;
			}
		}
	}
}

int client_inbound(Client_ctx* cc, int x, int y)
{
	return (x < cc->geom.w && x >= 0 && y < cc->geom.h && y >= 0);
}

int client_snapcalc(int n0, int n1, int e0, int e1, int snapdist)
{
	int s0, s1;

	s0 = s1 = 0;

	if (abs(e0 - n0) <= snapdist) s0 = e0 - n0;

	if (abs(e1 - n1) <= snapdist) s1 = e1 - n1;

	/* possible to snap in both directions */
	if (s0 != 0 && s1 != 0)
		if (abs(s0) < abs(s1))
			return s0;
		else
			return s1;
	else if (s0 != 0)
		return s0;
	else if (s1 != 0)
		return s1;
	else
		return 0;
}

void client_htile(Client_ctx* cc)
{
	Client_ctx* ci;
	Screen_ctx* sc = cc->sc;
	Geom area;
	int i, n, mh, x, w, h;

	i = n = 0;
	area = screen_area(sc, cc->geom.x + cc->geom.w / 2, cc->geom.y + cc->geom.h / 2, 1);

	TAILQ_FOREACH(ci, &sc->clientq, entry)
	{
		if (ci->gc != cc->gc) continue;
		if (ci->flags & CLIENT_HIDDEN || ci->flags & CLIENT_IGNORE || (ci == cc)
		    || ci->geom.x < area.x || ci->geom.x > (area.x + area.w) || ci->geom.y < area.y
		    || ci->geom.y > (area.y + area.h))
			continue;
		n++;
	}
	if (n == 0) return;

	if (cc->flags & CLIENT_VMAXIMIZED || cc->geom.h + (cc->bwidth * 2) >= area.h) return;

	cc->flags &= ~CLIENT_HMAXIMIZED;
	cc->geom.x = area.x;
	cc->geom.y = area.y;
	cc->geom.w = area.w - (cc->bwidth * 2);
	if (conf.htile > 0) cc->geom.h = ((area.h - (cc->bwidth * 2)) * conf.htile) / 100;
	client_resize(cc, 1);
	client_ptr_warp(cc);

	mh = cc->geom.h + (cc->bwidth * 2);
	x = area.x;
	w = area.w / n;
	h = area.h - mh;
	TAILQ_FOREACH(ci, &sc->clientq, entry)
	{
		if (ci->gc != cc->gc) continue;
		if (ci->flags & CLIENT_HIDDEN || ci->flags & CLIENT_IGNORE || (ci == cc)
		    || ci->geom.x < area.x || ci->geom.x > (area.x + area.w) || ci->geom.y < area.y
		    || ci->geom.y > (area.y + area.h))
			continue;
		ci->bwidth = conf.bwidth;
		ci->geom.x = x;
		ci->geom.y = area.y + mh;
		ci->geom.w = w - (ci->bwidth * 2);
		ci->geom.h = h - (ci->bwidth * 2);
		if (i + 1 == n) ci->geom.w = area.x + area.w - ci->geom.x - (ci->bwidth * 2);
		x += w;
		i++;
		client_resize(ci, 1);
	}
}

void client_vtile(Client_ctx* cc)
{
	Client_ctx* ci;
	Screen_ctx* sc = cc->sc;
	Geom area;
	int i, n, mw, y, w, h;

	i = n = 0;
	area = screen_area(sc, cc->geom.x + cc->geom.w / 2, cc->geom.y + cc->geom.h / 2, 1);

	TAILQ_FOREACH(ci, &sc->clientq, entry)
	{
		if (ci->gc != cc->gc) continue;
		if (ci->flags & CLIENT_HIDDEN || ci->flags & CLIENT_IGNORE || (ci == cc)
		    || ci->geom.x < area.x || ci->geom.x > (area.x + area.w) || ci->geom.y < area.y
		    || ci->geom.y > (area.y + area.h))
			continue;
		n++;
	}
	if (n == 0) return;

	if (cc->flags & CLIENT_HMAXIMIZED || cc->geom.w + (cc->bwidth * 2) >= area.w) return;

	cc->flags &= ~CLIENT_VMAXIMIZED;
	cc->geom.x = area.x;
	cc->geom.y = area.y;
	if (conf.vtile > 0) cc->geom.w = ((area.w - (cc->bwidth * 2)) * conf.vtile) / 100;
	cc->geom.h = area.h - (cc->bwidth * 2);
	client_resize(cc, 1);
	client_ptr_warp(cc);

	mw = cc->geom.w + (cc->bwidth * 2);
	y = area.y;
	h = area.h / n;
	w = area.w - mw;
	TAILQ_FOREACH(ci, &sc->clientq, entry)
	{
		if (ci->gc != cc->gc) continue;
		if (ci->flags & CLIENT_HIDDEN || ci->flags & CLIENT_IGNORE || (ci == cc)
		    || ci->geom.x < area.x || ci->geom.x > (area.x + area.w) || ci->geom.y < area.y
		    || ci->geom.y > (area.y + area.h))
			continue;
		ci->bwidth = conf.bwidth;
		ci->geom.x = area.x + mw;
		ci->geom.y = y;
		ci->geom.w = w - (ci->bwidth * 2);
		ci->geom.h = h - (ci->bwidth * 2);
		if (i + 1 == n) ci->geom.h = area.y + area.h - ci->geom.y - (ci->bwidth * 2);
		y += h;
		i++;
		client_resize(ci, 1);
	}
}
