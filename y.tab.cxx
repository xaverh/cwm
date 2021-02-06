#include "calmwm.hxx"
#include "queue.hxx"

#include <cassert>
#include <cctype>
#include <cerrno>
#include <climits>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <err.h>
#include <sys/types.h>

TAILQ_HEAD(files, file) files = TAILQ_HEAD_INITIALIZER(files);
static struct file {
	TAILQ_ENTRY(file) entry;
	FILE* stream;
	char* name;
	int lineno;
	int errors;
} * file, *topfile;
struct file* pushfile(char const*, FILE*);
int popfile();
int yyparse();
int yylex();
int yyerror(char const* /*fmt*/, ...) __attribute__((__format__(printf, 1, 2)))
__attribute__((__nonnull__(1)));
int kw_cmp(const void*, const void*);
int lookup(char*);
int lgetc(int);
int lungetc(int);
int findeol();

struct YYSTYPE {
	union {
		int64_t number;
		char* string;
	} v;
	int lineno;
};

enum yytokentype {
	YYEMPTY = -2,
	YYEOF = 0,
	YYerror = 256,
	YYUNDEF = 257,
	BINDKEY = 258,
	UNBINDKEY = 259,
	BINDMOUSE = 260,
	UNBINDMOUSE = 261,
	FONTNAME = 262,
	STICKY = 263,
	GAP = 264,
	AUTOGROUP = 265,
	COMMAND = 266,
	IGNORE = 267,
	WM = 268,
	YES = 269,
	NO = 270,
	BORDERWIDTH = 271,
	MOVEAMOUNT = 272,
	HTILE = 273,
	VTILE = 274,
	COLOR = 275,
	SNAPDIST = 276,
	ACTIVEBORDER = 277,
	INACTIVEBORDER = 278,
	URGENCYBORDER = 279,
	GROUPBORDER = 280,
	UNGROUPBORDER = 281,
	MENUBG = 282,
	MENUFG = 283,
	FONTCOLOR = 284,
	FONTSELCOLOR = 285,
	ERROR = 286,
	STRING = 287,
	NUMBER = 288
};

/* Value type.  */
extern YYSTYPE yylval;

/* Symbol kind.  */
enum yysymbol_kind_t {
	YYSYMBOL_YYEMPTY = -2,
	YYSYMBOL_YYEOF = 0,           /* "end of file"  */
	YYSYMBOL_YYerror = 1,         /* error  */
	YYSYMBOL_YYUNDEF = 2,         /* "invalid token"  */
	YYSYMBOL_BINDKEY = 3,         /* BINDKEY  */
	YYSYMBOL_UNBINDKEY = 4,       /* UNBINDKEY  */
	YYSYMBOL_BINDMOUSE = 5,       /* BINDMOUSE  */
	YYSYMBOL_UNBINDMOUSE = 6,     /* UNBINDMOUSE  */
	YYSYMBOL_FONTNAME = 7,        /* FONTNAME  */
	YYSYMBOL_STICKY = 8,          /* STICKY  */
	YYSYMBOL_GAP = 9,             /* GAP  */
	YYSYMBOL_AUTOGROUP = 10,      /* AUTOGROUP  */
	YYSYMBOL_COMMAND = 11,        /* COMMAND  */
	YYSYMBOL_IGNORE = 12,         /* IGNORE  */
	YYSYMBOL_WM = 13,             /* WM  */
	YYSYMBOL_YES = 14,            /* YES  */
	YYSYMBOL_NO = 15,             /* NO  */
	YYSYMBOL_BORDERWIDTH = 16,    /* BORDERWIDTH  */
	YYSYMBOL_MOVEAMOUNT = 17,     /* MOVEAMOUNT  */
	YYSYMBOL_HTILE = 18,          /* HTILE  */
	YYSYMBOL_VTILE = 19,          /* VTILE  */
	YYSYMBOL_COLOR = 20,          /* COLOR  */
	YYSYMBOL_SNAPDIST = 21,       /* SNAPDIST  */
	YYSYMBOL_ACTIVEBORDER = 22,   /* ACTIVEBORDER  */
	YYSYMBOL_INACTIVEBORDER = 23, /* INACTIVEBORDER  */
	YYSYMBOL_URGENCYBORDER = 24,  /* URGENCYBORDER  */
	YYSYMBOL_GROUPBORDER = 25,    /* GROUPBORDER  */
	YYSYMBOL_UNGROUPBORDER = 26,  /* UNGROUPBORDER  */
	YYSYMBOL_MENUBG = 27,         /* MENUBG  */
	YYSYMBOL_MENUFG = 28,         /* MENUFG  */
	YYSYMBOL_FONTCOLOR = 29,      /* FONTCOLOR  */
	YYSYMBOL_FONTSELCOLOR = 30,   /* FONTSELCOLOR  */
	YYSYMBOL_ERROR = 31,          /* ERROR  */
	YYSYMBOL_STRING = 32,         /* STRING  */
	YYSYMBOL_NUMBER = 33,         /* NUMBER  */
	YYSYMBOL_34_newline = 34,     /* '\n'  */
	YYSYMBOL_35_comma = 35,       /* ','  */
	YYSYMBOL_YYACCEPT = 36,       /* $accept  */
	YYSYMBOL_grammar = 37,        /* grammar  */
	YYSYMBOL_string = 38,         /* string  */
	YYSYMBOL_yesno = 39,          /* yesno  */
	YYSYMBOL_main = 40,           /* main  */
	YYSYMBOL_color = 41,          /* color  */
	YYSYMBOL_colors = 42          /* colors  */
};

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

