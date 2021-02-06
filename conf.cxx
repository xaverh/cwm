/*
 * calmwm-the calm window manager
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
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <err.h>
#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>

static char const* conf_bind_mask(char const*, unsigned int*);
static void conf_unbind_key(struct Conf*, Bind_ctx*);
static void conf_unbind_mouse(struct Conf*, Bind_ctx*);

static constexpr struct {
	int num;
	char const* name;
} group_binds[] = {
    {0, "nogroup"},
    {1, "one"},
    {2, "two"},
    {3, "three"},
    {4, "four"},
    {5, "five"},
    {6, "six"},
    {7, "seven"},
    {8, "eight"},
    {9, "nine"},
};

static constexpr int cursor_binds[] = {
    XC_left_ptr,            // CF_NORMAL
    XC_fleur,               // CF_MOVE
    XC_bottom_right_corner, // CF_RESIZE
    XC_question_arrow,      // CF_QUESTION
};

static constexpr char const* color_binds[] = {
    "#CCCCCC", // CWM_COLOR_BORDER_ACTIVE
    "#666666", // CWM_COLOR_BORDER_INACTIVE
    "#FC8814", // CWM_COLOR_BORDER_URGENCY
    "blue",    // CWM_COLOR_BORDER_GROUP
    "red",     // CWM_COLOR_BORDER_UNGROUP
    "black",   // CWM_COLOR_MENU_FG
    "white",   // CWM_COLOR_MENU_BG
    "black",   // CWM_COLOR_MENU_FONT
    "",        // CWM_COLOR_MENU_FONT_SEL
};

static constexpr struct {
	char const* tag;
	void (*const handler)(void*, Cargs*);
	Context const context;
	int const flag;
} name_to_func[] {
    {"window-lower", kbfunc_client_lower, Context::cc, 0},
    {"window-raise", kbfunc_client_raise, Context::cc, 0},
    {"window-hide", kbfunc_client_hide, Context::cc, 0},
    {"window-close", kbfunc_client_close, Context::cc, 0},
    {"window-delete", kbfunc_client_close, Context::cc, 0},
    {"window-htile", kbfunc_client_htile, Context::cc, 0},
    {"window-vtile", kbfunc_client_vtile, Context::cc, 0},
    {"window-stick", kbfunc_client_toggle_sticky, Context::cc, 0},
    {"window-fullscreen", kbfunc_client_toggle_fullscreen, Context::cc, 0},
    {"window-maximize", kbfunc_client_toggle_maximize, Context::cc, 0},
    {"window-vmaximize", kbfunc_client_toggle_vmaximize, Context::cc, 0},
    {"window-hmaximize", kbfunc_client_toggle_hmaximize, Context::cc, 0},
    {"window-freeze", kbfunc_client_toggle_freeze, Context::cc, 0},
    {"window-group", kbfunc_client_toggle_group, Context::cc, 0},
    {"window-movetogroup-1", kbfunc_client_movetogroup, Context::cc, 1},
    {"window-movetogroup-2", kbfunc_client_movetogroup, Context::cc, 2},
    {"window-movetogroup-3", kbfunc_client_movetogroup, Context::cc, 3},
    {"window-movetogroup-4", kbfunc_client_movetogroup, Context::cc, 4},
    {"window-movetogroup-5", kbfunc_client_movetogroup, Context::cc, 5},
    {"window-movetogroup-6", kbfunc_client_movetogroup, Context::cc, 6},
    {"window-movetogroup-7", kbfunc_client_movetogroup, Context::cc, 7},
    {"window-movetogroup-8", kbfunc_client_movetogroup, Context::cc, 8},
    {"window-movetogroup-9", kbfunc_client_movetogroup, Context::cc, 9},
    {"window-snap-up", kbfunc_client_snap, Context::cc, CWM_UP},
    {"window-snap-down", kbfunc_client_snap, Context::cc, CWM_DOWN},
    {"window-snap-right", kbfunc_client_snap, Context::cc, CWM_RIGHT},
    {"window-snap-left", kbfunc_client_snap, Context::cc, CWM_LEFT},
    {"window-snap-up-right", kbfunc_client_snap, Context::cc, CWM_UP_RIGHT},
    {"window-snap-up-left", kbfunc_client_snap, Context::cc, CWM_UP_LEFT},
    {"window-snap-down-right", kbfunc_client_snap, Context::cc, CWM_DOWN_RIGHT},
    {"window-snap-down-left", kbfunc_client_snap, Context::cc, CWM_DOWN_LEFT},
    {"window-move", kbfunc_client_move, Context::cc, 0},
    {"window-move-up", kbfunc_client_move, Context::cc, CWM_UP},
    {"window-move-down", kbfunc_client_move, Context::cc, CWM_DOWN},
    {"window-move-right", kbfunc_client_move, Context::cc, CWM_RIGHT},
    {"window-move-left", kbfunc_client_move, Context::cc, CWM_LEFT},
    {"window-move-up-big", kbfunc_client_move, Context::cc, CWM_UP_BIG},
    {"window-move-down-big", kbfunc_client_move, Context::cc, CWM_DOWN_BIG},
    {"window-move-right-big", kbfunc_client_move, Context::cc, CWM_RIGHT_BIG},
    {"window-move-left-big", kbfunc_client_move, Context::cc, CWM_LEFT_BIG},
    {"window-resize", kbfunc_client_resize, Context::cc, 0},
    {"window-resize-up", kbfunc_client_resize, Context::cc, CWM_UP},
    {"window-resize-down", kbfunc_client_resize, Context::cc, CWM_DOWN},
    {"window-resize-right", kbfunc_client_resize, Context::cc, CWM_RIGHT},
    {"window-resize-left", kbfunc_client_resize, Context::cc, CWM_LEFT},
    {"window-resize-up-big", kbfunc_client_resize, Context::cc, CWM_UP_BIG},
    {"window-resize-down-big", kbfunc_client_resize, Context::cc, CWM_DOWN_BIG},
    {"window-resize-right-big", kbfunc_client_resize, Context::cc, CWM_RIGHT_BIG},
    {"window-resize-left-big", kbfunc_client_resize, Context::cc, CWM_LEFT_BIG},
    {"window-menu-label", kbfunc_client_menu_label, Context::cc, 0},
    {"window-cycle", kbfunc_client_cycle, Context::sc, CWM_CYCLE_FORWARD},
    {"window-rcycle", kbfunc_client_cycle, Context::sc, CWM_CYCLE_REVERSE},
    {"window-cycle-ingroup",
     kbfunc_client_cycle,
     Context::sc,
     CWM_CYCLE_FORWARD | CWM_CYCLE_INGROUP},
    {"window-rcycle-ingroup",
     kbfunc_client_cycle,
     Context::sc,
     CWM_CYCLE_REVERSE | CWM_CYCLE_INGROUP},
    {"group-cycle", kbfunc_group_cycle, Context::sc, CWM_CYCLE_FORWARD},
    {"group-rcycle", kbfunc_group_cycle, Context::sc, CWM_CYCLE_REVERSE},
    {"group-toggle-all", kbfunc_group_toggle_all, Context::sc, 0},
    {"group-toggle-1", kbfunc_group_toggle, Context::sc, 1},
    {"group-toggle-2", kbfunc_group_toggle, Context::sc, 2},
    {"group-toggle-3", kbfunc_group_toggle, Context::sc, 3},
    {"group-toggle-4", kbfunc_group_toggle, Context::sc, 4},
    {"group-toggle-5", kbfunc_group_toggle, Context::sc, 5},
    {"group-toggle-6", kbfunc_group_toggle, Context::sc, 6},
    {"group-toggle-7", kbfunc_group_toggle, Context::sc, 7},
    {"group-toggle-8", kbfunc_group_toggle, Context::sc, 8},
    {"group-toggle-9", kbfunc_group_toggle, Context::sc, 9},
    {"group-only-1", kbfunc_group_only, Context::sc, 1},
    {"group-only-2", kbfunc_group_only, Context::sc, 2},
    {"group-only-3", kbfunc_group_only, Context::sc, 3},
    {"group-only-4", kbfunc_group_only, Context::sc, 4},
    {"group-only-5", kbfunc_group_only, Context::sc, 5},
    {"group-only-6", kbfunc_group_only, Context::sc, 6},
    {"group-only-7", kbfunc_group_only, Context::sc, 7},
    {"group-only-8", kbfunc_group_only, Context::sc, 8},
    {"group-only-9", kbfunc_group_only, Context::sc, 9},
    {"group-close-1", kbfunc_group_close, Context::sc, 1},
    {"group-close-2", kbfunc_group_close, Context::sc, 2},
    {"group-close-3", kbfunc_group_close, Context::sc, 3},
    {"group-close-4", kbfunc_group_close, Context::sc, 4},
    {"group-close-5", kbfunc_group_close, Context::sc, 5},
    {"group-close-6", kbfunc_group_close, Context::sc, 6},
    {"group-close-7", kbfunc_group_close, Context::sc, 7},
    {"group-close-8", kbfunc_group_close, Context::sc, 8},
    {"group-close-9", kbfunc_group_close, Context::sc, 9},
    {"pointer-move-up", kbfunc_ptrmove, Context::sc, CWM_UP},
    {"pointer-move-down", kbfunc_ptrmove, Context::sc, CWM_DOWN},
    {"pointer-move-left", kbfunc_ptrmove, Context::sc, CWM_LEFT},
    {"pointer-move-right", kbfunc_ptrmove, Context::sc, CWM_RIGHT},
    {"pointer-move-up-big", kbfunc_ptrmove, Context::sc, CWM_UP_BIG},
    {"pointer-move-down-big", kbfunc_ptrmove, Context::sc, CWM_DOWN_BIG},
    {"pointer-move-left-big", kbfunc_ptrmove, Context::sc, CWM_LEFT_BIG},
    {"pointer-move-right-big", kbfunc_ptrmove, Context::sc, CWM_RIGHT_BIG},
    {"menu-cmd", kbfunc_menu_cmd, Context::sc, 0},
    {"menu-group", kbfunc_menu_group, Context::sc, 0},
    {"menu-ssh", kbfunc_menu_ssh, Context::sc, 0},
    {"menu-window", kbfunc_menu_client, Context::sc, CWM_MENU_WINDOW_ALL},
    {"menu-window-hidden", kbfunc_menu_client, Context::sc, CWM_MENU_WINDOW_HIDDEN},
    {"menu-exec", kbfunc_menu_exec, Context::sc, 0},
    {"menu-exec-wm", kbfunc_menu_wm, Context::sc, 0},
    {"terminal", kbfunc_exec_term, Context::sc, 0},
    {"lock", kbfunc_exec_lock, Context::sc, 0},
    {"restart", kbfunc_cwm_status, Context::sc, Cwm_status::CWM_EXEC_WM},
    {"quit", kbfunc_cwm_status, Context::sc, Cwm_status::CWM_QUIT},
};

static constexpr unsigned int ignore_mods[] = {0, LockMask, Mod2Mask, Mod2Mask | LockMask};

static constexpr struct {
	char const ch;
	unsigned const mask;
} bind_mods[] = {
    {'S', ShiftMask},
    {'C', ControlMask},
    {'M', Mod1Mask},
    {'4', Mod4Mask},
    {'5', Mod5Mask},
};

static constexpr struct {
	char const* key;
	char const* func;
} key_binds[] = {
	{ "CM-Return",	"terminal" },
	{ "CM-Delete",	"lock" },
	{ "M-question",	"menu-exec" },
	{ "CM-w",	"menu-exec-wm" },
	{ "M-period",	"menu-ssh" },
	{ "M-Return",	"window-hide" },
	{ "M-Down",	"window-lower" },
	{ "M-Up",	"window-raise" },
	{ "M-slash",	"menu-window" },
	{ "C-slash",	"menu-cmd" },
	{ "M-Tab",	"window-cycle" },
	{ "MS-Tab",	"window-rcycle" },
	{ "CM-n",	"window-menu-label" },
	{ "CM-x",	"window-close" },
	{ "CM-a",	"group-toggle-all" },
	{ "CM-0",	"group-toggle-all" },
	{ "CM-1",	"group-toggle-1" },
	{ "CM-2",	"group-toggle-2" },
	{ "CM-3",	"group-toggle-3" },
	{ "CM-4",	"group-toggle-4" },
	{ "CM-5",	"group-toggle-5" },
	{ "CM-6",	"group-toggle-6" },
	{ "CM-7",	"group-toggle-7" },
	{ "CM-8",	"group-toggle-8" },
	{ "CM-9",	"group-toggle-9" },
	{ "M-Right",	"group-cycle" },
	{ "M-Left",	"group-rcycle" },
	{ "CM-g",	"window-group" },
	{ "CM-f",	"window-fullscreen" },
	{ "CM-m",	"window-maximize" },
	{ "CM-s",	"window-stick" },
	{ "CM-equal",	"window-vmaximize" },
	{ "CMS-equal",	"window-hmaximize" },
	{ "CMS-f",	"window-freeze" },
	{ "CMS-r",	"restart" },
	{ "CMS-q",	"quit" },
	{ "M-h",	"window-move-left" },
	{ "M-j",	"window-move-down" },
	{ "M-k",	"window-move-up" },
	{ "M-l",	"window-move-right" },
	{ "MS-h",	"window-move-left-big" },
	{ "MS-j",	"window-move-down-big" },
	{ "MS-k",	"window-move-up-big" },
	{ "MS-l",	"window-move-right-big" },
	{ "CM-h",	"window-resize-left" },
	{ "CM-j",	"window-resize-down" },
	{ "CM-k",	"window-resize-up" },
	{ "CM-l",	"window-resize-right" },
	{ "CMS-h",	"window-resize-left-big" },
	{ "CMS-j",	"window-resize-down-big" },
	{ "CMS-k",	"window-resize-up-big" },
	{ "CMS-l",	"window-resize-right-big" },
},
mouse_binds[] = {
	{ "1",		"menu-window" },
	{ "2",		"menu-group" },
	{ "3",		"menu-cmd" },
	{ "M-1",	"window-move" },
	{ "CM-1",	"window-group" },
	{ "M-2",	"window-resize" },
	{ "M-3",	"window-lower" },
	{ "CMS-3",	"window-hide" },
};

void conf_init(struct Conf* c)
{
	char const* home;
	struct passwd* pw;
	unsigned int i;

	c->stickygroups = 0;
	c->bwidth = 1;
	c->mamount = 1;
	c->htile = 50;
	c->vtile = 50;
	c->snapdist = 0;
	c->ngroups = 0;
	c->nameqlen = 5;

	TAILQ_INIT(&c->ignoreq);
	TAILQ_INIT(&c->autogroupq);
	TAILQ_INIT(&c->keybindq);
	TAILQ_INIT(&c->mousebindq);
	TAILQ_INIT(&c->cmdq);
	TAILQ_INIT(&c->wmq);

	for (auto const& k : key_binds) conf_bind_key(c, k.key, k.func);

	for (i = 0; i < nitems(mouse_binds); i++)
		conf_bind_mouse(c, mouse_binds[i].key, mouse_binds[i].func);

	for (i = 0; i < nitems(color_binds); i++) c->color[i] = xstrdup(color_binds[i]);

	conf_cmd_add(c, "lock", "xlock");
	conf_cmd_add(c, "term", "xterm");
	conf_wm_add(c, "cwm", "cwm");

	c->font = xstrdup("sans-serif:pixelsize=14:bold");
	c->wmname = xstrdup("CWM");

	home = getenv("HOME");
	if ((home == nullptr) || (*home == '\0')) {
		pw = getpwuid(getuid());
		if (pw != nullptr && pw->pw_dir != nullptr && *pw->pw_dir != '\0')
			home = pw->pw_dir;
		else
			home = "/";
	}
	xasprintf(&c->conf_file, "%s/%s", home, ".cwmrc");
	xasprintf(&c->known_hosts, "%s/%s", home, ".ssh/known_hosts");
}

void conf_clear(struct Conf* c)
{
	Autogroup* ag;
	Bind_ctx *kb, *mb;
	Winname* wn;
	Cmd_ctx *cmd, *wm;
	int i;

	while ((cmd = TAILQ_FIRST(&c->cmdq)) != nullptr) {
		TAILQ_REMOVE(&c->cmdq, cmd, entry);
		free(cmd->name);
		free(cmd->path);
		free(cmd);
	}
	while ((wm = TAILQ_FIRST(&c->wmq)) != nullptr) {
		TAILQ_REMOVE(&c->wmq, wm, entry);
		free(wm->name);
		free(wm->path);
		free(wm);
	}
	while ((kb = TAILQ_FIRST(&c->keybindq)) != nullptr) {
		TAILQ_REMOVE(&c->keybindq, kb, entry);
		free(kb);
	}
	while ((ag = TAILQ_FIRST(&c->autogroupq)) != nullptr) {
		TAILQ_REMOVE(&c->autogroupq, ag, entry);
		free(ag->wclass);
		free(ag->name);
		free(ag);
	}
	while ((wn = TAILQ_FIRST(&c->ignoreq)) != nullptr) {
		TAILQ_REMOVE(&c->ignoreq, wn, entry);
		free(wn->name);
		free(wn);
	}
	while ((mb = TAILQ_FIRST(&c->mousebindq)) != nullptr) {
		TAILQ_REMOVE(&c->mousebindq, mb, entry);
		free(mb);
	}
	for (i = 0; i < CWM_COLOR_NITEMS; i++) free(c->color[i]);

	free(c->conf_file);
	free(c->known_hosts);
	free(c->font);
	free(c->wmname);
}

void conf_cmd_add(struct Conf* c, char const* name, char const* path)
{
	Cmd_ctx *cmd, *cmdtmp = nullptr, *cmdnxt;

	cmd = static_cast<Cmd_ctx*>(xmalloc(sizeof(*cmd)));
	cmd->name = xstrdup(name);
	cmd->path = xstrdup(path);

	TAILQ_FOREACH_SAFE(cmdtmp, &c->cmdq, entry, cmdnxt)
	{
		if (strcmp(cmdtmp->name, name) == 0) {
			TAILQ_REMOVE(&c->cmdq, cmdtmp, entry);
			free(cmdtmp->name);
			free(cmdtmp->path);
			free(cmdtmp);
		}
	}
	TAILQ_INSERT_TAIL(&c->cmdq, cmd, entry);
}

void conf_wm_add(struct Conf* c, char const* name, char const* path)
{
	Cmd_ctx *wm, *wmtmp = nullptr, *wmnxt;

	wm = static_cast<Cmd_ctx*>(xmalloc(sizeof(*wm)));
	wm->name = xstrdup(name);
	wm->path = xstrdup(path);

	TAILQ_FOREACH_SAFE(wmtmp, &c->cmdq, entry, wmnxt)
	{
		if (strcmp(wmtmp->name, name) == 0) {
			TAILQ_REMOVE(&c->wmq, wmtmp, entry);
			free(wmtmp->name);
			free(wmtmp->path);
			free(wmtmp);
		}
	}
	TAILQ_INSERT_TAIL(&c->wmq, wm, entry);
}

void conf_autogroup(struct Conf* c, int num, char const* name, char const* wclass)
{
	Autogroup* ag;
	char* p;

	ag = static_cast<Autogroup*>(xmalloc(sizeof(*ag)));
	if ((p = const_cast<char*>(strchr(wclass, ','))) == nullptr) {
		if (name == nullptr)
			ag->name = nullptr;
		else
			ag->name = xstrdup(name);

		ag->wclass = xstrdup(wclass);
	} else {
		*(p++) = '\0';
		if (name == nullptr)
			ag->name = xstrdup(wclass);
		else
			ag->name = xstrdup(name);

		ag->wclass = xstrdup(p);
	}
	ag->num = num;
	TAILQ_INSERT_TAIL(&c->autogroupq, ag, entry);
}

void conf_ignore(struct Conf* c, char const* name)
{
	Winname* wn;

	wn = (Winname*)xmalloc(sizeof(*wn));
	wn->name = xstrdup(name);
	TAILQ_INSERT_TAIL(&c->ignoreq, wn, entry);
}

void conf_cursor(struct Conf* c)
{
	unsigned int i;

	for (i = 0; i < nitems(cursor_binds); i++)
		c->cursor[i] = XCreateFontCursor(X_Dpy, cursor_binds[i]);
}

void conf_client(Client_ctx* cc)
{
	Winname* wn;

	TAILQ_FOREACH(wn, &conf.ignoreq, entry)
	{
		if (strncasecmp(wn->name, cc->name, strlen(wn->name)) == 0) {
			cc->flags |= CLIENT_IGNORE;
			break;
		}
	}
}

void conf_screen(Screen_ctx* sc)
{
	unsigned int i;
	XftColor xc;

	sc->gap = conf.gap;
	sc->snapdist = conf.snapdist;

	sc->xftfont = XftFontOpenXlfd(X_Dpy, sc->which, conf.font);
	if (sc->xftfont == nullptr) {
		sc->xftfont = XftFontOpenName(X_Dpy, sc->which, conf.font);
		if (sc->xftfont == nullptr) errx(1, "%s: XftFontOpenName: %s", __func__, conf.font);
	}

	for (i = 0; i < nitems(color_binds); i++) {
		if (i == CWM_COLOR_MENU_FONT_SEL && *conf.color[i] == '\0') {
			xu_xorcolor(sc->xftcolor[CWM_COLOR_MENU_BG], sc->xftcolor[CWM_COLOR_MENU_FG], &xc);
			xu_xorcolor(sc->xftcolor[CWM_COLOR_MENU_FONT], xc, &xc);
			if (!XftColorAllocValue(X_Dpy,
			                        sc->visual,
			                        sc->colormap,
			                        &xc.color,
			                        &sc->xftcolor[CWM_COLOR_MENU_FONT_SEL]))
				warnx("XftColorAllocValue: %s", conf.color[i]);
			break;
		}
		if (!XftColorAllocName(X_Dpy, sc->visual, sc->colormap, conf.color[i], &sc->xftcolor[i])) {
			warnx("XftColorAllocName: %s", conf.color[i]);
			XftColorAllocName(X_Dpy, sc->visual, sc->colormap, color_binds[i], &sc->xftcolor[i]);
		}
	}

	conf_grab_kbd(sc->rootwin);
}

void conf_group(Screen_ctx* sc)
{
	for (auto const& g : group_binds) {
		group_init(sc, g.num, g.name);
		++conf.ngroups;
	}
}

static char const* conf_bind_mask(char const* name, unsigned int* mask)
{
	char const *dash;
	*mask = 0;
	if ((dash = strchr(name, '-')) == nullptr) return name;
	for (auto const& b : bind_mods) {
		if (auto ch = strchr(name, b.ch); ch != nullptr && ch < dash) *mask |= b.mask;
	}
	/* Skip past modifiers. */
	return (dash + 1);
}

