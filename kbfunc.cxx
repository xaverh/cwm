/*
 * calmwm - the calm window manager
 *
 * Copyright (c) 2004 Martin Murray <mmurray@monkey.org>
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

/* For FreeBSD. */
#define _WITH_GETLINE

#include "calmwm.hxx"
#include "queue.hxx"

#include <atomic>
#include <cerrno>
#include <climits>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <dirent.h>
#include <err.h>
#include <paths.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define HASH_MARKER "|1|"

extern std::atomic<int> cwm_status;

static void kbfunc_amount(int, int, int*, int*);
static void kbfunc_client_move_kb(void*, Cargs*);
static void kbfunc_client_move_mb(void*, Cargs*);
static void kbfunc_client_resize_kb(void*, Cargs*);
static void kbfunc_client_resize_mb(void*, Cargs*);

void kbfunc_cwm_status([[maybe_unused]] void* ctx, Cargs* cargs)
{
	cwm_status = cargs->flag;
}

static void kbfunc_amount(int flags, int amt, int* mx, int* my)
{
	static constexpr auto CWM_FACTOR {10};
	if (flags & CWM_BIGAMOUNT) amt *= CWM_FACTOR;

	switch (flags & (CWM_UP | CWM_DOWN | CWM_LEFT | CWM_RIGHT)) {
	case CWM_UP: *my -= amt; break;
	case CWM_DOWN: *my += amt; break;
	case CWM_RIGHT: *mx += amt; break;
	case CWM_LEFT: *mx -= amt; break;
	}
}

void kbfunc_ptrmove(void* ctx, Cargs* cargs)
{
	auto sc = (Screen_ctx*)ctx;
	int x, y;
	int mx = 0, my = 0;

	kbfunc_amount(cargs->flag, conf->mamount, &mx, &my);

	xu_ptr_get(sc->rootwin, &x, &y);
	xu_ptr_set(sc->rootwin, x + mx, y + my);
}

void kbfunc_client_move(void* ctx, Cargs* cargs)
{
	if (cargs->xev == cwm::Xev::btn)
		kbfunc_client_move_mb(ctx, cargs);
	else
		kbfunc_client_move_kb(ctx, cargs);
}

void kbfunc_client_resize(void* ctx, Cargs* cargs)
{
	if (cargs->xev == cwm::Xev::btn)
		kbfunc_client_resize_mb(ctx, cargs);
	else
		kbfunc_client_resize_kb(ctx, cargs);
}

static void kbfunc_client_move_kb(void* ctx, Cargs* cargs)
{
	auto cc = (Client_ctx*)ctx;
	Screen_ctx* sc = cc->sc;
	Geom area;
	int mx = 0, my = 0;

	if (cc->flags & Client_ctx::freeze) return;

	kbfunc_amount(cargs->flag, conf->mamount, &mx, &my);

	cc->geom.x += mx;
	if (cc->geom.x < -(cc->geom.w + cc->bwidth - 1)) cc->geom.x = -(cc->geom.w + cc->bwidth - 1);
	if (cc->geom.x > (sc->view.w - cc->bwidth - 1)) cc->geom.x = sc->view.w - cc->bwidth - 1;
	cc->geom.y += my;
	if (cc->geom.y < -(cc->geom.h + cc->bwidth - 1)) cc->geom.y = -(cc->geom.h + cc->bwidth - 1);
	if (cc->geom.y > (sc->view.h - cc->bwidth - 1)) cc->geom.y = sc->view.h - cc->bwidth - 1;

	area = screen_area(sc, cc->geom.x + cc->geom.w / 2, cc->geom.y + cc->geom.h / 2, 1);
	cc->geom.x += client_snapcalc(cc->geom.x,
	                              cc->geom.x + cc->geom.w + (cc->bwidth * 2),
	                              area.x,
	                              area.x + area.w,
	                              sc->snapdist);
	cc->geom.y += client_snapcalc(cc->geom.y,
	                              cc->geom.y + cc->geom.h + (cc->bwidth * 2),
	                              area.y,
	                              area.y + area.h,
	                              sc->snapdist);

	client_move(cc);
	client_ptr_inbound(cc, 1);
	XSync(X_Dpy, True);
}

