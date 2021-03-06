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
#include <sys/types.h>
#include <unistd.h>

static char const* conf_bind_mask(char const*, unsigned int*);

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

Conf::Conf() noexcept
{
	ignoreq.tqh_last = &(ignoreq).tqh_first;
	autogroupq.tqh_last = &(autogroupq).tqh_first;
	keybindq.tqh_last = &(keybindq).tqh_first;
	mousebindq.tqh_last = &(mousebindq).tqh_first;
	cmdq.tqh_last = &(cmdq).tqh_first;

	cmd_add("lock", "xlock");
	cmd_add("term", "xterm");
	wm_add("cwm", "cwm");

	for (auto const& k : key_binds) this->bind_key(k.key, k.func);
	for (auto const& m : mouse_binds) this->bind_mouse(m.key, m.func);

	std::filesystem::path home;
	if (std::string_view home_sv {getenv("HOME")}; !home_sv.empty()) {
		home = home_sv;
	} else {
		home = "/";
	}

	conf_file = home / ".cwmrc";
	known_hosts = home / ".ssh/known_hosts";
}

Conf::~Conf()
{
	Autogroup* ag;
	Bind_ctx *kb, *mb;
	Winname* wn;
	Cmd_ctx* cmd;

	while ((cmd = TAILQ_FIRST(&cmdq)) != nullptr) {
		TAILQ_REMOVE(&cmdq, cmd, entry);
		std::free(cmd->name);
		std::free(cmd->path);
		std::free(cmd);
	}
	while ((kb = TAILQ_FIRST(&keybindq)) != nullptr) {
		TAILQ_REMOVE(&keybindq, kb, entry);
		std::free(kb);
	}
	while ((ag = TAILQ_FIRST(&autogroupq)) != nullptr) {
		TAILQ_REMOVE(&autogroupq, ag, entry);
		std::free(ag->wclass);
		std::free(ag->name);
		std::free(ag);
	}
	while ((wn = TAILQ_FIRST(&ignoreq)) != nullptr) {
		TAILQ_REMOVE(&ignoreq, wn, entry);
		std::free(wn->name);
		std::free(wn);
	}
	while ((mb = TAILQ_FIRST(&mousebindq)) != nullptr) {
		TAILQ_REMOVE(&mousebindq, mb, entry);
		std::free(mb);
	}
}

void Conf::cmd_add(char const* name, char const* path)
{
	Cmd_ctx *cmd, *cmdtmp = nullptr, *cmdnxt;

	cmd = static_cast<Cmd_ctx*>(std::malloc(sizeof(*cmd)));
	cmd->name = strdup(name);
	cmd->path = strdup(path);

	TAILQ_FOREACH_SAFE(cmdtmp, &cmdq, entry, cmdnxt)
	{
		if (strcmp(cmdtmp->name, name) == 0) {
			TAILQ_REMOVE(&cmdq, cmdtmp, entry);
			std::free(cmdtmp->name);
			std::free(cmdtmp->path);
			std::free(cmdtmp);
		}
	}
	TAILQ_INSERT_TAIL(&cmdq, cmd, entry);
}

void Conf::wm_add(char const* name, char const* path)
{
	Cmd_ctx *wm, *wmtmp = nullptr, *wmnxt;

	wm = static_cast<Cmd_ctx*>(std::malloc(sizeof(*wm)));
	wm->name = strdup(name);
	wm->path = strdup(path);

	TAILQ_FOREACH_SAFE(wmtmp, &cmdq, entry, wmnxt) // [FIXME] cmdq, really_
	{
		if (strcmp(wmtmp->name, name) == 0) {
			wmq.remove(wmtmp);
			std::free(wmtmp->name);
			std::free(wmtmp->path);
			std::free(wmtmp);
		}
	}
	wmq.push_back(wm);
}

void Conf::autogroup(int num, char const* name, char const* wclass)
{
	Autogroup* ag;
	char* p;

	ag = static_cast<Autogroup*>(std::malloc(sizeof(*ag)));
	if ((p = const_cast<char*>(strchr(wclass, ','))) == nullptr) {
		if (name == nullptr)
			ag->name = nullptr;
		else
			ag->name = strdup(name);

		ag->wclass = strdup(wclass);
	} else {
		*(p++) = '\0';
		if (name == nullptr)
			ag->name = strdup(wclass);
		else
			ag->name = strdup(name);

		ag->wclass = strdup(p);
	}
	ag->num = num;
	TAILQ_INSERT_TAIL(&autogroupq, ag, entry);
}

void Conf::ignore(char const* name)
{
	Winname* wn;

	wn = (Winname*)std::malloc(sizeof(*wn));
	wn->name = strdup(name);
	TAILQ_INSERT_TAIL(&ignoreq, wn, entry);
}

void Conf::create_cursor()
{
	for (std::size_t i {0}; i < nitems(cursor_binds); ++i)
		cursor[i] = XCreateFontCursor(X_Dpy, cursor_binds[i]);
}

void conf_client(Client_ctx* cc)
{
	Winname* wn;

	TAILQ_FOREACH(wn, &conf->ignoreq, entry)
	{
		if (strncasecmp(wn->name, cc->name, strlen(wn->name)) == 0) {
			cc->flags |= Client_ctx::ignore;
			break;
		}
	}
}