/* YYFINAL -- State number of the termination state.  */
static constexpr int YYFINAL {2};
/* YYLAST -- Last index in YYTABLE.  */
static constexpr int YYLAST {74};
/* YYNTOKENS -- Number of terminals.  */
static constexpr int YYNTOKENS {36};
/* YYNSTATES -- Number of states.  */
static constexpr int YYNSTATES {76};

static constexpr int_least8_t yytranslate[]
    = {0,  2,  2,  2,  2,  2,  2,  2,  2,  2,  34, 2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
       2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  35, 2,
       2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
       2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
       2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
       2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
       2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
       2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
       2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
       2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
       2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
       2,  2,  2,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
       21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static constexpr int_least8_t yypact[] = {
    -33, 0,   -33, -32, -17, -9,  5,   6,   7,   21,  8,   9,   11,  12,  13,  14,  15,  16,  17,
    3,   18,  -33, 19,  20,  -33, 23,  -33, 23,  -33, -33, -33, -33, -33, 24,  26,  23,  -33, 23,
    -33, -33, -33, -33, 27,  28,  29,  30,  31,  32,  33,  34,  35,  -33, -33, -33, -33, -33, 36,
    36,  37,  38,  36,  36,  -33, -33, -33, -33, -33, -33, -33, -33, -33, -33, 39,  42,  -33, -33};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static constexpr int_least8_t yydefact[]
    = {2,  0,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 0, 0,  0,
       0,  0,  3,  0,  0,  6,  0,  25, 0,  27, 11, 9,  10, 12, 0,  0, 0, 22, 0,
       13, 16, 14, 15, 0,  0,  0,  0,  0,  0,  0,  0,  0,  28, 17, 4, 5, 8,  24,
       26, 0,  20, 18, 19, 29, 30, 31, 32, 33, 34, 35, 36, 37, 7,  0, 0, 23, 21};

/* YYPGOTO[NTERM-NUM].  */
static constexpr int_least8_t yypgoto[] = {-33, -33, -13, -33, -33, -33, -33};

/* YYDEFGOTO[NTERM-NUM].  */
static constexpr int_least8_t yydefgoto[] = {-1, 1, 56, 32, 22, 23, 51};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static constexpr int_least8_t yytable[]
    = {2,  3,  24, 4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 57, 25, 15, 16, 17,
       18, 19, 20, 60, 26, 61, 42, 43, 44, 45, 46, 47, 48, 49, 50, 21, 30, 31, 27,
       28, 29, 0,  33, 34, 35, 36, 37, 0,  38, 39, 40, 41, 52, 0,  53, 54, 55, 0,
       58, 59, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 0,  72, 0,  74, 73, 75};

static constexpr int_least8_t yycheck[]
    = {0,  1,  34, 3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 27, 32, 16, 17, 18,
       19, 20, 21, 35, 32, 37, 22, 23, 24, 25, 26, 27, 28, 29, 30, 34, 14, 15, 32,
       32, 32, -1, 33, 33, 32, 32, 32, -1, 33, 33, 33, 33, 33, -1, 34, 34, 32, -1,
       33, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, -1, 33, -1, 33, 35, 32};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static constexpr int_least8_t yystos[]
    = {0,  37, 0,  1,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 16, 17, 18, 19,
       20, 21, 34, 40, 41, 34, 32, 32, 32, 32, 32, 14, 15, 39, 33, 33, 32, 32, 32,
       33, 33, 33, 33, 22, 23, 24, 25, 26, 27, 28, 29, 30, 42, 33, 34, 34, 32, 38,
       38, 33, 32, 38, 38, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 33, 35, 33, 32};

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static constexpr int_least8_t yyr1[]
    = {0,  36, 37, 37, 37, 37, 37, 38, 38, 39, 39, 40, 40, 40, 40, 40, 40, 40, 40,
       40, 40, 40, 40, 40, 40, 40, 40, 40, 41, 42, 42, 42, 42, 42, 42, 42, 42, 42};

/* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static constexpr int_least8_t yyr2[] = {0, 2, 0, 2, 3, 3, 3, 2, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 3,
                                        3, 3, 5, 2, 5, 3, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2};

enum { YYENOMEM = -2 };

/* YYINITDEPTH -- initial size of the parser's stacks.  */
static constexpr auto YYINITDEPTH {200};

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void yydestruct(char const* yymsg,
                       [[maybe_unused]] yysymbol_kind_t yykind,
                       [[maybe_unused]] YYSTYPE* yyvaluep)
{
	if (!yymsg) yymsg = "Deleting";
}

/* Lookahead token kind.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;

/*----------.
| yyparse.  |
`----------*/

int yyparse()
{
	auto yystate = 0;
	/* Number of tokens to shift before error messages enabled.  */
	int yyerrstatus = 0;

	/* Refer to the stacks through separate pointers, to allow yyoverflow
	   to reallocate them elsewhere.  */

	/* The state stack: array, bottom, top.  */
	int_least8_t yyssa[YYINITDEPTH];
	int_least8_t* yyss = yyssa;
	int_least8_t* yyssp = yyss;

	/* The semantic value stack: array, bottom, top.  */
	YYSTYPE yyvsa[YYINITDEPTH];
	YYSTYPE* yyvs = yyvsa;
	YYSTYPE* yyvsp = yyvs;

	int yyn;
	/* The return value of yyparse.  */
	int yyresult;
	/* Lookahead symbol kind.  */
	yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
	/* The variables used to return semantic value and location from the
	   action routines.  */
	YYSTYPE yyval;

	/* The number of symbols on the RHS of the reduced rule.
	   Keep to zero when no symbol should be popped.  */
	int yylen = 0;

	yychar = YYEMPTY; /* Cause a token to be read.  */
	goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
	/* In all cases, when you get here, the value and location stacks
	   have just been pushed.  So pushing a state here evens the stacks.  */
	yyssp++;

/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
	assert(0 <= yystate && yystate < YYNSTATES);
	*yyssp = static_cast<int_least8_t>(yystate);
	if (yyss + YYINITDEPTH - 1 <= yyssp) goto yyexhaustedlab;
	if (yystate == YYFINAL) goto yyacceptlab;
	goto yybackup;

yybackup:
	/* Do appropriate processing given the current state.  Read a
	   lookahead token if we need one and don't already have one.  */

	/* First try to decide what to do without reference to lookahead token.  */
	yyn = yypact[yystate];
	if (yyn == -33) goto yydefault;

	/* Not known => get a lookahead token if don't already have one.  */

	/* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
	if (yychar == YYEMPTY) yychar = yylex();

	if (yychar <= YYEOF) {
		yychar = YYEOF;
		yytoken = YYSYMBOL_YYEOF;
	} else if (yychar == YYerror) {
		/* The scanner already issued an error message, process directly
		   to error recovery.  But do not keep the error token as
		   lookahead, it is too special and may lead us to an endless
		   loop in error recovery. */
		yychar = YYUNDEF;
		yytoken = YYSYMBOL_YYerror;
		goto yyerrlab1;
	} else {
		yytoken
		    = (0 <= (yychar) && (yychar) <= 288 ? static_cast<yysymbol_kind_t>(yytranslate[yychar])
		                                        : YYSYMBOL_YYUNDEF);
	}

	/* If the proper action on seeing token YYTOKEN is to reduce or to
	   detect an error, take that action.  */
	yyn += yytoken;
	if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken) goto yydefault;
	yyn = yytable[yyn];
	if (yyn <= 0) {
		yyn = -yyn;
		goto yyreduce;
	}

	/* Count tokens shifted since error; after three, turn off error
	   status.  */
	if (yyerrstatus) yyerrstatus--;

	/* Shift the lookahead token.  */
	yystate = yyn;

	*++yyvsp = yylval;

	/* Discard the shifted token.  */
	yychar = YYEMPTY;
	goto yynewstate;

