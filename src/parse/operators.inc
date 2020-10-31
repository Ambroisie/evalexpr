/*
 * Corresponds to the following grammar, with E as start:
 *
 * E : T [ ('+'|'-') T ]*
 * T : F [ ('*'|'/') F ]*
 * F : [ ('-'|'+') ]* P
 * P : G ('^') F
 * G : ( '(' E ')' | CONSTANT ) [ '!' ]
 *
 * G, the operand, is parsed by a specific function to start the process.
 */

#ifndef OP
# define OP(Kind, Prio, Assoc, Fix, /* Operator string */ ...)
#endif
#ifndef BINOP
# define BINOP(Kind, Prio, Assoc, ...) \
    OP(Kind, Prio, Assoc, OP_INFIX, __VA_ARGS__)
#endif
#ifndef PREOP
# define PREOP(Kind, Prio, Assoc, ...) \
    OP(Kind, Prio, Assoc, OP_PREFIX, __VA_ARGS__)
#endif
#ifndef POSTOP
# define POSTOP(Kind, Prio, Assoc, ...) \
    OP(Kind, Prio, Assoc, OP_POSTFIX, __VA_ARGS__)
#endif

BINOP (BINOP_PLUS,    1, ASSOC_LEFT,  '+', 0)
BINOP (BINOP_MINUS,   1, ASSOC_LEFT,  '-', 0)
BINOP (BINOP_TIMES,   2, ASSOC_LEFT,  '*', 0)
BINOP (BINOP_DIVIDES, 2, ASSOC_LEFT,  '/', 0)
BINOP (BINOP_POW,     4, ASSOC_RIGHT, '^', 0)
PREOP (UNOP_NEGATE,   3, ASSOC_RIGHT, '-', 0)
PREOP (UNOP_IDENTITY, 3, ASSOC_RIGHT, '+', 0)
POSTOP(UNOP_FACT,     5, ASSOC_NONE,  '!', 0)

#undef BINOP
#undef PREOP
#undef POSTOP
#undef OP
