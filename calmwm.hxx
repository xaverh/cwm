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

#ifndef _CALMWM_H_
#define _CALMWM_H_

#include "queue.hxx"

#include <X11/XKBlib.h>
#include <X11/Xatom.h>
#include <X11/Xft/Xft.h>
#include <X11/Xlib.h>
#include <X11/Xproto.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#include <X11/extensions/Xrandr.h>
#include <X11/keysym.h>
#include <array>
#include <cstdio>
#include <filesystem>
#include <list>
#include <memory>
#include <string>
#include <string_view>
#include <sys/param.h>

using namespace std::string_view_literals;
using namespace std::string_literals;

/* prototypes for portable-included functions */
long long strtonum(char const*, long long, long long, char const**);
size_t strlcat(char*, char const*, size_t);
size_t strlcpy(char*, char const*, size_t);

template<typename T>
constexpr auto nitems(T const& i)
{
	return sizeof(i) / sizeof(i[0]);
}

static constexpr unsigned BUTTONMASK {ButtonPressMask | ButtonReleaseMask};
static constexpr unsigned MOUSEMASK {BUTTONMASK | PointerMotionMask};
static constexpr unsigned IGNOREMODMASK {LockMask | Mod2Mask | 0x2000};

/* direction/amount */
static constexpr unsigned CWM_UP {0x0001};
static constexpr unsigned CWM_DOWN {0x0002};
static constexpr unsigned CWM_LEFT {0x0004};
static constexpr unsigned CWM_RIGHT {0x0008};
static constexpr unsigned CWM_BIGAMOUNT {0x0010};
static constexpr unsigned CWM_UP_BIG {CWM_UP | CWM_BIGAMOUNT};
static constexpr unsigned CWM_DOWN_BIG {CWM_DOWN | CWM_BIGAMOUNT};
static constexpr unsigned CWM_LEFT_BIG {CWM_LEFT | CWM_BIGAMOUNT};
static constexpr unsigned CWM_RIGHT_BIG {CWM_RIGHT | CWM_BIGAMOUNT};
static constexpr unsigned CWM_UP_RIGHT {CWM_UP | CWM_RIGHT};
static constexpr unsigned CWM_UP_LEFT {CWM_UP | CWM_LEFT};
static constexpr unsigned CWM_DOWN_RIGHT {CWM_DOWN | CWM_RIGHT};
static constexpr unsigned CWM_DOWN_LEFT {CWM_DOWN | CWM_LEFT};
static constexpr unsigned CWM_CYCLE_FORWARD {0x0001};
static constexpr unsigned CWM_CYCLE_REVERSE {0x0002};
static constexpr unsigned CWM_CYCLE_INGROUP {0x0004};

enum Cwm_status { CWM_QUIT, CWM_RUNNING, CWM_EXEC_WM };

enum Cursor_font : std::size_t { CF_NORMAL, CF_MOVE, CF_RESIZE, CF_QUESTION, CF_NITEMS };

enum Color : std::size_t {
	CWM_COLOR_BORDER_ACTIVE,
	CWM_COLOR_BORDER_INACTIVE,
	CWM_COLOR_BORDER_URGENCY,
	CWM_COLOR_BORDER_GROUP,
	CWM_COLOR_BORDER_UNGROUP,
	CWM_COLOR_MENU_FG,
	CWM_COLOR_MENU_BG,
	CWM_COLOR_MENU_FONT,
	CWM_COLOR_MENU_FONT_SEL,
	CWM_COLOR_NITEMS
};

struct Geom {
	int x;
	int y;
	int w;
	int h;
};

struct Gap {
	int top;
	int bottom;
	int left;
	int right;
};

struct Winname {
	TAILQ_ENTRY(Winname) entry;
	char* name;
};

TAILQ_HEAD(name_q, Winname);
TAILQ_HEAD(ignore_q, Winname);