/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
	yyn = yydefact[yystate];
	if (yyn == 0) goto yyerrlab;
	goto yyreduce;

/*-----------------------------.
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
	/* yyn is the number of a rule to reduce with.  */
	yylen = yyr2[yyn];

	/* If YYLEN is nonzero, implement the default value of the action:
	   '$$ = $1'.
	   Otherwise, the following line sets YYVAL to garbage.
	   This behavior is undocumented and Bison
	   users should not rely upon it.  Assigning to YYVAL
	   unconditionally makes the parser a bit smaller, and it avoids a
	   GCC warning that YYVAL may be used uninitialized.  */
	yyval = yyvsp[1 - yylen];

	switch (yyn) {
	case 6: /* grammar: grammar error '\n'  */ file->errors++; break;
	case 7: /* string: string STRING  */
		if (asprintf(&(yyval.v.string), "%s %s", (yyvsp[-1].v.string), (yyvsp[0].v.string)) == -1) {
			std::free(yyvsp[-1].v.string);
			std::free(yyvsp[0].v.string);
			yyerror("string: asprintf");
			goto yyerrorlab;
		}
		std::free(yyvsp[-1].v.string);
		std::free(yyvsp[0].v.string);
		break;
	case 9: /* yesno: YES  */ (yyval.v.number) = 1; break;
	case 10: /* yesno: NO  */ (yyval.v.number) = 0; break;
	case 11: /* main: FONTNAME STRING  */ conf->font = (yyvsp[0].v.string); break;
	case 12: /* main: STICKY yesno  */ conf->stickygroups = (yyvsp[0].v.number); break;
	case 13: /* main: BORDERWIDTH NUMBER  */
		if (yyvsp[0].v.number < 0 || yyvsp[0].v.number > INT_MAX) {
			yyerror("invalid borderwidth");
			goto yyerrorlab;
		}
		conf->bwidth = (yyvsp[0].v.number);
		break;
	case 14: /* main: HTILE NUMBER  */
		if (yyvsp[0].v.number < 0 || yyvsp[0].v.number > 99) {
			yyerror("invalid htile percent");
			goto yyerrorlab;
		}
		conf->htile = (yyvsp[0].v.number);
		break;
	case 15: /* main: VTILE NUMBER  */
		if (yyvsp[0].v.number < 0 || yyvsp[0].v.number > 99) {
			yyerror("invalid vtile percent");
			goto yyerrorlab;
		}
		conf->vtile = (yyvsp[0].v.number);
		break;
	case 16: /* main: MOVEAMOUNT NUMBER  */
		if (yyvsp[0].v.number < 0 || yyvsp[0].v.number > INT_MAX) {
			yyerror("invalid movemount");
			goto yyerrorlab;
		}
		conf->mamount = (yyvsp[0].v.number);
		break;
	case 17: /* main: SNAPDIST NUMBER  */
		if (yyvsp[0].v.number < 0 || yyvsp[0].v.number > INT_MAX) {
			yyerror("invalid snapdist");
			goto yyerrorlab;
		}
		conf->snapdist = (yyvsp[0].v.number);
		break;
	case 18: /* main: COMMAND STRING string  */
		if (strlen(yyvsp[0].v.string) >= PATH_MAX) {
			yyerror("%s command path too long", (yyvsp[-1].v.string));
			std::free(yyvsp[-1].v.string);
			std::free(yyvsp[0].v.string);
			goto yyerrorlab;
		}
		conf->cmd_add(yyvsp[-1].v.string, yyvsp[0].v.string);
		std::free(yyvsp[-1].v.string);
		std::free(yyvsp[0].v.string);
		break;
	case 19: /* main: WM STRING string  */
		if (strlen((yyvsp[0].v.string)) >= PATH_MAX) {
			yyerror("%s wm path too long", (yyvsp[-1].v.string));
			std::free(yyvsp[-1].v.string);
			std::free(yyvsp[0].v.string);
			goto yyerrorlab;
		}
		conf->wm_add(yyvsp[-1].v.string, yyvsp[0].v.string);
		std::free(yyvsp[-1].v.string);
		std::free(yyvsp[0].v.string);
		break;
	case 20: /* main: AUTOGROUP NUMBER STRING  */
		if ((yyvsp[-1].v.number) < 0 || (yyvsp[-1].v.number) > 9) {
			yyerror("invalid autogroup");
			std::free(yyvsp[0].v.string);
			goto yyerrorlab;
		}
		conf->autogroup(yyvsp[-1].v.number, nullptr, yyvsp[0].v.string);
		std::free(yyvsp[0].v.string);
		break;
	case 21: /* main: AUTOGROUP NUMBER STRING ',' STRING  */
		if ((yyvsp[-3].v.number) < 0 || (yyvsp[-3].v.number) > 9) {
			yyerror("invalid autogroup");
			std::free(yyvsp[-2].v.string);
			std::free(yyvsp[0].v.string);
			goto yyerrorlab;
		}
		conf->autogroup(yyvsp[-3].v.number, yyvsp[-2].v.string, yyvsp[0].v.string);
		std::free(yyvsp[-2].v.string);
		std::free(yyvsp[0].v.string);
		break;
	case 22: /* main: IGNORE STRING  */
		conf->ignore(yyvsp[0].v.string);
		std::free(yyvsp[0].v.string);
		break;
	case 23: /* main: GAP NUMBER NUMBER NUMBER NUMBER  */
		if ((yyvsp[-3].v.number) < 0 || (yyvsp[-3].v.number) > INT_MAX || (yyvsp[-2].v.number) < 0
		    || (yyvsp[-2].v.number) > INT_MAX || (yyvsp[-1].v.number) < 0
		    || (yyvsp[-1].v.number) > INT_MAX || (yyvsp[0].v.number) < 0
		    || (yyvsp[0].v.number) > INT_MAX) {
			yyerror("invalid gap");
			goto yyerrorlab;
		}
		conf->gap.top = yyvsp[-3].v.number;
		conf->gap.bottom = yyvsp[-2].v.number;
		conf->gap.left = yyvsp[-1].v.number;
		conf->gap.right = yyvsp[0].v.number;
		break;
	case 24: /* main: BINDKEY STRING string  */
		if (!conf->bind_key(yyvsp[-1].v.string, yyvsp[0].v.string)) {
			yyerror("invalid bind-key: %s %s", yyvsp[-1].v.string, yyvsp[0].v.string);
			std::free(yyvsp[-1].v.string);
			std::free(yyvsp[0].v.string);
			goto yyerrorlab;
		}
		std::free(yyvsp[-1].v.string);
		std::free(yyvsp[0].v.string);
		break;
	case 25: /* main: UNBINDKEY STRING  */
		if (!conf->bind_key(yyvsp[0].v.string, nullptr)) {
			yyerror("invalid unbind-key: %s", yyvsp[0].v.string);
			std::free(yyvsp[0].v.string);
			goto yyerrorlab;
		}
		std::free(yyvsp[0].v.string);
		break;
	case 26: /* main: BINDMOUSE STRING string  */
		if (!conf->bind_mouse(yyvsp[-1].v.string, yyvsp[0].v.string)) {
			yyerror("invalid bind-mouse: %s %s", yyvsp[-1].v.string, yyvsp[0].v.string);
			std::free(yyvsp[-1].v.string);
			std::free(yyvsp[0].v.string);
			goto yyerrorlab;
		}
		std::free(yyvsp[-1].v.string);
		std::free(yyvsp[0].v.string);
		break;
	case 27: /* main: UNBINDMOUSE STRING  */
		if (!conf->bind_mouse((yyvsp[0].v.string), nullptr)) {
			yyerror("invalid unbind-mouse: %s", yyvsp[0].v.string);
			std::free(yyvsp[0].v.string);
			goto yyerrorlab;
		}
		std::free(yyvsp[0].v.string);
		break;
	case 29: /* colors: ACTIVEBORDER STRING  */
		conf->color[CWM_COLOR_BORDER_ACTIVE] = (yyvsp[0].v.string);
		break;
	case 30: /* colors: INACTIVEBORDER STRING  */
		conf->color[CWM_COLOR_BORDER_INACTIVE] = (yyvsp[0].v.string);
		break;
	case 31: /* colors: URGENCYBORDER STRING  */
		conf->color[CWM_COLOR_BORDER_URGENCY] = (yyvsp[0].v.string);
		break;
	case 32: /* colors: GROUPBORDER STRING  */
		conf->color[CWM_COLOR_BORDER_GROUP] = (yyvsp[0].v.string);
		break;
	case 33: /* colors: UNGROUPBORDER STRING  */
		conf->color[CWM_COLOR_BORDER_UNGROUP] = (yyvsp[0].v.string);
		break;
	case 34: /* colors: MENUBG STRING  */
		conf->color[CWM_COLOR_MENU_BG] = (yyvsp[0].v.string);
		break;
	case 35: /* colors: MENUFG STRING  */
		conf->color[CWM_COLOR_MENU_FG] = (yyvsp[0].v.string);
		break;
	case 36: /* colors: FONTCOLOR STRING  */
		conf->color[CWM_COLOR_MENU_FONT] = (yyvsp[0].v.string);
		break;
	case 37: /* colors: FONTSELCOLOR STRING  */
		conf->color[CWM_COLOR_MENU_FONT_SEL] = (yyvsp[0].v.string);
		break;
	default: break;
	}
	/* User semantic actions sometimes alter yychar, and that requires
	   that yytoken be updated with the new translation.  We take the
	   approach of translating immediately before every use of yytoken.
	   One alternative is translating here after every semantic action,
	   but that translation would be missed if the semantic action invokes
	   yyabortlab, yyacceptlab, or yyerrorlab immediately after altering yychar or
	   if it invokes YYBACKUP.  In the case of yyabortlab or yyacceptlab, an
	   incorrect destructor might then be invoked immediately.  In the
	   case of yyerrorlab or YYBACKUP, subsequent parser actions might lead
	   to an incorrect destructor call or verbose syntax error message
	   before the lookahead is translated.  */

	yyvsp -= yylen, yyssp -= yylen;
	yylen = 0;

	*++yyvsp = yyval;

	/* Now 'shift' the result of the reduction.  Determine what state
	   that goes to, based on the state we popped back to and the rule
	   number reduced by.  */
	{
		const int yylhs = yyr1[yyn] - YYNTOKENS;
		const int yyi = yypgoto[yylhs] + *yyssp;
		yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp ? yytable[yyi]
		                                                               : yydefgoto[yylhs]);
	}

	goto yynewstate;

