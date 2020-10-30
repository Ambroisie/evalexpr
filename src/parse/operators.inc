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
# define OP(Op, Kind, Prio, Assoc, Fix)
#endif
#ifndef BINOP
# define BINOP(Op, Kind, Prio, Assoc) \
    OP(Op, Kind, Prio, Assoc, OP_INFIX)
#endif
#ifndef PREOP
# define PREOP(Op, Kind, Prio, Assoc) \
    OP(Op, Kind, Prio, Assoc, OP_PREFIX)
#endif
#ifndef POSTOP
# define POSTOP(Op, Kind, Prio, Assoc) \
    OP(Op, Kind, Prio, Assoc, OP_POSTFIX)
#endif

BINOP(+,  BINOP_PLUS,    1, ASSOC_LEFT)
BINOP(-,  BINOP_MINUS,   1, ASSOC_LEFT)
BINOP(*,  BINOP_TIMES,   2, ASSOC_LEFT)
BINOP(/,  BINOP_DIVIDES, 2, ASSOC_LEFT)
BINOP(^,  BINOP_POW,     4, ASSOC_RIGHT)
PREOP(-,  UNOP_NEGATE,   3, ASSOC_RIGHT)
PREOP(+,  UNOP_IDENTITY, 3, ASSOC_RIGHT)
POSTOP(!, UNOP_FACT,     5, ASSOC_NONE)

#undef BINOP
#undef PREOP
#undef POSTOP
#undef OP