struct Client_ctx {
	TAILQ_ENTRY(Client_ctx) entry;
	struct Screen_ctx* sc;
	struct Group_ctx* gc;
	Window win;
	Colormap colormap;
	int bwidth;  /* border width */
	int obwidth; /* original border width */
	Geom geom, savegeom, fullgeom;
	struct {
		long flags; /* defined hints */
		int basew;  /* desired width */
		int baseh;  /* desired height */
		int minw;   /* minimum width */
		int minh;   /* minimum height */
		int maxw;   /* maximum width */
		int maxh;   /* maximum height */
		int incw;   /* width increment progression */
		int inch;   /* height increment progression */
		float mina; /* minimum aspect ratio */
		float maxa; /* maximum aspect ratio */
	} hint;
	struct {
		int x; /* x position */
		int y; /* y position */
	} ptr;
	struct {
		int h; /* height */
		int w; /* width */
	} dim;

	unsigned flags;
	int stackingorder;
	name_q nameq;
	char* name;
	char* label;
	char* res_class;   /* class hint */
	char* res_name;    /* class hint */
	int initial_state; /* wm hint */

	void apply_sizehints() noexcept;
	void close() const noexcept;
	void config() const;

	enum flag : unsigned {
		hidden = 0x0001,
		ignore = 0x0002,
		vmaximized = 0x0004,
		hmaximized = 0x0008,
		freeze = 0x0010,
		group = 0x0020,
		ungroup = 0x0040,
		input = 0x0080,
		wm_delete_window = 0x0100,
		wm_take_focus = 0x0200,
		urgency = 0x0400,
		fullscreen = 0x0800,
		sticky = 0x1000,
		active = 0x2000,
		skip_pager = 0x4000,
		skip_taskbar = 0x8000,
		skip_cycle = hidden | ignore | skip_taskbar | skip_pager,
		highlight = group | ungroup,
		maxflags = vmaximized | hmaximized,
		maximized = vmaximized | hmaximized
	};
};

TAILQ_HEAD(client_q, Client_ctx);

struct Group_ctx {
	TAILQ_ENTRY(Group_ctx) entry;
	struct Screen_ctx* sc;
	char* name;
	int num;
};

TAILQ_HEAD(group_q, Group_ctx);

struct Autogroup {
	TAILQ_ENTRY(Autogroup) entry;
	char* wclass;
	char* name;
	int num;
};

TAILQ_HEAD(autogroup_q, Autogroup);

struct Region_ctx {
	TAILQ_ENTRY(Region_ctx) entry;
	int num;
	Geom view; /* viewable area */
	Geom work; /* workable area, gap-applied */
};
TAILQ_HEAD(region_q, Region_ctx);

struct Screen_ctx {
	TAILQ_ENTRY(Screen_ctx) entry;
	int which;
	Window rootwin;
	int cycling;
	int hideall;
	int snapdist;
	Geom view; /* viewable area */
	Geom work; /* workable area, gap-applied */
	Gap gap;
	struct client_q clientq;
	struct region_q regionq;
	struct group_q groupq;
	Group_ctx* group_active;
	Colormap colormap;
	Visual* visual;
	struct {
		Window win;
		XftDraw* xftdraw;
	} prop;
	XftColor xftcolor[CWM_COLOR_NITEMS];
	XftFont* xftfont;
};

TAILQ_HEAD(screen_q, Screen_ctx);

namespace cwm {
enum class Xev { key, btn };
}

struct Cargs {
	char* cmd;
	int flag;
	cwm::Xev xev;
};

enum class Context { none, cc, sc };

struct Bind_ctx {
	TAILQ_ENTRY(Bind_ctx) entry;
	void (*callback)(void*, Cargs*);
	Cargs* cargs;
	Context context;
	unsigned int modmask;
	union {
		KeySym keysym;
		unsigned int button;
	} press;
};

TAILQ_HEAD(keybind_q, Bind_ctx);

TAILQ_HEAD(mousebind_q, Bind_ctx);