/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
	/* Make sure we have latest lookahead translation.  See comments at
	   user semantic actions for why this is necessary.  */
	yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY
	                            : (0 <= (yychar) && (yychar) <= 288
	                                   ? static_cast<yysymbol_kind_t>(yytranslate[yychar])
	                                   : YYSYMBOL_YYUNDEF);
	/* If not already recovering from an error, report this error.  */
	if (!yyerrstatus) {
		++yynerrs;
		yyerror("syntax error");
	}

	if (yyerrstatus == 3) {
		/* If just tried and failed to reuse lookahead token after an
		   error, discard it.  */

		if (yychar <= YYEOF) {
			/* Return failure if at end of input.  */
			if (yychar == YYEOF) goto yyabortlab;
		} else {
			yydestruct("Error: discarding", yytoken, &yylval);
			yychar = YYEMPTY;
		}
	}

	/* Else will try to reuse lookahead token after shifting the error
	   token.  */
	goto yyerrlab1;

yyerrorlab:
	// Do not reclaim the symbols of the rule whose action triggered this yyerrorlab.
	yyvsp -= yylen, yyssp -= yylen;
	yylen = 0;
	yystate = *yyssp;
	goto yyerrlab1;

/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and yyerrorlab|
`-------------------------------------------------------------*/
yyerrlab1:
	yyerrstatus = 3; /* Each real token shifted decrements this.  */

	/* Pop stack until we find a state that shifts the error token.  */
	for (;;) {
		yyn = yypact[yystate];
		if (yyn != -33) {
			yyn += YYSYMBOL_YYerror;
			if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror) {
				yyn = yytable[yyn];
				if (0 < yyn) break;
			}
		}

		/* Pop the current state because it cannot handle the error token.  */
		if (yyssp == yyss) goto yyabortlab;

		yydestruct("Error: popping", static_cast<yysymbol_kind_t>(yystos[yystate]), yyvsp);
		(yyvsp -= (1), yyssp -= (1));
		yystate = *yyssp;
	}

	*++yyvsp = yylval;

	yystate = yyn;
	goto yynewstate;

/*-------------------------------------.
| yyacceptlab -- yyacceptlab comes here.  |
`-------------------------------------*/
yyacceptlab:
	yyresult = 0;
	goto yyreturn;