int conf_bind_key(struct Conf* c, char const* bind, char const* cmd)
{
	Bind_ctx* kb;
	Cargs* cargs;
	char const* key;
	unsigned int i;

	if ((strcmp(bind, "all") == 0) && (cmd == nullptr)) {
		conf_unbind_key(c, nullptr);
		return 1;
	}
	kb = (Bind_ctx*)xmalloc(sizeof(*kb));
	key = conf_bind_mask(bind, &kb->modmask);
	kb->press.keysym = XStringToKeysym(key);
	if (kb->press.keysym == NoSymbol) {
		warnx("unknown symbol: %s", key);
		free(kb);
		return 0;
	}
	conf_unbind_key(c, kb);
	if (cmd == nullptr) {
		free(kb);
		return 1;
	}
	cargs = (Cargs*)xcalloc(1, sizeof(*cargs));
	for (i = 0; i < nitems(name_to_func); i++) {
		if (strcmp(name_to_func[i].tag, cmd) != 0) continue;
		kb->callback = name_to_func[i].handler;
		kb->context = name_to_func[i].context;
		cargs->flag = name_to_func[i].flag;
		goto out;
	}
	kb->callback = kbfunc_exec_cmd;
	kb->context = Context::none;
	cargs->flag = 0;
	cargs->cmd = xstrdup(cmd);
out:
	kb->cargs = cargs;
	TAILQ_INSERT_TAIL(&c->keybindq, kb, entry);
	return 1;
}