static void kbfunc_client_move_mb(void* ctx, [[maybe_unused]] Cargs* cargs)
{
	auto cc = (Client_ctx*)ctx;
	XEvent ev;
	Time ltime = 0;
	Screen_ctx* sc = cc->sc;
	Geom area;
	int move = 1;

	client_raise(cc);

	if (cc->flags & Client_ctx::freeze) return;

	client_ptr_inbound(cc, 1);

	if (XGrabPointer(X_Dpy,
	                 cc->win,
	                 False,
	                 MOUSEMASK,
	                 GrabModeAsync,
	                 GrabModeAsync,
	                 None,
	                 conf->cursor[CF_MOVE],
	                 CurrentTime)
	    != GrabSuccess)
		return;

	screen_prop_win_create(sc, cc->win);
	screen_prop_win_draw(sc, "%+5d%+5d", cc->geom.x, cc->geom.y);
	while (move) {
		XMaskEvent(X_Dpy, MOUSEMASK, &ev);
		switch (ev.type) {
		case MotionNotify:
			/* not more than 60 times / second */
			if ((ev.xmotion.time - ltime) <= (1000 / 60)) continue;
			ltime = ev.xmotion.time;

			cc->geom.x = ev.xmotion.x_root - cc->ptr.x - cc->bwidth;
			cc->geom.y = ev.xmotion.y_root - cc->ptr.y - cc->bwidth;

			area = screen_area(sc, cc->geom.x + cc->geom.w / 2, cc->geom.y + cc->geom.h / 2, 1);
			cc->geom.x += client_snapcalc(cc->geom.x,
			                              cc->geom.x + cc->geom.w + (cc->bwidth * 2),
			                              area.x,
			                              area.x + area.w,
			                              sc->snapdist);
			cc->geom.y += client_snapcalc(cc->geom.y,
			                              cc->geom.y + cc->geom.h + (cc->bwidth * 2),
			                              area.y,
			                              area.y + area.h,
			                              sc->snapdist);
			client_move(cc);
			screen_prop_win_draw(sc, "%+5d%+5d", cc->geom.x, cc->geom.y);
			break;
		case ButtonRelease: move = 0; break;
		}
	}
	if (ltime) client_move(cc);
	screen_prop_win_destroy(sc);
	XUngrabPointer(X_Dpy, CurrentTime);
}

static void kbfunc_client_resize_kb(void* ctx, Cargs* cargs)
{
	auto cc = (Client_ctx*)ctx;
	int mx = 0, my = 0;
	int amt = 1;

	if (cc->flags & Client_ctx::freeze) return;

	if (!(cc->hint.flags & PResizeInc)) amt = conf->mamount;

	kbfunc_amount(cargs->flag, amt, &mx, &my);

	if ((cc->geom.w += mx * cc->hint.incw) < cc->hint.minw) cc->geom.w = cc->hint.minw;
	if ((cc->geom.h += my * cc->hint.inch) < cc->hint.minh) cc->geom.h = cc->hint.minh;
	if (cc->geom.x + cc->geom.w + cc->bwidth - 1 < 0) cc->geom.x = -(cc->geom.w + cc->bwidth - 1);
	if (cc->geom.y + cc->geom.h + cc->bwidth - 1 < 0) cc->geom.y = -(cc->geom.h + cc->bwidth - 1);

	client_resize(cc, 1);
	client_ptr_inbound(cc, 1);
	XSync(X_Dpy, True);
}

static void kbfunc_client_resize_mb(void* ctx, [[maybe_unused]] Cargs* cargs)
{
	auto cc = (Client_ctx*)ctx;
	XEvent ev;
	Time ltime = 0;
	Screen_ctx* sc = cc->sc;
	int resize = 1;

	if (cc->flags & Client_ctx::freeze) return;

	client_raise(cc);
	client_ptr_save(cc);

	xu_ptr_set(cc->win, cc->geom.w, cc->geom.h);

	if (XGrabPointer(X_Dpy,
	                 cc->win,
	                 False,
	                 MOUSEMASK,
	                 GrabModeAsync,
	                 GrabModeAsync,
	                 None,
	                 conf->cursor[CF_RESIZE],
	                 CurrentTime)
	    != GrabSuccess)
		return;

	screen_prop_win_create(sc, cc->win);
	screen_prop_win_draw(sc, "%4d x %-4d", cc->dim.w, cc->dim.h);
	while (resize) {
		XMaskEvent(X_Dpy, MOUSEMASK, &ev);
		switch (ev.type) {
		case MotionNotify:
			/* not more than 60 times / second */
			if ((ev.xmotion.time - ltime) <= (1000 / 60)) continue;
			ltime = ev.xmotion.time;

			cc->geom.w = ev.xmotion.x;
			cc->geom.h = ev.xmotion.y;
			cc->apply_sizehints();
			client_resize(cc, 1);
			screen_prop_win_draw(sc, "%4d x %-4d", cc->dim.w, cc->dim.h);
			break;
		case ButtonRelease: resize = 0; break;
		}
	}
	if (ltime) client_resize(cc, 1);
	screen_prop_win_destroy(sc);
	XUngrabPointer(X_Dpy, CurrentTime);

	/* Make sure the pointer stays within the window. */
	client_ptr_inbound(cc, 0);
}