/*-----------------------------------.
| yyabortlab -- yyabortlab comes here.  |
`-----------------------------------*/
yyabortlab:
	yyresult = 1;
	goto yyreturn;

/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
	yyerror("memory exhausted");
	yyresult = 2;
	goto yyreturn;

/*-------------------------------------------------------.
| yyreturn -- parsing is finished, clean up and return.  |
`-------------------------------------------------------*/
yyreturn:
	if (yychar != YYEMPTY) {
		/* Make sure we have latest lookahead translation.  See comments at
		   user semantic actions for why this is necessary.  */
		yytoken
		    = (0 <= (yychar) && (yychar) <= 288 ? static_cast<yysymbol_kind_t>(yytranslate[yychar])
		                                        : YYSYMBOL_YYUNDEF);
		yydestruct("Cleanup: discarding lookahead", yytoken, &yylval);
	}
	/* Do not reclaim the symbols of the rule whose action triggered
	   this yyabortlab or yyacceptlab.  */
	(yyvsp -= (yylen), yyssp -= (yylen));
	while (yyssp != yyss) {
		yydestruct("Cleanup: popping", static_cast<yysymbol_kind_t>(yystos[+*yyssp]), yyvsp);
		(yyvsp -= (1), yyssp -= (1));
	}
	if (yyss != yyssa) std::free(yyss);

	return yyresult;
}