static void conf_unbind_key(struct Conf* c, Bind_ctx* unbind)
{
	Bind_ctx *key = nullptr, *keynxt;

	TAILQ_FOREACH_SAFE(key, &c->keybindq, entry, keynxt)
	{
		if ((unbind == nullptr)
		    || ((key->modmask == unbind->modmask) && (key->press.keysym == unbind->press.keysym))) {
			TAILQ_REMOVE(&c->keybindq, key, entry);
			free(key->cargs->cmd);
			free(key->cargs);
			free(key);
		}
	}
}

int conf_bind_mouse(struct Conf* c, char const* bind, char const* cmd)
{
	Bind_ctx* mb;
	Cargs* cargs;
	char const *button, *errstr;
	unsigned int i;

	if ((strcmp(bind, "all") == 0) && (cmd == nullptr)) {
		conf_unbind_mouse(c, nullptr);
		return 1;
	}
	mb = (Bind_ctx*)xmalloc(sizeof(*mb));
	button = conf_bind_mask(bind, &mb->modmask);
	mb->press.button = strtonum(button, Button1, Button5, &errstr);
	if (errstr) {
		warnx("button number is %s: %s", errstr, button);
		free(mb);
		return 0;
	}
	conf_unbind_mouse(c, mb);
	if (cmd == nullptr) {
		free(mb);
		return 1;
	}
	cargs = (Cargs*)xcalloc(1, sizeof(*cargs));
	for (i = 0; i < nitems(name_to_func); i++) {
		if (strcmp(name_to_func[i].tag, cmd) != 0) continue;
		mb->callback = name_to_func[i].handler;
		mb->context = name_to_func[i].context;
		cargs->flag = name_to_func[i].flag;
		goto out;
	}
	mb->callback = kbfunc_exec_cmd;
	mb->context = Context::none;
	cargs->flag = 0;
	cargs->cmd = xstrdup(cmd);
out:
	mb->cargs = cargs;
	TAILQ_INSERT_TAIL(&c->mousebindq, mb, entry);
	return 1;
}