void kbfunc_client_snap(void* ctx, Cargs* cargs)
{
	auto cc = (Client_ctx*)ctx;
	Screen_ctx* sc = cc->sc;
	Geom area;
	int flags;

	area = screen_area(sc, cc->geom.x + cc->geom.w / 2, cc->geom.y + cc->geom.h / 2, 1);

	flags = cargs->flag;
	while (flags) {
		if (flags & CWM_UP) {
			cc->geom.y = area.y;
			flags &= ~CWM_UP;
		}
		if (flags & CWM_LEFT) {
			cc->geom.x = area.x;
			flags &= ~CWM_LEFT;
		}
		if (flags & CWM_RIGHT) {
			cc->geom.x = area.x + area.w - cc->geom.w - (cc->bwidth * 2);
			flags &= ~CWM_RIGHT;
		}
		if (flags & CWM_DOWN) {
			cc->geom.y = area.y + area.h - cc->geom.h - (cc->bwidth * 2);
			flags &= ~CWM_DOWN;
		}
	}
	client_move(cc);
	client_ptr_inbound(cc, 1);
}

void kbfunc_client_close(void* ctx, [[maybe_unused]] Cargs* cargs)
{
	((Client_ctx*)ctx)->close();
}

void kbfunc_client_lower(void* ctx, [[maybe_unused]] Cargs* cargs)
{
	client_ptr_save((Client_ctx*)ctx);
	client_lower((Client_ctx*)ctx);
}

void kbfunc_client_raise(void* ctx, [[maybe_unused]] Cargs* cargs)
{
	client_raise((Client_ctx*)ctx);
}

void kbfunc_client_hide(void* ctx, [[maybe_unused]] Cargs* cargs)
{
	client_hide((Client_ctx*)ctx);
}

void kbfunc_client_toggle_freeze(void* ctx, [[maybe_unused]] Cargs* cargs)
{
	client_toggle_freeze((Client_ctx*)ctx);
}

void kbfunc_client_toggle_sticky(void* ctx, [[maybe_unused]] Cargs* cargs)
{
	client_toggle_sticky((Client_ctx*)ctx);
}

void kbfunc_client_toggle_fullscreen(void* ctx, [[maybe_unused]] Cargs* cargs)
{
	client_toggle_fullscreen((Client_ctx*)ctx);
}

void kbfunc_client_toggle_maximize(void* ctx, [[maybe_unused]] Cargs* cargs)
{
	client_toggle_maximize((Client_ctx*)ctx);
}

void kbfunc_client_toggle_hmaximize(void* ctx, [[maybe_unused]] Cargs* cargs)
{
	client_toggle_hmaximize((Client_ctx*)ctx);
}

void kbfunc_client_toggle_vmaximize(void* ctx, [[maybe_unused]] Cargs* cargs)
{
	client_toggle_vmaximize((Client_ctx*)ctx);
}

void kbfunc_client_htile(void* ctx, [[maybe_unused]] Cargs* cargs)
{
	client_htile((Client_ctx*)ctx);
}

void kbfunc_client_vtile(void* ctx, [[maybe_unused]] Cargs* cargs)
{
	client_vtile((Client_ctx*)ctx);
}