struct keywords {
	char const* k_name;
	int k_val;
};

int yyerror(char const* fmt, ...)
{
	va_list ap;

	file->errors++;
	va_start(ap, fmt);
	fprintf(stderr, "%s:%d: ", file->name, yylval.lineno);
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	va_end(ap);
	return (0);
}

int kw_cmp(const void* k, const void* e)
{
	return (strcmp((char const*)k, ((const struct keywords*)e)->k_name));
}

int lookup(char* s)
{
	/* this has to be sorted always */
	static const struct keywords keywords[] = {{"activeborder", ACTIVEBORDER},
	                                           {"autogroup", AUTOGROUP},
	                                           {"bind-key", BINDKEY},
	                                           {"bind-mouse", BINDMOUSE},
	                                           {"borderwidth", BORDERWIDTH},
	                                           {"color", COLOR},
	                                           {"command", COMMAND},
	                                           {"font", FONTCOLOR},
	                                           {"fontname", FONTNAME},
	                                           {"gap", GAP},
	                                           {"groupborder", GROUPBORDER},
	                                           {"htile", HTILE},
	                                           {"ignore", IGNORE},
	                                           {"inactiveborder", INACTIVEBORDER},
	                                           {"menubg", MENUBG},
	                                           {"menufg", MENUFG},
	                                           {"moveamount", MOVEAMOUNT},
	                                           {"no", NO},
	                                           {"selfont", FONTSELCOLOR},
	                                           {"snapdist", SNAPDIST},
	                                           {"sticky", STICKY},
	                                           {"unbind-key", UNBINDKEY},
	                                           {"unbind-mouse", UNBINDMOUSE},
	                                           {"ungroupborder", UNGROUPBORDER},
	                                           {"urgencyborder", URGENCYBORDER},
	                                           {"vtile", VTILE},
	                                           {"wm", WM},
	                                           {"yes", YES}};
	const struct keywords* p;

	p = (struct keywords const*)
	    bsearch(s, keywords, sizeof(keywords) / sizeof(keywords[0]), sizeof(keywords[0]), kw_cmp);

	if (p)
		return (p->k_val);
	else
		return (STRING);
}