static void conf_unbind_mouse(struct Conf* c, Bind_ctx* unbind)
{
	Bind_ctx *mb = nullptr, *mbnxt;

	TAILQ_FOREACH_SAFE(mb, &c->mousebindq, entry, mbnxt)
	{
		if ((unbind == nullptr)
		    || ((mb->modmask == unbind->modmask) && (mb->press.button == unbind->press.button))) {
			TAILQ_REMOVE(&c->mousebindq, mb, entry);
			free(mb->cargs->cmd);
			free(mb->cargs);
			free(mb);
		}
	}
}

void conf_grab_kbd(Window win)
{
	Bind_ctx* kb;
	KeyCode kc;
	unsigned int i;

	XUngrabKey(X_Dpy, AnyKey, AnyModifier, win);

	TAILQ_FOREACH(kb, &conf.keybindq, entry)
	{
		kc = XKeysymToKeycode(X_Dpy, kb->press.keysym);
		if ((XkbKeycodeToKeysym(X_Dpy, kc, 0, 0) != kb->press.keysym)
		    && (XkbKeycodeToKeysym(X_Dpy, kc, 0, 1) == kb->press.keysym))
			kb->modmask |= ShiftMask;

		for (i = 0; i < nitems(ignore_mods); i++)
			XGrabKey(X_Dpy,
			         kc,
			         (kb->modmask | ignore_mods[i]),
			         win,
			         True,
			         GrabModeAsync,
			         GrabModeAsync);
	}
}

void conf_grab_mouse(Window win)
{
	Bind_ctx* mb;
	unsigned int i;

	XUngrabButton(X_Dpy, AnyButton, AnyModifier, win);

	TAILQ_FOREACH(mb, &conf.mousebindq, entry)
	{
		if (mb->context != Context::cc) continue;
		for (i = 0; i < nitems(ignore_mods); i++) {
			XGrabButton(X_Dpy,
			            mb->press.button,
			            (mb->modmask | ignore_mods[i]),
			            win,
			            False,
			            BUTTONMASK,
			            GrabModeAsync,
			            GrabModeSync,
			            None,
			            None);
		}
	}
}