struct Cmd_ctx {
	TAILQ_ENTRY(Cmd_ctx) entry;
	char* name;
	char* path;
};

TAILQ_HEAD(cmd_q, Cmd_ctx);

static constexpr unsigned CWM_MENU_DUMMY {0x0001};
static constexpr unsigned CWM_MENU_FILE {0x0002};
static constexpr unsigned CWM_MENU_LIST {0x0004};
static constexpr unsigned CWM_MENU_WINDOW_ALL {0x0008};
static constexpr unsigned CWM_MENU_WINDOW_HIDDEN {0x0010};

static constexpr std::size_t MENU_MAXENTRY {200};

struct Menu {
	TAILQ_ENTRY(Menu) entry;
	TAILQ_ENTRY(Menu) resultentry;
	char text[MENU_MAXENTRY + 1];
	char print[MENU_MAXENTRY + 1];
	void* ctx;
	bool dummy;
	bool abort;
};

TAILQ_HEAD(menu_q, Menu);

struct Conf {
	Conf() noexcept;
	~Conf();
	Conf(Conf const&) = delete;
	Conf(Conf&&) = default;
	Conf& operator=(Conf const&) = delete;
	Conf& operator=(Conf&&) = default;
	keybind_q keybindq {nullptr, nullptr};
	mousebind_q mousebindq {nullptr, nullptr};
	autogroup_q autogroupq {nullptr, nullptr};
	ignore_q ignoreq {nullptr, nullptr};
	cmd_q cmdq {nullptr, nullptr};
	std::list<Cmd_ctx*> wmq;
	int ngroups {0};
	bool stickygroups {false};
	int nameqlen {5};
	int bwidth {1};
	int mamount {1};
	int snapdist {0};
	int htile {50};
	int vtile {50};
	Gap gap {};
	std::array<std::string, CWM_COLOR_NITEMS> color {
	    "#CCCCCC"s, // CWM_COLOR_BORDER_ACTIVE
	    "#666666"s, // CWM_COLOR_BORDER_INACTIVE
	    "#FC8814"s, // CWM_COLOR_BORDER_URGENCY
	    "blue"s,    // CWM_COLOR_BORDER_GROUP
	    "red"s,     // CWM_COLOR_BORDER_UNGROUP
	    "black"s,   // CWM_COLOR_MENU_FG
	    "white"s,   // CWM_COLOR_MENU_BG
	    "black"s,   // CWM_COLOR_MENU_FONT
	    ""s         // CWM_COLOR_MENU_FONT_SEL
	};
	std::string_view font {"sans-serif:pixelsize=14:bold"sv};
	std::string_view wmname {"CWM"sv};
	std::array<Cursor, CF_NITEMS> cursor {None, None, None, None};
	int xrandr {0};
	int xrandr_event_base {0};
	std::filesystem::path conf_file;
	std::filesystem::path known_hosts;
	char* wm_argv {nullptr};
	int debug {0};

	[[nodiscard]] int parse();
	void create_cursor();
	void autogroup(int, char const*, char const*);
	[[nodiscard]] int bind_key(char const*, char const*);
	void unbind_key(Bind_ctx*);
	[[nodiscard]] int bind_mouse(char const*, char const*);
	void unbind_mouse(Bind_ctx*);
	void cmd_add(char const*, char const*);
	void wm_add(char const*, char const*);
	void ignore(char const*);
};