static constexpr std::size_t MAXPUSHBACK {128};

char* parsebuf;
int parseindex;
char pushback_buffer[MAXPUSHBACK];
std::size_t pushback_index {0};

int lgetc(int quotec)
{
	int c, next;

	if (parsebuf) {
		/* Read character from the parsebuffer instead of input. */
		if (parseindex >= 0) {
			c = parsebuf[parseindex++];
			if (c != '\0') return (c);
			parsebuf = nullptr;
		} else
			parseindex++;
	}

	if (pushback_index) return (pushback_buffer[--pushback_index]);

	if (quotec) {
		if ((c = getc(file->stream)) == EOF) {
			yyerror("reached end of file while parsing "
			        "quoted string");
			if (file == topfile || popfile() == EOF) return (EOF);
			return (quotec);
		}
		return (c);
	}

	while ((c = getc(file->stream)) == '\\') {
		next = getc(file->stream);
		if (next != '\n') {
			c = next;
			break;
		}
		yylval.lineno = file->lineno;
		file->lineno++;
	}

	while (c == EOF) {
		if (file == topfile || popfile() == EOF) return (EOF);
		c = getc(file->stream);
	}
	return (c);
}

int lungetc(int c)
{
	if (c == EOF) return (EOF);
	if (parsebuf) {
		parseindex--;
		if (parseindex >= 0) return (c);
	}
	if (pushback_index < MAXPUSHBACK - 1)
		return (pushback_buffer[pushback_index++] = c);
	else
		return (EOF);
}

