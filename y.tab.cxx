/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "3.7.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* First part of user prologue.  */
#line 22 "parse.y"

#include "calmwm.hxx"
#include "queue.hxx"

#include <cctype>
#include <cerrno>
#include <climits>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <err.h>
#include <sys/types.h>

#define YYSTYPE_IS_DECLARED

TAILQ_HEAD(files, file) files = TAILQ_HEAD_INITIALIZER(files);
static struct file {
	TAILQ_ENTRY(file) entry;
	FILE* stream;
	char* name;
	int lineno;
	int errors;
} * file, *topfile;
struct file* pushfile(const char*, FILE*);
int popfile(void);
int yyparse(void);
int yylex(void);
int yyerror(const char*, ...) __attribute__((__format__(printf, 1, 2)))
__attribute__((__nonnull__(1)));
int kw_cmp(const void*, const void*);
int lookup(char*);
int lgetc(int);
int lungetc(int);
int findeol(void);

static struct Conf* conf;

typedef struct {
	union {
		int64_t number;
		char* string;
	} v;
	int lineno;
} YYSTYPE;

#line 122 "y.tab.c"

#ifndef YY_CAST
#define YY_CAST(Type, Val) static_cast<Type>(Val)
#define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type>(Val)
#endif

/* Debug traces.  */
#ifndef YYDEBUG
#define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token kinds.  */
#ifndef YYTOKENTYPE
#define YYTOKENTYPE
enum yytokentype {
	YYEMPTY = -2,
	YYEOF = 0,            /* "end of file"  */
	YYerror = 256,        /* error  */
	YYUNDEF = 257,        /* "invalid token"  */
	BINDKEY = 258,        /* BINDKEY  */
	UNBINDKEY = 259,      /* UNBINDKEY  */
	BINDMOUSE = 260,      /* BINDMOUSE  */
	UNBINDMOUSE = 261,    /* UNBINDMOUSE  */
	FONTNAME = 262,       /* FONTNAME  */
	STICKY = 263,         /* STICKY  */
	GAP = 264,            /* GAP  */
	AUTOGROUP = 265,      /* AUTOGROUP  */
	COMMAND = 266,        /* COMMAND  */
	IGNORE = 267,         /* IGNORE  */
	WM = 268,             /* WM  */
	YES = 269,            /* YES  */
	NO = 270,             /* NO  */
	BORDERWIDTH = 271,    /* BORDERWIDTH  */
	MOVEAMOUNT = 272,     /* MOVEAMOUNT  */
	HTILE = 273,          /* HTILE  */
	VTILE = 274,          /* VTILE  */
	COLOR = 275,          /* COLOR  */
	SNAPDIST = 276,       /* SNAPDIST  */
	ACTIVEBORDER = 277,   /* ACTIVEBORDER  */
	INACTIVEBORDER = 278, /* INACTIVEBORDER  */
	URGENCYBORDER = 279,  /* URGENCYBORDER  */
	GROUPBORDER = 280,    /* GROUPBORDER  */
	UNGROUPBORDER = 281,  /* UNGROUPBORDER  */
	MENUBG = 282,         /* MENUBG  */
	MENUFG = 283,         /* MENUFG  */
	FONTCOLOR = 284,      /* FONTCOLOR  */
	FONTSELCOLOR = 285,   /* FONTSELCOLOR  */
	ERROR = 286,          /* ERROR  */
	STRING = 287,         /* STRING  */
	NUMBER = 288          /* NUMBER  */
};
typedef enum yytokentype yytoken_kind_t;
#endif
/* Token kinds.  */
#define YYEOF 0
#define YYerror 256
#define YYUNDEF 257
#define BINDKEY 258
#define UNBINDKEY 259
#define BINDMOUSE 260
#define UNBINDMOUSE 261
#define FONTNAME 262
#define STICKY 263
#define GAP 264
#define AUTOGROUP 265
#define COMMAND 266
#define IGNORE 267
#define WM 268
#define YES 269
#define NO 270
#define BORDERWIDTH 271
#define MOVEAMOUNT 272
#define HTILE 273
#define VTILE 274
#define COLOR 275
#define SNAPDIST 276
#define ACTIVEBORDER 277
#define INACTIVEBORDER 278
#define URGENCYBORDER 279
#define GROUPBORDER 280
#define UNGROUPBORDER 281
#define MENUBG 282
#define MENUFG 283
#define FONTCOLOR 284
#define FONTSELCOLOR 285
#define ERROR 286
#define STRING 287
#define NUMBER 288

/* Value type.  */

extern YYSTYPE yylval;

int yyparse(void);

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
	YYSYMBOL_34_n_ = 34,          /* '\n'  */
	YYSYMBOL_35_ = 35,            /* ','  */
	YYSYMBOL_YYACCEPT = 36,       /* $accept  */
	YYSYMBOL_grammar = 37,        /* grammar  */
	YYSYMBOL_string = 38,         /* string  */
	YYSYMBOL_yesno = 39,          /* yesno  */
	YYSYMBOL_main = 40,           /* main  */
	YYSYMBOL_color = 41,          /* color  */
	YYSYMBOL_colors = 42          /* colors  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;

#ifdef short
#undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <climits> and (if available) <cstdint> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
#include <climits> /* INFRINGES ON USER NAME SPACE */
#if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#include <cstdint> /* INFRINGES ON USER NAME SPACE */
#define YY_STDINT_H
#endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
#if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#define YYPTRDIFF_T __PTRDIFF_TYPE__
#define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
#elif defined PTRDIFF_MAX
#ifndef ptrdiff_t
#include <cstddef> /* INFRINGES ON USER NAME SPACE */
#endif
#define YYPTRDIFF_T ptrdiff_t
#define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
#else
#define YYPTRDIFF_T long
#define YYPTRDIFF_MAXIMUM LONG_MAX
#endif
#endif