/* MWM hints */
static constexpr auto MWM_HINTS_ELEMENTS {3};
static constexpr auto MWM_FLAGS_FUNCTIONS {1 << 0};
static constexpr auto MWM_FLAGS_DECORATIONS {1 << 1};
static constexpr auto MWM_FLAGS_INPUT_MODE {1 << 2};
static constexpr auto MWM_FLAGS_STATUS {1 << 3};
static constexpr auto MWM_FUNCS_ALL {1 << 0};
static constexpr auto MWM_FUNCS_RESIZE {1 << 1};
static constexpr auto MWM_FUNCS_MOVE {1 << 2};
static constexpr auto MWM_FUNCS_MINIMIZE {1 << 3};
static constexpr auto MWM_FUNCS_MAXIMIZE {1 << 4};
static constexpr auto MWM_FUNCS_CLOSE {1 << 5};
static constexpr auto MWM_DECOR_ALL {1 << 0};
static constexpr auto MWM_DECOR_BORDER {1 << 1};
static constexpr auto MWM_DECOR_RESIZE_HANDLE {1 << 2};
static constexpr auto MWM_DECOR_TITLEBAR {1 << 3};
static constexpr auto MWM_DECOR_MENU {1 << 4};
static constexpr auto MWM_DECOR_MINIMIZE {1 << 5};
static constexpr auto MWM_DECOR_MAXIMIZE {1 << 6};

struct Mwm_hints {
	unsigned long flags;
	unsigned long functions;
	unsigned long decorations;
};

enum Cwmh : std::size_t {
	WM_STATE,
	WM_DELETE_WINDOW,
	WM_TAKE_FOCUS,
	WM_PROTOCOLS,
	_MOTIF_WM_HINTS,
	UTF8_STRING,
	WM_CHANGE_STATE,
	CWMH_NITEMS
};

static constexpr auto _NET_WM_STATES_NITEMS {9};