void kbfunc_client_cycle(void* ctx, [[maybe_unused]] Cargs* cargs)
{
	auto sc = (Screen_ctx*)ctx;
	Client_ctx *newcc, *oldcc, *prevcc;
	int again = 1;
	auto flags = cargs->flag;

	/* For X apps that ignore/steal events. */
	if (cargs->xev == cwm::Xev::key)
		XGrabKeyboard(X_Dpy, sc->rootwin, True, GrabModeAsync, GrabModeAsync, CurrentTime);

	if (TAILQ_EMPTY(&sc->clientq)) return;

	prevcc = TAILQ_FIRST(&sc->clientq);
	oldcc = client_current(sc);
	if (oldcc == nullptr)
		oldcc = (flags & CWM_CYCLE_REVERSE) ? TAILQ_LAST(&sc->clientq, client_q)
		                                    : TAILQ_FIRST(&sc->clientq);

	newcc = oldcc;
	while (again) {
		again = 0;

		newcc = (flags & CWM_CYCLE_REVERSE) ? client_prev(newcc) : client_next(newcc);

		/* Only cycle visible and non-ignored windows. */
		if ((newcc->flags & (Client_ctx::skip_cycle))
		    || ((flags & CWM_CYCLE_INGROUP) && (newcc->gc != oldcc->gc)))
			again = 1;

		/* Is oldcc the only non-hidden window? */
		if (newcc == oldcc) {
			if (again) return; /* No windows visible. */
			break;
		}
	}

	/* Reset when cycling mod is released. XXX I hate this hack */
	sc->cycling = 1;
	client_ptr_save(oldcc);
	client_raise(prevcc);
	client_raise(newcc);
	if (!client_inbound(newcc, newcc->ptr.x, newcc->ptr.y)) {
		newcc->ptr.x = newcc->geom.w / 2;
		newcc->ptr.y = newcc->geom.h / 2;
	}
	client_ptr_warp(newcc);
}

void kbfunc_client_toggle_group(void* ctx, Cargs* cargs)
{
	auto cc = (Client_ctx*)ctx;

	/* For X apps that ignore/steal events. */
	if (cargs->xev == cwm::Xev::key)
		XGrabKeyboard(X_Dpy, cc->win, True, GrabModeAsync, GrabModeAsync, CurrentTime);

	group_toggle_membership(cc);
}

void kbfunc_client_movetogroup(void* ctx, Cargs* cargs)
{
	group_movetogroup((Client_ctx*)ctx, cargs->flag);
}

void kbfunc_group_only(void* ctx, Cargs* cargs)
{
	group_only((Screen_ctx*)ctx, cargs->flag);
}

void kbfunc_group_toggle(void* ctx, Cargs* cargs)
{
	group_toggle((Screen_ctx*)ctx, cargs->flag);
}

void kbfunc_group_toggle_all(void* ctx, [[maybe_unused]] Cargs* cargs)
{
	group_toggle_all((Screen_ctx*)ctx);
}

void kbfunc_group_close(void* ctx, Cargs* cargs)
{
	group_close((Screen_ctx*)ctx, cargs->flag);
}

void kbfunc_group_cycle(void* ctx, Cargs* cargs)
{
	group_cycle((Screen_ctx*)ctx, cargs->flag);
}

void kbfunc_menu_client(void* ctx, Cargs* cargs)
{
	auto sc = (Screen_ctx*)ctx;
	Client_ctx *cc, *old_cc;
	Menu* mi;
	struct menu_q menuq;
	int mflags = 0;

	if (cargs->xev == cwm::Xev::btn) mflags |= CWM_MENU_LIST;

	TAILQ_INIT(&menuq);
	TAILQ_FOREACH(cc, &sc->clientq, entry)
	{
		if ((cargs->flag & CWM_MENU_WINDOW_ALL) || (cc->flags & Client_ctx::hidden))
			menuq_add(&menuq, cc, nullptr);
	}

	if ((mi = menu_filter(sc,
	                      &menuq,
	                      "window",
	                      nullptr,
	                      mflags,
	                      search_match_client,
	                      search_print_client))
	    != nullptr) {
		cc = (Client_ctx*)mi->ctx;
		client_show(cc);
		if ((old_cc = client_current(sc)) != nullptr) client_ptr_save(old_cc);
		client_ptr_warp(cc);
	}

	menuq_clear(&menuq);
}