#ifndef YYSIZE_T
#ifdef __SIZE_TYPE__
#define YYSIZE_T __SIZE_TYPE__
#elif defined size_t
#define YYSIZE_T size_t
#elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#include <cstddef> /* INFRINGES ON USER NAME SPACE */
#define YYSIZE_T size_t
#else
#define YYSIZE_T unsigned
#endif
#endif

#define YYSIZE_MAXIMUM                                                                             \
	YY_CAST(                                                                                       \
	    YYPTRDIFF_T,                                                                               \
	    (YYPTRDIFF_MAXIMUM < YY_CAST(YYSIZE_T, -1) ? YYPTRDIFF_MAXIMUM : YY_CAST(YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST(YYPTRDIFF_T, sizeof(X))

/* Stored state numbers (used for stacks). */
typedef yytype_int8 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
#if defined YYENABLE_NLS && YYENABLE_NLS
#if ENABLE_NLS
#include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#define YY_(Msgid) dgettext("bison-runtime", Msgid)
#endif
#endif
#ifndef YY_
#define YY_(Msgid) Msgid
#endif
#endif

#ifndef YY_ATTRIBUTE_PURE
#if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#define YY_ATTRIBUTE_PURE __attribute__((__pure__))
#else
#define YY_ATTRIBUTE_PURE
#endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
#if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#define YY_ATTRIBUTE_UNUSED __attribute__((__unused__))
#else
#define YY_ATTRIBUTE_UNUSED
#endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if !defined lint || defined __GNUC__
#define YYUSE(E) ((void)(E))
#else
#define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && !defined __ICC && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                                                        \
	_Pragma("GCC diagnostic push") _Pragma("GCC diagnostic ignored \"-Wuninitialized\"")           \
	    _Pragma("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
#define YY_IGNORE_MAYBE_UNINITIALIZED_END _Pragma("GCC diagnostic pop")
#else
#define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
#define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
#define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
#define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && !defined __ICC && 6 <= __GNUC__
#define YY_IGNORE_USELESS_CAST_BEGIN                                                               \
	_Pragma("GCC diagnostic push") _Pragma("GCC diagnostic ignored \"-Wuseless-cast\"")
#define YY_IGNORE_USELESS_CAST_END _Pragma("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
#define YY_IGNORE_USELESS_CAST_BEGIN
#define YY_IGNORE_USELESS_CAST_END
#endif

#define YY_ASSERT(E) ((void)(0 && (E)))

#if !defined yyoverflow

/* The parser invokes alloca or malloc; define the necessary symbols.  */

#ifdef YYSTACK_USE_ALLOCA
#if YYSTACK_USE_ALLOCA
#ifdef __GNUC__
#define YYSTACK_ALLOC __builtin_alloca
#elif defined __BUILTIN_VA_ARG_INCR
#include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#elif defined _AIX
#define YYSTACK_ALLOC __alloca
#elif defined _MSC_VER
#include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#define alloca _alloca
#else
#define YYSTACK_ALLOC alloca
#if !defined _ALLOCA_H && !defined EXIT_SUCCESS
#include <cstdlib> /* INFRINGES ON USER NAME SPACE */
/* Use EXIT_SUCCESS as a witness for cstdlib.  */
#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS 0
#endif
#endif
#endif
#endif
#endif

#ifdef YYSTACK_ALLOC
/* Pacify GCC's 'empty if-body' warning.  */
#define YYSTACK_FREE(Ptr)                                                                          \
	do { /* empty */                                                                               \
		;                                                                                          \
	} while (0)
#ifndef YYSTACK_ALLOC_MAXIMUM
/* The OS might guarantee only one guard page at the bottom of the stack,
   and a page size can be as small as 4096 bytes.  So we cannot safely
   invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
   to allow for a few compiler-allocated temporary stack slots.  */
#define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#endif
#else
#define YYSTACK_ALLOC YYMALLOC
#define YYSTACK_FREE YYFREE
#ifndef YYSTACK_ALLOC_MAXIMUM
#define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#endif
#if (defined __cplusplus && !defined EXIT_SUCCESS                                                  \
     && !((defined YYMALLOC || defined malloc) && (defined YYFREE || defined free)))
#include <cstdlib> /* INFRINGES ON USER NAME SPACE */
#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS 0
#endif
#endif
#ifndef YYMALLOC
#define YYMALLOC malloc
#if !defined malloc && !defined EXIT_SUCCESS
void* malloc(YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#endif
#endif
#ifndef YYFREE
#define YYFREE free
#if !defined free && !defined EXIT_SUCCESS
void free(void*);       /* INFRINGES ON USER NAME SPACE */
#endif
#endif
#endif
#endif /* !defined yyoverflow */

#if (!defined yyoverflow                                                                           \
     && (!defined __cplusplus || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc {
	yy_state_t yyss_alloc;
	YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
#define YYSTACK_GAP_MAXIMUM (YYSIZEOF(union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
#define YYSTACK_BYTES(N) ((N) * (YYSIZEOF(yy_state_t) + YYSIZEOF(YYSTYPE)) + YYSTACK_GAP_MAXIMUM)

#define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
#define YYSTACK_RELOCATE(Stack_alloc, Stack)                                                       \
	do {                                                                                           \
		YYPTRDIFF_T yynewbytes;                                                                    \
		YYCOPY(&yyptr->Stack_alloc, Stack, yysize);                                                \
		Stack = &yyptr->Stack_alloc;                                                               \
		yynewbytes = yystacksize * YYSIZEOF(*Stack) + YYSTACK_GAP_MAXIMUM;                         \
		yyptr += yynewbytes / YYSIZEOF(*yyptr);                                                    \
	} while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
#ifndef YYCOPY
#if defined __GNUC__ && 1 < __GNUC__
#define YYCOPY(Dst, Src, Count)                                                                    \
	__builtin_memcpy(Dst, Src, YY_CAST(YYSIZE_T, (Count)) * sizeof(*(Src)))
#else
#define YYCOPY(Dst, Src, Count)                                                                    \
	do {                                                                                           \
		YYPTRDIFF_T yyi;                                                                           \
		for (yyi = 0; yyi < (Count); yyi++) (Dst)[yyi] = (Src)[yyi];                               \
	} while (0)
#endif
#endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL 2
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST 74

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS 36
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS 7
/* YYNRULES -- Number of rules.  */
#define YYNRULES 37
/* YYNSTATES -- Number of states.  */
#define YYNSTATES 76

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK 288

/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                                                           \
	(0 <= (YYX) && (YYX) <= YYMAXUTOK ? YY_CAST(yysymbol_kind_t, yytranslate[YYX])                 \
	                                  : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[]
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

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] = {
    0,   89,  89,  90,  91,  92,  93,  96,  106, 109, 110, 113, 117, 120, 127, 134, 141, 148, 155,
    166, 177, 186, 197, 201, 214, 224, 232, 242, 252, 255, 259, 263, 267, 271, 275, 279, 283, 287};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST(yysymbol_kind_t, yystos[State])

#if YYDEBUG || 0
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char* yysymbol_name(yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char* const yytname[] = {"\"end of file\"",
                                      "error",
                                      "\"invalid token\"",
                                      "BINDKEY",
                                      "UNBINDKEY",
                                      "BINDMOUSE",
                                      "UNBINDMOUSE",
                                      "FONTNAME",
                                      "STICKY",
                                      "GAP",
                                      "AUTOGROUP",
                                      "COMMAND",
                                      "IGNORE",
                                      "WM",
                                      "YES",
                                      "NO",
                                      "BORDERWIDTH",
                                      "MOVEAMOUNT",
                                      "HTILE",
                                      "VTILE",
                                      "COLOR",
                                      "SNAPDIST",
                                      "ACTIVEBORDER",
                                      "INACTIVEBORDER",
                                      "URGENCYBORDER",
                                      "GROUPBORDER",
                                      "UNGROUPBORDER",
                                      "MENUBG",
                                      "MENUFG",
                                      "FONTCOLOR",
                                      "FONTSELCOLOR",
                                      "ERROR",
                                      "STRING",
                                      "NUMBER",
                                      "'\\n'",
                                      "','",
                                      "$accept",
                                      "grammar",
                                      "string",
                                      "yesno",
                                      "main",
                                      "color",
                                      "colors",
                                      nullptr};

static const char* yysymbol_name(yysymbol_kind_t yysymbol)
{
	return yytname[yysymbol];
}
#endif

#ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_int16 yytoknum[]
    = {0,   256, 257, 258, 259, 260, 261, 262, 263, 264, 265, 266, 267, 268, 269, 270, 271, 272,
       273, 274, 275, 276, 277, 278, 279, 280, 281, 282, 283, 284, 285, 286, 287, 288, 10,  44};
#endif

#define YYPACT_NINF (-33)

#define yypact_value_is_default(Yyn) ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1)

#define yytable_value_is_error(Yyn) 0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int8 yypact[] = {
    -33, 0,   -33, -32, -17, -9,  5,   6,   7,   21,  8,   9,   11,  12,  13,  14,  15,  16,  17,
    3,   18,  -33, 19,  20,  -33, 23,  -33, 23,  -33, -33, -33, -33, -33, 24,  26,  23,  -33, 23,
    -33, -33, -33, -33, 27,  28,  29,  30,  31,  32,  33,  34,  35,  -33, -33, -33, -33, -33, 36,
    36,  37,  38,  36,  36,  -33, -33, -33, -33, -33, -33, -33, -33, -33, -33, 39,  42,  -33, -33};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int8 yydefact[]
    = {2,  0,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 0, 0,  0,
       0,  0,  3,  0,  0,  6,  0,  25, 0,  27, 11, 9,  10, 12, 0,  0, 0, 22, 0,
       13, 16, 14, 15, 0,  0,  0,  0,  0,  0,  0,  0,  0,  28, 17, 4, 5, 8,  24,
       26, 0,  20, 18, 19, 29, 30, 31, 32, 33, 34, 35, 36, 37, 7,  0, 0, 23, 21};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] = {-33, -33, -13, -33, -33, -33, -33};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] = {-1, 1, 56, 32, 22, 23, 51};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int8 yytable[]
    = {2,  3,  24, 4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 57, 25, 15, 16, 17,
       18, 19, 20, 60, 26, 61, 42, 43, 44, 45, 46, 47, 48, 49, 50, 21, 30, 31, 27,
       28, 29, 0,  33, 34, 35, 36, 37, 0,  38, 39, 40, 41, 52, 0,  53, 54, 55, 0,
       58, 59, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 0,  72, 0,  74, 73, 75};

static const yytype_int8 yycheck[]
    = {0,  1,  34, 3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 27, 32, 16, 17, 18,
       19, 20, 21, 35, 32, 37, 22, 23, 24, 25, 26, 27, 28, 29, 30, 34, 14, 15, 32,
       32, 32, -1, 33, 33, 32, 32, 32, -1, 33, 33, 33, 33, 33, -1, 34, 34, 32, -1,
       33, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, -1, 33, -1, 33, 35, 32};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_int8 yystos[]
    = {0,  37, 0,  1,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 16, 17, 18, 19,
       20, 21, 34, 40, 41, 34, 32, 32, 32, 32, 32, 14, 15, 39, 33, 33, 32, 32, 32,
       33, 33, 33, 33, 22, 23, 24, 25, 26, 27, 28, 29, 30, 42, 33, 34, 34, 32, 38,
       38, 33, 32, 38, 38, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 33, 35, 33, 32};

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_int8 yyr1[]
    = {0,  36, 37, 37, 37, 37, 37, 38, 38, 39, 39, 40, 40, 40, 40, 40, 40, 40, 40,
       40, 40, 40, 40, 40, 40, 40, 40, 40, 41, 42, 42, 42, 42, 42, 42, 42, 42, 42};

/* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_int8 yyr2[] = {0, 2, 0, 2, 3, 3, 3, 2, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 3,
                                   3, 3, 5, 2, 5, 3, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2};

enum { YYENOMEM = -2 };

#define yyerrok (yyerrstatus = 0)
#define yyclearin (yychar = YYEMPTY)

#define YYACCEPT goto yyacceptlab
#define YYABORT goto yyabortlab
#define YYERROR goto yyerrorlab

#define YYRECOVERING() (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                                                     \
	do                                                                                             \
		if (yychar == YYEMPTY) {                                                                   \
			yychar = (Token);                                                                      \
			yylval = (Value);                                                                      \
			YYPOPSTACK(yylen);                                                                     \
			yystate = *yyssp;                                                                      \
			goto yybackup;                                                                         \
		} else {                                                                                   \
			yyerror(YY_("syntax error: cannot back up"));                                          \
			YYERROR;                                                                               \
		}                                                                                          \
	while (0)

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF

/* Enable debugging if requested.  */
#if YYDEBUG

#ifndef YYFPRINTF
#include <cstdio> /* INFRINGES ON USER NAME SPACE */
#define YYFPRINTF fprintf
#endif

#define YYDPRINTF(Args)                                                                            \
	do {                                                                                           \
		if (yydebug) YYFPRINTF Args;                                                               \
	} while (0)

/* This macro is provided for backward compatibility. */
#ifndef YY_LOCATION_PRINT
#define YY_LOCATION_PRINT(File, Loc) ((void)0)
#endif

#define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                                              \
	do {                                                                                           \
		if (yydebug) {                                                                             \
			YYFPRINTF(stderr, "%s ", Title);                                                       \
			yy_symbol_print(stderr, Kind, Value);                                                  \
			YYFPRINTF(stderr, "\n");                                                               \
		}                                                                                          \
	} while (0)

/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void yy_symbol_value_print(FILE* yyo, yysymbol_kind_t yykind, YYSTYPE const* const yyvaluep)
{
	FILE* yyoutput = yyo;
	YYUSE(yyoutput);
	if (!yyvaluep) return;
#ifdef YYPRINT
	if (yykind < YYNTOKENS) YYPRINT(yyo, yytoknum[yykind], *yyvaluep);
#endif
	YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
	YYUSE(yykind);
	YY_IGNORE_MAYBE_UNINITIALIZED_END
}

/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void yy_symbol_print(FILE* yyo, yysymbol_kind_t yykind, YYSTYPE const* const yyvaluep)
{
	YYFPRINTF(yyo, "%s %s (", yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name(yykind));

	yy_symbol_value_print(yyo, yykind, yyvaluep);
	YYFPRINTF(yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void yy_stack_print(yy_state_t* yybottom, yy_state_t* yytop)
{
	YYFPRINTF(stderr, "Stack now");
	for (; yybottom <= yytop; yybottom++) {
		int yybot = *yybottom;
		YYFPRINTF(stderr, " %d", yybot);
	}
	YYFPRINTF(stderr, "\n");
}

#define YY_STACK_PRINT(Bottom, Top)                                                                \
	do {                                                                                           \
		if (yydebug) yy_stack_print((Bottom), (Top));                                              \
	} while (0)

/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void yy_reduce_print(yy_state_t* yyssp, YYSTYPE* yyvsp, int yyrule)
{
	int yylno = yyrline[yyrule];
	int yynrhs = yyr2[yyrule];
	int yyi;
	YYFPRINTF(stderr, "Reducing stack by rule %d (line %d):\n", yyrule - 1, yylno);
	/* The symbols being reduced.  */
	for (yyi = 0; yyi < yynrhs; yyi++) {
		YYFPRINTF(stderr, "   $%d = ", yyi + 1);
		yy_symbol_print(stderr,
		                YY_ACCESSING_SYMBOL(+yyssp[yyi + 1 - yynrhs]),
		                &yyvsp[(yyi + 1) - (yynrhs)]);
		YYFPRINTF(stderr, "\n");
	}
}

#define YY_REDUCE_PRINT(Rule)                                                                      \
	do {                                                                                           \
		if (yydebug) yy_reduce_print(yyssp, yyvsp, Rule);                                          \
	} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
#define YYDPRINTF(Args) ((void)0)
#define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
#define YY_STACK_PRINT(Bottom, Top)
#define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */

/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
#define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
#define YYMAXDEPTH 10000
#endif

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void yydestruct(const char* yymsg, yysymbol_kind_t yykind, YYSTYPE* yyvaluep)
{
	YYUSE(yyvaluep);
	if (!yymsg) yymsg = "Deleting";
	YY_SYMBOL_PRINT(yymsg, yykind, yyvaluep, yylocationp);

	YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
	YYUSE(yykind);
	YY_IGNORE_MAYBE_UNINITIALIZED_END
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

int yyparse(void)
{
	yy_state_fast_t yystate = 0;
	/* Number of tokens to shift before error messages enabled.  */
	int yyerrstatus = 0;

	/* Refer to the stacks through separate pointers, to allow yyoverflow
	   to reallocate them elsewhere.  */

	/* Their size.  */
	YYPTRDIFF_T yystacksize = YYINITDEPTH;

	/* The state stack: array, bottom, top.  */
	yy_state_t yyssa[YYINITDEPTH];
	yy_state_t* yyss = yyssa;
	yy_state_t* yyssp = yyss;

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

#define YYPOPSTACK(N) (yyvsp -= (N), yyssp -= (N))

	/* The number of symbols on the RHS of the reduced rule.
	   Keep to zero when no symbol should be popped.  */
	int yylen = 0;

	YYDPRINTF((stderr, "Starting parse\n"));

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
	YYDPRINTF((stderr, "Entering state %d\n", yystate));
	YY_ASSERT(0 <= yystate && yystate < YYNSTATES);
	YY_IGNORE_USELESS_CAST_BEGIN
	*yyssp = YY_CAST(yy_state_t, yystate);
	YY_IGNORE_USELESS_CAST_END
	YY_STACK_PRINT(yyss, yyssp);

	if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
		goto yyexhaustedlab;
#else
	{
		/* Get the current used size of the three stacks, in elements.  */
		YYPTRDIFF_T yysize = yyssp - yyss + 1;

#if defined yyoverflow
		{
			/* Give user a chance to reallocate the stack.  Use copies of
			   these so that the &'s don't force the real ones into
			   memory.  */
			yy_state_t* yyss1 = yyss;
			YYSTYPE* yyvs1 = yyvs;

			/* Each stack pointer address is followed by the size of the
			   data in use in that stack, in bytes.  This used to be a
			   conditional around just the two extra args, but that might
			   be undefined if yyoverflow is a macro.  */
			yyoverflow(YY_("memory exhausted"),
			           &yyss1,
			           yysize * YYSIZEOF(*yyssp),
			           &yyvs1,
			           yysize * YYSIZEOF(*yyvsp),
			           &yystacksize);
			yyss = yyss1;
			yyvs = yyvs1;
		}
#else /* defined YYSTACK_RELOCATE */
		/* Extend the stack our own way.  */
		if (YYMAXDEPTH <= yystacksize) goto yyexhaustedlab;
		yystacksize *= 2;
		if (YYMAXDEPTH < yystacksize) yystacksize = YYMAXDEPTH;

		{
			yy_state_t* yyss1 = yyss;
			union yyalloc* yyptr
			    = YY_CAST(union yyalloc*,
			              YYSTACK_ALLOC(YY_CAST(YYSIZE_T, YYSTACK_BYTES(yystacksize))));
			if (!yyptr) goto yyexhaustedlab;
			YYSTACK_RELOCATE(yyss_alloc, yyss);
			YYSTACK_RELOCATE(yyvs_alloc, yyvs);
#undef YYSTACK_RELOCATE
			if (yyss1 != yyssa) YYSTACK_FREE(yyss1);
		}
#endif

		yyssp = yyss + yysize - 1;
		yyvsp = yyvs + yysize - 1;

		YY_IGNORE_USELESS_CAST_BEGIN
		YYDPRINTF((stderr, "Stack size increased to %ld\n", YY_CAST(long, yystacksize)));
		YY_IGNORE_USELESS_CAST_END

		if (yyss + yystacksize - 1 <= yyssp) YYABORT;
	}
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */

	if (yystate == YYFINAL) YYACCEPT;

	goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:
	/* Do appropriate processing given the current state.  Read a
	   lookahead token if we need one and don't already have one.  */

	/* First try to decide what to do without reference to lookahead token.  */
	yyn = yypact[yystate];
	if (yypact_value_is_default(yyn)) goto yydefault;

	/* Not known => get a lookahead token if don't already have one.  */

	/* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
	if (yychar == YYEMPTY) {
		YYDPRINTF((stderr, "Reading a token\n"));
		yychar = yylex();
	}

	if (yychar <= YYEOF) {
		yychar = YYEOF;
		yytoken = YYSYMBOL_YYEOF;
		YYDPRINTF((stderr, "Now at end of input.\n"));
	} else if (yychar == YYerror) {
		/* The scanner already issued an error message, process directly
		   to error recovery.  But do not keep the error token as
		   lookahead, it is too special and may lead us to an endless
		   loop in error recovery. */
		yychar = YYUNDEF;
		yytoken = YYSYMBOL_YYerror;
		goto yyerrlab1;
	} else {
		yytoken = YYTRANSLATE(yychar);
		YY_SYMBOL_PRINT("Next token is", yytoken, &yylval, &yylloc);
	}

	/* If the proper action on seeing token YYTOKEN is to reduce or to
	   detect an error, take that action.  */
	yyn += yytoken;
	if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken) goto yydefault;
	yyn = yytable[yyn];
	if (yyn <= 0) {
		if (yytable_value_is_error(yyn)) goto yyerrlab;
		yyn = -yyn;
		goto yyreduce;
	}

	/* Count tokens shifted since error; after three, turn off error
	   status.  */
	if (yyerrstatus) yyerrstatus--;

	/* Shift the lookahead token.  */
	YY_SYMBOL_PRINT("Shifting", yytoken, &yylval, &yylloc);
	yystate = yyn;
	YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
	*++yyvsp = yylval;
	YY_IGNORE_MAYBE_UNINITIALIZED_END

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

	YY_REDUCE_PRINT(yyn);
	switch (yyn) {
	case 6: /* grammar: grammar error '\n'  */
#line 93 "parse.y"
	{
		file->errors++;
	}
#line 1282 "y.tab.c"
	break;

	case 7: /* string: string STRING  */
#line 96 "parse.y"
	{
		if (asprintf(&(yyval.v.string), "%s %s", (yyvsp[-1].v.string), (yyvsp[0].v.string)) == -1) {
			free((yyvsp[-1].v.string));
			free((yyvsp[0].v.string));
			yyerror("string: asprintf");
			YYERROR;
		}
		free((yyvsp[-1].v.string));
		free((yyvsp[0].v.string));
	}
#line 1297 "y.tab.c"
	break;

	case 9: /* yesno: YES  */
#line 109 "parse.y"
	{
		(yyval.v.number) = 1;
	}
#line 1303 "y.tab.c"
	break;

	case 10: /* yesno: NO  */
#line 110 "parse.y"
	{
		(yyval.v.number) = 0;
	}
#line 1309 "y.tab.c"
	break;

	case 11: /* main: FONTNAME STRING  */
#line 113 "parse.y"
	{
		free(conf->font);
		conf->font = (yyvsp[0].v.string);
	}
#line 1318 "y.tab.c"
	break;

	case 12: /* main: STICKY yesno  */
#line 117 "parse.y"
	{
		conf->stickygroups = (yyvsp[0].v.number);
	}
#line 1326 "y.tab.c"
	break;

	case 13: /* main: BORDERWIDTH NUMBER  */
#line 120 "parse.y"
	{
		if ((yyvsp[0].v.number) < 0 || (yyvsp[0].v.number) > INT_MAX) {
			yyerror("invalid borderwidth");
			YYERROR;
		}
		conf->bwidth = (yyvsp[0].v.number);
	}
#line 1338 "y.tab.c"
	break;

	case 14: /* main: HTILE NUMBER  */
#line 127 "parse.y"
	{
		if ((yyvsp[0].v.number) < 0 || (yyvsp[0].v.number) > 99) {
			yyerror("invalid htile percent");
			YYERROR;
		}
		conf->htile = (yyvsp[0].v.number);
	}
#line 1350 "y.tab.c"
	break;

	case 15: /* main: VTILE NUMBER  */
#line 134 "parse.y"
	{
		if ((yyvsp[0].v.number) < 0 || (yyvsp[0].v.number) > 99) {
			yyerror("invalid vtile percent");
			YYERROR;
		}
		conf->vtile = (yyvsp[0].v.number);
	}
#line 1362 "y.tab.c"
	break;

	case 16: /* main: MOVEAMOUNT NUMBER  */
#line 141 "parse.y"
	{
		if ((yyvsp[0].v.number) < 0 || (yyvsp[0].v.number) > INT_MAX) {
			yyerror("invalid movemount");
			YYERROR;
		}
		conf->mamount = (yyvsp[0].v.number);
	}
#line 1374 "y.tab.c"
	break;

	case 17: /* main: SNAPDIST NUMBER  */
#line 148 "parse.y"
	{
		if ((yyvsp[0].v.number) < 0 || (yyvsp[0].v.number) > INT_MAX) {
			yyerror("invalid snapdist");
			YYERROR;
		}
		conf->snapdist = (yyvsp[0].v.number);
	}
#line 1386 "y.tab.c"
	break;

	case 18: /* main: COMMAND STRING string  */
#line 155 "parse.y"
	{
		if (strlen((yyvsp[0].v.string)) >= PATH_MAX) {
			yyerror("%s command path too long", (yyvsp[-1].v.string));
			free((yyvsp[-1].v.string));
			free((yyvsp[0].v.string));
			YYERROR;
		}
		conf_cmd_add(conf, (yyvsp[-1].v.string), (yyvsp[0].v.string));
		free((yyvsp[-1].v.string));
		free((yyvsp[0].v.string));
	}
#line 1402 "y.tab.c"
	break;

	case 19: /* main: WM STRING string  */
#line 166 "parse.y"
	{
		if (strlen((yyvsp[0].v.string)) >= PATH_MAX) {
			yyerror("%s wm path too long", (yyvsp[-1].v.string));
			free((yyvsp[-1].v.string));
			free((yyvsp[0].v.string));
			YYERROR;
		}
		conf_wm_add(conf, (yyvsp[-1].v.string), (yyvsp[0].v.string));
		free((yyvsp[-1].v.string));
		free((yyvsp[0].v.string));
	}
#line 1418 "y.tab.c"
	break;

	case 20: /* main: AUTOGROUP NUMBER STRING  */
#line 177 "parse.y"
	{
		if ((yyvsp[-1].v.number) < 0 || (yyvsp[-1].v.number) > 9) {
			yyerror("invalid autogroup");
			free((yyvsp[0].v.string));
			YYERROR;
		}
		conf_autogroup(conf, (yyvsp[-1].v.number), nullptr, (yyvsp[0].v.string));
		free((yyvsp[0].v.string));
	}
#line 1432 "y.tab.c"
	break;

	case 21: /* main: AUTOGROUP NUMBER STRING ',' STRING  */
#line 186 "parse.y"
	{
		if ((yyvsp[-3].v.number) < 0 || (yyvsp[-3].v.number) > 9) {
			yyerror("invalid autogroup");
			free((yyvsp[-2].v.string));
			free((yyvsp[0].v.string));
			YYERROR;
		}
		conf_autogroup(conf, (yyvsp[-3].v.number), (yyvsp[-2].v.string), (yyvsp[0].v.string));
		free((yyvsp[-2].v.string));
		free((yyvsp[0].v.string));
	}
#line 1448 "y.tab.c"
	break;

	case 22: /* main: IGNORE STRING  */
#line 197 "parse.y"
	{
		conf_ignore(conf, (yyvsp[0].v.string));
		free((yyvsp[0].v.string));
	}
#line 1457 "y.tab.c"
	break;

	case 23: /* main: GAP NUMBER NUMBER NUMBER NUMBER  */
#line 201 "parse.y"
	{
		if ((yyvsp[-3].v.number) < 0 || (yyvsp[-3].v.number) > INT_MAX || (yyvsp[-2].v.number) < 0
		    || (yyvsp[-2].v.number) > INT_MAX || (yyvsp[-1].v.number) < 0
		    || (yyvsp[-1].v.number) > INT_MAX || (yyvsp[0].v.number) < 0
		    || (yyvsp[0].v.number) > INT_MAX) {
			yyerror("invalid gap");
			YYERROR;
		}
		conf->gap.top = (yyvsp[-3].v.number);
		conf->gap.bottom = (yyvsp[-2].v.number);
		conf->gap.left = (yyvsp[-1].v.number);
		conf->gap.right = (yyvsp[0].v.number);
	}
#line 1475 "y.tab.c"
	break;

	case 24: /* main: BINDKEY STRING string  */
#line 214 "parse.y"
	{
		if (!conf_bind_key(conf, (yyvsp[-1].v.string), (yyvsp[0].v.string))) {
			yyerror("invalid bind-key: %s %s", (yyvsp[-1].v.string), (yyvsp[0].v.string));
			free((yyvsp[-1].v.string));
			free((yyvsp[0].v.string));
			YYERROR;
		}
		free((yyvsp[-1].v.string));
		free((yyvsp[0].v.string));
	}
#line 1490 "y.tab.c"
	break;

	case 25: /* main: UNBINDKEY STRING  */
#line 224 "parse.y"
	{
		if (!conf_bind_key(conf, (yyvsp[0].v.string), nullptr)) {
			yyerror("invalid unbind-key: %s", (yyvsp[0].v.string));
			free((yyvsp[0].v.string));
			YYERROR;
		}
		free((yyvsp[0].v.string));
	}
#line 1503 "y.tab.c"
	break;

	case 26: /* main: BINDMOUSE STRING string  */
#line 232 "parse.y"
	{
		if (!conf_bind_mouse(conf, (yyvsp[-1].v.string), (yyvsp[0].v.string))) {
			yyerror("invalid bind-mouse: %s %s", (yyvsp[-1].v.string), (yyvsp[0].v.string));
			free((yyvsp[-1].v.string));
			free((yyvsp[0].v.string));
			YYERROR;
		}
		free((yyvsp[-1].v.string));
		free((yyvsp[0].v.string));
	}
#line 1518 "y.tab.c"
	break;

	case 27: /* main: UNBINDMOUSE STRING  */
#line 242 "parse.y"
	{
		if (!conf_bind_mouse(conf, (yyvsp[0].v.string), nullptr)) {
			yyerror("invalid unbind-mouse: %s", (yyvsp[0].v.string));
			free((yyvsp[0].v.string));
			YYERROR;
		}
		free((yyvsp[0].v.string));
	}
#line 1531 "y.tab.c"
	break;

	case 29: /* colors: ACTIVEBORDER STRING  */
#line 255 "parse.y"
	{
		free(conf->color[CWM_COLOR_BORDER_ACTIVE]);
		conf->color[CWM_COLOR_BORDER_ACTIVE] = (yyvsp[0].v.string);
	}
#line 1540 "y.tab.c"
	break;

	case 30: /* colors: INACTIVEBORDER STRING  */
#line 259 "parse.y"
	{
		free(conf->color[CWM_COLOR_BORDER_INACTIVE]);
		conf->color[CWM_COLOR_BORDER_INACTIVE] = (yyvsp[0].v.string);
	}
#line 1549 "y.tab.c"
	break;

	case 31: /* colors: URGENCYBORDER STRING  */
#line 263 "parse.y"
	{
		free(conf->color[CWM_COLOR_BORDER_URGENCY]);
		conf->color[CWM_COLOR_BORDER_URGENCY] = (yyvsp[0].v.string);
	}
#line 1558 "y.tab.c"
	break;

	case 32: /* colors: GROUPBORDER STRING  */
#line 267 "parse.y"
	{
		free(conf->color[CWM_COLOR_BORDER_GROUP]);
		conf->color[CWM_COLOR_BORDER_GROUP] = (yyvsp[0].v.string);
	}
#line 1567 "y.tab.c"
	break;

	case 33: /* colors: UNGROUPBORDER STRING  */
#line 271 "parse.y"
	{
		free(conf->color[CWM_COLOR_BORDER_UNGROUP]);
		conf->color[CWM_COLOR_BORDER_UNGROUP] = (yyvsp[0].v.string);
	}
#line 1576 "y.tab.c"
	break;

	case 34: /* colors: MENUBG STRING  */
#line 275 "parse.y"
	{
		free(conf->color[CWM_COLOR_MENU_BG]);
		conf->color[CWM_COLOR_MENU_BG] = (yyvsp[0].v.string);
	}
#line 1585 "y.tab.c"
	break;

	case 35: /* colors: MENUFG STRING  */
#line 279 "parse.y"
	{
		free(conf->color[CWM_COLOR_MENU_FG]);
		conf->color[CWM_COLOR_MENU_FG] = (yyvsp[0].v.string);
	}
#line 1594 "y.tab.c"
	break;

	case 36: /* colors: FONTCOLOR STRING  */
#line 283 "parse.y"
	{
		free(conf->color[CWM_COLOR_MENU_FONT]);
		conf->color[CWM_COLOR_MENU_FONT] = (yyvsp[0].v.string);
	}
#line 1603 "y.tab.c"
	break;

	case 37: /* colors: FONTSELCOLOR STRING  */
#line 287 "parse.y"
	{
		free(conf->color[CWM_COLOR_MENU_FONT_SEL]);
		conf->color[CWM_COLOR_MENU_FONT_SEL] = (yyvsp[0].v.string);
	}
#line 1612 "y.tab.c"
	break;

#line 1616 "y.tab.c"

	default: break;
	}
	/* User semantic actions sometimes alter yychar, and that requires
	   that yytoken be updated with the new translation.  We take the
	   approach of translating immediately before every use of yytoken.
	   One alternative is translating here after every semantic action,
	   but that translation would be missed if the semantic action invokes
	   YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
	   if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
	   incorrect destructor might then be invoked immediately.  In the
	   case of YYERROR or YYBACKUP, subsequent parser actions might lead
	   to an incorrect destructor call or verbose syntax error message
	   before the lookahead is translated.  */
	YY_SYMBOL_PRINT("-> $$ =", YY_CAST(yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

	YYPOPSTACK(yylen);
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
	yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE(yychar);
	/* If not already recovering from an error, report this error.  */
	if (!yyerrstatus) {
		++yynerrs;
		yyerror(YY_("syntax error"));
	}

	if (yyerrstatus == 3) {
		/* If just tried and failed to reuse lookahead token after an
		   error, discard it.  */

		if (yychar <= YYEOF) {
			/* Return failure if at end of input.  */
			if (yychar == YYEOF) YYABORT;
		} else {
			yydestruct("Error: discarding", yytoken, &yylval);
			yychar = YYEMPTY;
		}
	}

	/* Else will try to reuse lookahead token after shifting the error
	   token.  */
	goto yyerrlab1;

/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
	/* Pacify compilers when the user code never invokes YYERROR and the
	   label yyerrorlab therefore never appears in user code.  */
	if (0) YYERROR;

	/* Do not reclaim the symbols of the rule whose action triggered
	   this YYERROR.  */
	YYPOPSTACK(yylen);
	yylen = 0;
	YY_STACK_PRINT(yyss, yyssp);
	yystate = *yyssp;
	goto yyerrlab1;

/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
	yyerrstatus = 3; /* Each real token shifted decrements this.  */

	/* Pop stack until we find a state that shifts the error token.  */
	for (;;) {
		yyn = yypact[yystate];
		if (!yypact_value_is_default(yyn)) {
			yyn += YYSYMBOL_YYerror;
			if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror) {
				yyn = yytable[yyn];
				if (0 < yyn) break;
			}
		}

		/* Pop the current state because it cannot handle the error token.  */
		if (yyssp == yyss) YYABORT;

		yydestruct("Error: popping", YY_ACCESSING_SYMBOL(yystate), yyvsp);
		YYPOPSTACK(1);
		yystate = *yyssp;
		YY_STACK_PRINT(yyss, yyssp);
	}

	YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
	*++yyvsp = yylval;
	YY_IGNORE_MAYBE_UNINITIALIZED_END

	/* Shift the error token.  */
	YY_SYMBOL_PRINT("Shifting", YY_ACCESSING_SYMBOL(yyn), yyvsp, yylsp);

	yystate = yyn;
	goto yynewstate;

/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
	yyresult = 0;
	goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
	yyresult = 1;
	goto yyreturn;

#if !defined yyoverflow
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
	yyerror(YY_("memory exhausted"));
	yyresult = 2;
	goto yyreturn;
#endif

/*-------------------------------------------------------.
| yyreturn -- parsing is finished, clean up and return.  |
`-------------------------------------------------------*/
yyreturn:
	if (yychar != YYEMPTY) {
		/* Make sure we have latest lookahead translation.  See comments at
		   user semantic actions for why this is necessary.  */
		yytoken = YYTRANSLATE(yychar);
		yydestruct("Cleanup: discarding lookahead", yytoken, &yylval);
	}
	/* Do not reclaim the symbols of the rule whose action triggered
	   this YYABORT or YYACCEPT.  */
	YYPOPSTACK(yylen);
	YY_STACK_PRINT(yyss, yyssp);
	while (yyssp != yyss) {
		yydestruct("Cleanup: popping", YY_ACCESSING_SYMBOL(+*yyssp), yyvsp);
		YYPOPSTACK(1);
	}
#ifndef yyoverflow
	if (yyss != yyssa) YYSTACK_FREE(yyss);
#endif

	return yyresult;
}

#line 292 "parse.y"

struct keywords {
	const char* k_name;
	int k_val;
};

int yyerror(const char* fmt, ...)
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
	return (strcmp((const char*)k, ((const struct keywords*)e)->k_name));
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

#define MAXPUSHBACK 128

char* parsebuf;
int parseindex;
char pushback_buffer[MAXPUSHBACK];
int pushback_index = 0;

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

int findeol(void)
{
	int c;

	parsebuf = nullptr;

	/* skip to either EOF or the first real EOL */
	while (1) {
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

int yylex(void)
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
		while (1) {
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

#define allowed_to_end_number(x)                                                                   \
	(isspace(x) || x == ')' || x == ',' || x == '/' || x == '}' || x == '=')

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
		if (c == EOF || allowed_to_end_number(c)) {
			const char* errstr = nullptr;

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

/* Similar to other parse.y copies, but also allows '/' in strings */
#define allowed_in_string(x)                                                                       \
	(isalnum(x)                                                                                    \
	 || (ispunct(x) && x != '(' && x != ')' && x != '{' && x != '}' && x != '<' && x != '>'        \
	     && x != '!' && x != '=' && x != '#' && x != ','))

	if (isalnum(c) || c == ':' || c == '_' || c == '*' || c == '/') {
		do {
			*p++ = c;
			if ((unsigned)(p - buf) >= sizeof(buf)) {
				yyerror("string too long");
				return (findeol());
			}
		} while ((c = lgetc(0)) != EOF && (allowed_in_string(c)));
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

struct file* pushfile(const char* name, FILE* stream)
{
	struct file* nfile;

	nfile = (struct file*)xcalloc(1, sizeof(struct file));
	nfile->name = xstrdup(name);
	nfile->stream = stream;
	nfile->lineno = 1;
	TAILQ_INSERT_TAIL(&files, nfile, entry);
	return (nfile);
}

int popfile(void)
{
	struct file* prev;

	if ((prev = TAILQ_PREV(file, files, entry)) != nullptr) prev->errors += file->errors;

	TAILQ_REMOVE(&files, file, entry);
	fclose(file->stream);
	free(file->name);
	free(file);
	file = prev;
	return (file ? 0 : EOF);
}

int parse_config(const char* filename, struct Conf* xconf)
{
	FILE* stream;
	int errors = 0;

	conf = xconf;

	stream = fopen(filename, "r");
	if (stream == nullptr) {
		if (errno == ENOENT) return (0);
		warn("%s", filename);
		return (-1);
	}
	file = pushfile(filename, stream);
	topfile = file;

	yyparse();
	errors = file->errors;
	popfile();

	return (errors ? -1 : 0);
}