enum Ewmh {
	_NET_SUPPORTED,
	_NET_SUPPORTING_WM_CHECK,
	_NET_ACTIVE_WINDOW,
	_NET_CLIENT_LIST,
	_NET_CLIENT_LIST_STACKING,
	_NET_NUMBER_OF_DESKTOPS,
	_NET_CURRENT_DESKTOP,
	_NET_DESKTOP_VIEWPORT,
	_NET_DESKTOP_GEOMETRY,
	_NET_VIRTUAL_ROOTS,
	_NET_SHOWING_DESKTOP,
	_NET_DESKTOP_NAMES,
	_NET_WORKAREA,
	_NET_WM_NAME,
	_NET_WM_DESKTOP,
	_NET_CLOSE_WINDOW,
	_NET_WM_STATE,
	_NET_WM_STATE_STICKY,
	_NET_WM_STATE_MAXIMIZED_VERT,
	_NET_WM_STATE_MAXIMIZED_HORZ,
	_NET_WM_STATE_HIDDEN,
	_NET_WM_STATE_FULLSCREEN,
	_NET_WM_STATE_DEMANDS_ATTENTION,
	_NET_WM_STATE_SKIP_PAGER,
	_NET_WM_STATE_SKIP_TASKBAR,
	_CWM_WM_STATE_FREEZE,
	EWMH_NITEMS
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

enum net_wm_state { _NET_WM_STATE_REMOVE, _NET_WM_STATE_ADD, _NET_WM_STATE_TOGGLE };

extern Display* X_Dpy;
extern Time Last_Event_Time;
extern std::array<Atom, CWMH_NITEMS> cwmh;
extern std::array<Atom, EWMH_NITEMS> ewmh;
extern struct screen_q Screenq;
extern std::unique_ptr<struct Conf> conf;

void usage();

Client_ctx* client_current(Screen_ctx*);
void client_draw_border(Client_ctx*);
Client_ctx* client_find(Window);
void client_get_sizehints(Client_ctx*);
void client_hide(Client_ctx*);
void client_htile(Client_ctx*);
int client_inbound(Client_ctx*, int, int);
Client_ctx* client_init(Window, Screen_ctx*);
void client_lower(Client_ctx*);
void client_move(Client_ctx*);
void client_mtf(Client_ctx*);
Client_ctx* client_next(Client_ctx*);
Client_ctx* client_prev(Client_ctx*);
void client_ptr_inbound(Client_ctx*, int);
void client_ptr_save(Client_ctx*);
void client_ptr_warp(Client_ctx*);
void client_raise(Client_ctx*);
void client_remove(Client_ctx*);
void client_resize(Client_ctx*, int);
void client_set_active(Client_ctx*);
void client_set_name(Client_ctx*);
void client_show(Client_ctx*);
int client_snapcalc(int, int, int, int, int);
void client_toggle_hidden(Client_ctx*);
void client_toggle_hmaximize(Client_ctx*);
void client_toggle_fullscreen(Client_ctx*);
void client_toggle_freeze(Client_ctx*);
void client_toggle_maximize(Client_ctx*);
void client_toggle_skip_pager(Client_ctx*);
void client_toggle_skip_taskbar(Client_ctx*);
void client_toggle_sticky(Client_ctx*);
void client_toggle_vmaximize(Client_ctx*);
void client_transient(Client_ctx*);
void client_urgency(Client_ctx*);
void client_vtile(Client_ctx*);
void client_wm_hints(Client_ctx*);

void group_assign(Group_ctx*, Client_ctx*);
int group_autogroup(Client_ctx*);
void group_cycle(Screen_ctx*, int);
void group_hide(Group_ctx*);
int group_holds_only_hidden(Group_ctx*);
int group_holds_only_sticky(Group_ctx*);
void group_init(Screen_ctx*, int, char const*);
void group_movetogroup(Client_ctx*, int);
void group_only(Screen_ctx*, int);
void group_close(Screen_ctx*, int);
int group_restore(Client_ctx*);
void group_show(Group_ctx*);
void group_toggle(Screen_ctx*, int);
void group_toggle_all(Screen_ctx*);
void group_toggle_membership(Client_ctx*);
void group_update_names(Screen_ctx*);

void search_match_client(struct menu_q*, struct menu_q*, char*);
void search_match_cmd(struct menu_q*, struct menu_q*, char*);
void search_match_exec(struct menu_q*, struct menu_q*, char*);
void search_match_group(struct menu_q*, struct menu_q*, char*);
void search_match_path(struct menu_q*, struct menu_q*, char*);
void search_match_text(struct menu_q*, struct menu_q*, char*);
void search_match_wm(struct menu_q*, struct menu_q*, char*);
void search_print_client(Menu*, int);
void search_print_cmd(Menu*, int);
void search_print_group(Menu*, int);
void search_print_text(Menu*, int);
void search_print_wm(Menu*, int);

Region_ctx* region_find(Screen_ctx*, int, int);
void screen_assert_clients_within(Screen_ctx*);
Geom screen_area(Screen_ctx*, int, int, int);
Screen_ctx* screen_find(Window);
void screen_init(int);
void screen_prop_win_create(Screen_ctx*, Window);
void screen_prop_win_destroy(Screen_ctx*);
void screen_prop_win_draw(Screen_ctx*, char const*, ...) __attribute__((__format__(printf, 2, 3)))
__attribute__((__nonnull__(2)));
void screen_update_geometry(Screen_ctx*);
void screen_updatestackingorder(Screen_ctx*);

void kbfunc_cwm_status(void*, Cargs*);
void kbfunc_ptrmove(void*, Cargs*);
void kbfunc_client_snap(void*, Cargs*);
void kbfunc_client_move(void*, Cargs*);
void kbfunc_client_resize(void*, Cargs*);
void kbfunc_client_close(void*, Cargs*);
void kbfunc_client_lower(void*, Cargs*);
void kbfunc_client_raise(void*, Cargs*);
void kbfunc_client_hide(void*, Cargs*);
void kbfunc_client_toggle_freeze(void*, Cargs*);
void kbfunc_client_toggle_sticky(void*, Cargs*);
void kbfunc_client_toggle_fullscreen(void*, Cargs*);
void kbfunc_client_toggle_maximize(void*, Cargs*);
void kbfunc_client_toggle_hmaximize(void*, Cargs*);
void kbfunc_client_toggle_vmaximize(void*, Cargs*);
void kbfunc_client_htile(void*, Cargs*);
void kbfunc_client_vtile(void*, Cargs*);
void kbfunc_client_cycle(void*, Cargs*);
void kbfunc_client_toggle_group(void*, Cargs*);
void kbfunc_client_movetogroup(void*, Cargs*);
void kbfunc_group_toggle(void*, Cargs*);
void kbfunc_group_only(void*, Cargs*);
void kbfunc_group_close(void*, Cargs*);
void kbfunc_group_cycle(void*, Cargs*);
void kbfunc_group_toggle_all(void*, Cargs*);
void kbfunc_menu_client(void*, Cargs*);
void kbfunc_menu_cmd(void*, Cargs*);
void kbfunc_menu_group(void*, Cargs*);
void kbfunc_menu_wm(void*, Cargs*);
void kbfunc_menu_exec(void*, Cargs*);
void kbfunc_menu_ssh(void*, Cargs*);
void kbfunc_client_menu_label(void*, Cargs*);
void kbfunc_exec_cmd(void*, Cargs*);
void kbfunc_exec_lock(void*, Cargs*);
void kbfunc_exec_term(void*, Cargs*);

Menu* menu_filter(Screen_ctx*,
                  struct menu_q*,
                  char const*,
                  char const*,
                  int,
                  void (*)(struct menu_q*, struct menu_q*, char*),
                  void (*)(Menu*, int));
void menuq_add(struct menu_q*, void*, char const*, ...) __attribute__((__format__(printf, 3, 4)));
void menuq_clear(struct menu_q*);

void conf_clear(struct Conf*);
void conf_client(Client_ctx*);
void conf_grab_kbd(Window);
void conf_grab_mouse(Window);
void conf_screen(Screen_ctx*);
void conf_group(Screen_ctx*);

void xev_process();

int xu_get_prop(Window, Atom, Atom, long, unsigned char**);
int xu_get_strprop(Window, Atom, char**);
void xu_ptr_get(Window, int*, int*);
void xu_ptr_set(Window, int, int);
void xu_get_wm_state(Window, long*);
void xu_set_wm_state(Window, long);
void xu_send_clientmsg(Window, Atom, Time);
void xu_xorcolor(XftColor, XftColor, XftColor*);

void xu_atom_init();
void xu_ewmh_net_supported(Screen_ctx*);
void xu_ewmh_net_supported_wm_check(Screen_ctx*);
void xu_ewmh_net_desktop_geometry(Screen_ctx*);
void xu_ewmh_net_desktop_viewport(Screen_ctx*);
void xu_ewmh_net_workarea(Screen_ctx*);
void xu_ewmh_net_client_list(Screen_ctx*);
void xu_ewmh_net_client_list_stacking(Screen_ctx*);
void xu_ewmh_net_active_window(Screen_ctx*, Window);
void xu_ewmh_net_number_of_desktops(Screen_ctx*);
void xu_ewmh_net_showing_desktop(Screen_ctx*);
void xu_ewmh_net_virtual_roots(Screen_ctx*);
void xu_ewmh_net_current_desktop(Screen_ctx*);
void xu_ewmh_net_desktop_names(Screen_ctx*);
int xu_ewmh_get_net_wm_desktop(Client_ctx*, long*);
void xu_ewmh_set_net_wm_desktop(Client_ctx*);
Atom* xu_ewmh_get_net_wm_state(Client_ctx*, int*);
void xu_ewmh_handle_net_wm_state_msg(Client_ctx*, int, Atom, Atom);
void xu_ewmh_set_net_wm_state(Client_ctx*);
void xu_ewmh_restore_net_wm_state(Client_ctx*);

char* u_argv(char* const*);
void u_exec(char*);
void u_spawn(char*);
void log_debug(int, char const*, char const*, ...) __attribute__((__format__(printf, 3, 4)))
__attribute__((__nonnull__(3)));

#endif /* _CALMWM_H_ */