int findeol()
{
	int c;

	parsebuf = nullptr;

	/* skip to either EOF or the first real EOL */
	while (true) {
		if (pushback_index)
			c = pushback_buffer[--pushback_index];
		else
			c = lgetc(0);
		if (c == '\n') {
			file->lineno++;
			break;
		}
		if (c == EOF) break;
	}
	return (ERROR);
}

int yylex()
{
	char buf[8096];
	char* p;
	int quotec, next, c;
	int token;

	p = buf;
	while ((c = lgetc(0)) == ' ' || c == '\t')
		; /* nothing */

	yylval.lineno = file->lineno;
	if (c == '#')
		while ((c = lgetc(0)) != '\n' && c != EOF)
			; /* nothing */

	switch (c) {
	case '\'':
	case '"':
		quotec = c;
		while (true) {
			if ((c = lgetc(quotec)) == EOF) return (0);
			if (c == '\n') {
				file->lineno++;
				continue;
			} else if (c == '\\') {
				if ((next = lgetc(quotec)) == EOF) return (0);
				if (next == quotec || next == ' ' || next == '\t')
					c = next;
				else if (next == '\n') {
					file->lineno++;
					continue;
				} else
					lungetc(next);
			} else if (c == quotec) {
				*p = '\0';
				break;
			} else if (c == '\0') {
				yyerror("syntax error");
				return (findeol());
			}
			if (p + 1 >= buf + sizeof(buf) - 1) {
				yyerror("string too long");
				return (findeol());
			}
			*p++ = (char)c;
		}
		yylval.v.string = xstrdup(buf);
		return (STRING);
	}

	if (c == '-' || isdigit(c)) {
		do {
			*p++ = c;
			if ((unsigned)(p - buf) >= sizeof(buf)) {
				yyerror("string too long");
				return (findeol());
			}
		} while ((c = lgetc(0)) != EOF && isdigit(c));
		lungetc(c);
		if (p == buf + 1 && buf[0] == '-') goto nodigits;
		if (c == EOF || (isspace(c) || c == ')' || c == ',' || c == '/' || c == '}' || c == '=')) {
			char const* errstr = nullptr;

			*p = '\0';
			yylval.v.number = strtonum(buf, LLONG_MIN, LLONG_MAX, &errstr);
			if (errstr) {
				yyerror("\"%s\" invalid number: %s", buf, errstr);
				return (findeol());
			}
			return (NUMBER);
		} else {
nodigits:
			while (p > buf + 1) lungetc(*--p);
			c = *--p;
			if (c == '-') return (c);
		}
	}

	if (isalnum(c) || c == ':' || c == '_' || c == '*' || c == '/') {
		do {
			*p++ = c;
			if ((unsigned)(p - buf) >= sizeof(buf)) {
				yyerror("string too long");
				return (findeol());
			}
		} while ((c = lgetc(0)) != EOF
		         && ((isalnum(c)
		              || (ispunct(c) && c != '(' && c != ')' && c != '{' && c != '}' && c != '<'
		                  && c != '>' && c != '!' && c != '=' && c != '#' && c != ','))));
		lungetc(c);
		*p = '\0';
		if ((token = lookup(buf)) == STRING) yylval.v.string = xstrdup(buf);
		return (token);
	}
	if (c == '\n') {
		yylval.lineno = file->lineno;
		file->lineno++;
	}
	if (c == EOF) return (0);
	return (c);
}

struct file* pushfile(char const* name, FILE* stream)
{
	struct file* nfile;

	nfile = (struct file*)xcalloc(1, sizeof(struct file));
	nfile->name = xstrdup(name);
	nfile->stream = stream;
	nfile->lineno = 1;
	TAILQ_INSERT_TAIL(&files, nfile, entry);
	return (nfile);
}

int popfile()
{
	struct file* prev;

	if ((prev = TAILQ_PREV(file, files, entry)) != nullptr) prev->errors += file->errors;

	TAILQ_REMOVE(&files, file, entry);
	fclose(file->stream);
	std::free(file->name);
	std::free(file);
	file = prev;
	return (file ? 0 : EOF);
}

int Conf::parse()
{
	int errors = 0;

	FILE* stream = fopen(conf_file.c_str(), "r");
	if (stream == nullptr) {
		if (errno == ENOENT) return (0);
		warn("%s", conf_file.c_str());
		return (-1);
	}
	file = pushfile(conf_file.c_str(), stream);
	topfile = file;

	yyparse();
	errors = file->errors;
	popfile();

	return (errors ? -1 : 0);
}
