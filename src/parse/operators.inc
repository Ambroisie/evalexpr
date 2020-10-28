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

#ifndef BINOP
# define BINOP(_, __, ___, ____, _____)
#endif
#ifndef PREOP
# define PREOP(_, __, ___, ____, _____)
#endif
#ifndef POSTOP
# define POSTOP(_, __, ___, ____, _____)
#endif

BINOP(+,  BINOP_PLUS,    1, ASSOC_LEFT,  OP_INFIX)
BINOP(-,  BINOP_MINUS,   1, ASSOC_LEFT,  OP_INFIX)
BINOP(*,  BINOP_TIMES,   2, ASSOC_LEFT,  OP_INFIX)
BINOP(/,  BINOP_DIVIDES, 2, ASSOC_LEFT,  OP_INFIX)
BINOP(^,  BINOP_POW,     4, ASSOC_RIGHT, OP_INFIX)
PREOP(-,  UNOP_NEGATE,   3, ASSOC_RIGHT, OP_PREFIX)
PREOP(+,  UNOP_IDENTITY, 3, ASSOC_RIGHT, OP_PREFIX)
POSTOP(!, UNOP_FACT,     5, ASSOC_NONE,  OP_POSTFIX)

#undef BINOP
#undef PREOP
#undef POSTOP