void kbfunc_menu_cmd(void* ctx, Cargs* cargs)
{
	auto sc = (Screen_ctx*)ctx;
	Cmd_ctx* cmd;
	Menu* mi;
	struct menu_q menuq;
	int mflags = 0;

	if (cargs->xev == cwm::Xev::btn) mflags |= CWM_MENU_LIST;

	TAILQ_INIT(&menuq);
	TAILQ_FOREACH(cmd, &conf->cmdq, entry)
	{
		if ((strcmp(cmd->name, "lock") == 0) || (strcmp(cmd->name, "term") == 0)) continue;
		menuq_add(&menuq, cmd, nullptr);
	}

	if ((mi = menu_filter(sc,
	                      &menuq,
	                      "application",
	                      nullptr,
	                      mflags,
	                      search_match_cmd,
	                      search_print_cmd))
	    != nullptr) {
		cmd = (Cmd_ctx*)mi->ctx;
		u_spawn(cmd->path);
	}

	menuq_clear(&menuq);
}

void kbfunc_menu_group(void* ctx, Cargs* cargs)
{
	auto sc = (Screen_ctx*)ctx;
	Group_ctx* gc;
	Menu* mi;
	struct menu_q menuq;
	int mflags = 0;

	if (cargs->xev == cwm::Xev::btn) mflags |= CWM_MENU_LIST;

	TAILQ_INIT(&menuq);
	TAILQ_FOREACH(gc, &sc->groupq, entry)
	{
		if (group_holds_only_sticky(gc)) continue;
		menuq_add(&menuq, gc, nullptr);
	}

	if ((mi = menu_filter(sc,
	                      &menuq,
	                      "group",
	                      nullptr,
	                      mflags,
	                      search_match_group,
	                      search_print_group))
	    != nullptr) {
		gc = (Group_ctx*)mi->ctx;
		(group_holds_only_hidden(gc)) ? group_show(gc) : group_hide(gc);
	}

	menuq_clear(&menuq);
}

void kbfunc_menu_wm(void* ctx, Cargs* cargs)
{
	auto sc = (Screen_ctx*)ctx;
	Menu* mi;
	struct menu_q menuq;
	int mflags = 0;

	if (cargs->xev == cwm::Xev::btn) mflags |= CWM_MENU_LIST;

	TAILQ_INIT(&menuq);
	for (auto wm : conf->wmq) menuq_add(&menuq, wm, nullptr);

	if ((mi = menu_filter(sc, &menuq, "wm", nullptr, mflags, search_match_wm, search_print_wm))
	    != nullptr) {
		auto wm = (Cmd_ctx*)mi->ctx;
		std::free(conf->wm_argv);
		conf->wm_argv = strdup(wm->path);
		cwm_status = Cwm_status::CWM_EXEC_WM;
	}

	menuq_clear(&menuq);
}

void kbfunc_menu_exec(void* ctx, [[maybe_unused]] Cargs* cargs)
{
	static constexpr auto NPATHS {256};
	auto sc = (Screen_ctx*)ctx;
	char **ap, *paths[NPATHS], *path, *pathcpy;
	char tpath[PATH_MAX];
	struct stat sb;
	DIR* dirp;
	struct dirent* dp;
	Menu* mi;
	struct menu_q menuq;
	int i;
	int mflags = (CWM_MENU_DUMMY | CWM_MENU_FILE);

	TAILQ_INIT(&menuq);

	if ((path = getenv("PATH")) == nullptr) path = const_cast<char*>(_PATH_DEFPATH);
	pathcpy = path = strdup(path);

	for (ap = paths; ap < &paths[NPATHS - 1] && (*ap = strsep(&pathcpy, ":")) != nullptr;) {
		if (**ap != '\0') ap++;
	}
	*ap = nullptr;
	for (i = 0; i < NPATHS && paths[i] != nullptr; i++) {
		if ((dirp = opendir(paths[i])) == nullptr) continue;

		while ((dp = readdir(dirp)) != nullptr) {
			(void)memset(tpath, '\0', sizeof(tpath));

			if (auto l {snprintf(tpath, sizeof(tpath), "%s/%s", paths[i], dp->d_name)};
			    l == -1 || l >= sizeof(tpath))
				continue;
			/* Skip everything but regular files and symlinks. */
			if (dp->d_type != DT_REG && dp->d_type != DT_LNK) {
				/* lstat(2) in case d_type isn't supported. */
				if (lstat(tpath, &sb) == -1) continue;
				if (!S_ISREG(sb.st_mode) && !S_ISLNK(sb.st_mode)) continue;
			}
			if (access(tpath, X_OK) == 0) menuq_add(&menuq, nullptr, "%s", dp->d_name);
		}
		(void)closedir(dirp);
	}
	free(path);

	if ((mi
	     = menu_filter(sc, &menuq, "exec", nullptr, mflags, search_match_exec, search_print_text))
	    != nullptr) {
		if (mi->text[0] == '\0') goto out;
		u_spawn(mi->text);
	}
out:
	if (mi != nullptr && mi->dummy) free(mi);
	menuq_clear(&menuq);
}