void conf_screen(Screen_ctx* sc)
{
	XftColor xc;

	sc->gap = conf->gap;
	sc->snapdist = conf->snapdist;

	sc->xftfont = XftFontOpenXlfd(X_Dpy, sc->which, conf->font.data());
	if (sc->xftfont == nullptr) {
		sc->xftfont = XftFontOpenName(X_Dpy, sc->which, conf->font.data());
		if (sc->xftfont == nullptr) errx(1, "%s: XftFontOpenName: %s", __func__, conf->font.data());
	}

	for (std::size_t i = 0; i < nitems(color_binds); ++i) {
		if (i == CWM_COLOR_MENU_FONT_SEL && conf->color[i].empty()) {
			xu_xorcolor(sc->xftcolor[CWM_COLOR_MENU_BG], sc->xftcolor[CWM_COLOR_MENU_FG], &xc);
			xu_xorcolor(sc->xftcolor[CWM_COLOR_MENU_FONT], xc, &xc);
			if (!XftColorAllocValue(X_Dpy,
			                        sc->visual,
			                        sc->colormap,
			                        &xc.color,
			                        &sc->xftcolor[CWM_COLOR_MENU_FONT_SEL]))
				warnx("XftColorAllocValue: %s", conf->color[i].c_str());
			break;
		}
		if (!XftColorAllocName(X_Dpy,
		                       sc->visual,
		                       sc->colormap,
		                       conf->color[i].c_str(),
		                       &sc->xftcolor[i])) {
			warnx("XftColorAllocName: %s", conf->color[i].c_str());
			XftColorAllocName(X_Dpy, sc->visual, sc->colormap, color_binds[i], &sc->xftcolor[i]);
		}
	}

	conf_grab_kbd(sc->rootwin);
}

void conf_group(Screen_ctx* sc)
{
	for (auto const& g : group_binds) {
		group_init(sc, g.num, g.name);
		++conf->ngroups;
	}
}

static char const* conf_bind_mask(char const* name, unsigned int* mask)
{
	char const* dash;
	*mask = 0;
	if ((dash = strchr(name, '-')) == nullptr) return name;
	for (auto const& b : bind_mods) {
		if (auto ch = strchr(name, b.ch); ch != nullptr && ch < dash) *mask |= b.mask;
	}
	/* Skip past modifiers. */
	return (dash + 1);
}

int Conf::bind_key(char const* bind, char const* cmd)
{
	Bind_ctx* kb;
	Cargs* cargs;
	char const* key;
	unsigned int i;

	if ((strcmp(bind, "all") == 0) && (cmd == nullptr)) {
		unbind_key(nullptr);
		return 1;
	}
	kb = (Bind_ctx*)std::malloc(sizeof(*kb));
	key = conf_bind_mask(bind, &kb->modmask);
	kb->press.keysym = XStringToKeysym(key);
	if (kb->press.keysym == NoSymbol) {
		warnx("unknown symbol: %s", key);
		free(kb);
		return 0;
	}
	unbind_key(kb);
	if (cmd == nullptr) {
		free(kb);
		return 1;
	}
	cargs = (Cargs*)calloc(1, sizeof(*cargs));
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
	cargs->cmd = strdup(cmd);
out:
	kb->cargs = cargs;
	TAILQ_INSERT_TAIL(&keybindq, kb, entry);
	return 1;
}

void Conf::unbind_key(Bind_ctx* unbind)
{
	Bind_ctx *key = nullptr, *keynxt;

	TAILQ_FOREACH_SAFE(key, &keybindq, entry, keynxt)
	{
		if ((unbind == nullptr)
		    || ((key->modmask == unbind->modmask) && (key->press.keysym == unbind->press.keysym))) {
			TAILQ_REMOVE(&keybindq, key, entry);
			free(key->cargs->cmd);
			free(key->cargs);
			free(key);
		}
	}
}

int Conf::bind_mouse(char const* bind, char const* cmd)
{
	Bind_ctx* mb;
	Cargs* cargs;
	char const *button, *errstr;
	unsigned int i;

	if ((strcmp(bind, "all") == 0) && (cmd == nullptr)) {
		this->unbind_mouse(nullptr);
		return 1;
	}
	mb = (Bind_ctx*)std::malloc(sizeof(*mb));
	button = conf_bind_mask(bind, &mb->modmask);
	mb->press.button = strtonum(button, Button1, Button5, &errstr);
	if (errstr) {
		warnx("button number is %s: %s", errstr, button);
		free(mb);
		return 0;
	}
	unbind_mouse(mb);
	if (cmd == nullptr) {
		free(mb);
		return 1;
	}
	cargs = (Cargs*)calloc(1, sizeof(*cargs));
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
	cargs->cmd = strdup(cmd);
out:
	mb->cargs = cargs;
	TAILQ_INSERT_TAIL(&mousebindq, mb, entry);
	return 1;
}

void Conf::unbind_mouse(Bind_ctx* unbind)
{
	Bind_ctx *mb = nullptr, *mbnxt;

	TAILQ_FOREACH_SAFE(mb, &mousebindq, entry, mbnxt)
	{
		if ((unbind == nullptr)
		    || ((mb->modmask == unbind->modmask) && (mb->press.button == unbind->press.button))) {
			TAILQ_REMOVE(&mousebindq, mb, entry);
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

	TAILQ_FOREACH(kb, &conf->keybindq, entry)
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

	TAILQ_FOREACH(mb, &conf->mousebindq, entry)
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