void kbfunc_menu_ssh(void* ctx, [[maybe_unused]] Cargs* cargs)
{
	auto sc = (Screen_ctx*)ctx;
	Cmd_ctx* cmd;
	Menu* mi;
	struct menu_q menuq;
	FILE* fp;
	char *buf, *lbuf, *p;
	char hostbuf[_POSIX_HOST_NAME_MAX + 1];
	char path[PATH_MAX];
	int l;
	size_t len;
	ssize_t slen;
	int mflags = (CWM_MENU_DUMMY);

	TAILQ_FOREACH(cmd, &conf->cmdq, entry)
	{
		if (strcmp(cmd->name, "term") == 0) break;
	}
	TAILQ_INIT(&menuq);

	if ((fp = fopen(conf->known_hosts.c_str(), "r")) == nullptr) {
		warn("%s: %s", __func__, conf->known_hosts.c_str());
		goto menu;
	}

	lbuf = nullptr;
	len = 0;
	while ((slen = getline(&lbuf, &len, fp)) != -1) {
		buf = lbuf;
		if (buf[slen - 1] == '\n') buf[slen - 1] = '\0';

		/* skip hashed hosts */
		if (strncmp(buf, HASH_MARKER, strlen(HASH_MARKER)) == 0) continue;
		for (p = buf; *p != ',' && *p != ' ' && p != buf + slen; p++)
			;
		/* ignore badness */
		if (p - buf + 1 > sizeof(hostbuf)) continue;
		(void)strlcpy(hostbuf, buf, p - buf + 1);
		menuq_add(&menuq, nullptr, "%s", hostbuf);
	}
	free(lbuf);
	if (ferror(fp)) err(1, "%s", path);
	(void)fclose(fp);
menu:
	if ((mi = menu_filter(sc, &menuq, "ssh", nullptr, mflags, search_match_text, search_print_text))
	    != nullptr) {
		if (mi->text[0] == '\0') goto out;
		l = snprintf(path,
		             sizeof(path),
		             "%s -T '[ssh] %s' -e ssh %s",
		             cmd->path,
		             mi->text,
		             mi->text);
		if (l == -1 || l >= sizeof(path)) goto out;
		u_spawn(path);
	}
out:
	if (mi != nullptr && mi->dummy) free(mi);
	menuq_clear(&menuq);
}

void kbfunc_client_menu_label(void* ctx, [[maybe_unused]] Cargs* cargs)
{
	auto cc = (Client_ctx*)ctx;
	Menu* mi;
	struct menu_q menuq;
	int mflags = (CWM_MENU_DUMMY);

	TAILQ_INIT(&menuq);

	/* dummy is set, so this will always return */
	mi = menu_filter(cc->sc,
	                 &menuq,
	                 "label",
	                 cc->label,
	                 mflags,
	                 search_match_text,
	                 search_print_text);

	if (!mi->abort) { cc->label = mi->text; }
	free(mi);
}

void kbfunc_exec_cmd([[maybe_unused]] void* ctx, Cargs* cargs)
{
	u_spawn(cargs->cmd);
}

void kbfunc_exec_term([[maybe_unused]] void* ctx, [[maybe_unused]] Cargs* cargs)
{
	Cmd_ctx* cmd;
	TAILQ_FOREACH(cmd, &conf->cmdq, entry)
	{
		if (strcmp(cmd->name, "term") == 0) u_spawn(cmd->path);
	}
}

void kbfunc_exec_lock([[maybe_unused]] void* ctx, [[maybe_unused]] Cargs* cargs)
{
	Cmd_ctx* cmd;
	TAILQ_FOREACH(cmd, &conf->cmdq, entry)
	{
		if (strcmp(cmd->name, "lock") == 0) u_spawn(cmd->path);
	}
}
