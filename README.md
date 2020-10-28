# Evalexpr

This was a small experiment in writing a precedence climbing parser, using C.
I also have my reference parsing implementation, using recursive descent, to
compare the parsing results.

This is mostly based on the explanation found at this address:
https://www.engr.mun.ca/~theo/Misc/exp_parsing.htm (use the Wayback Machine to
read it).

## How to build

Simply launch the following command

```sh
42sh$ make
```

If you want to build an `evalexpr` command using recursive descent instead, use:

```sh
42sh$ make USE_CLIMBING=0
```

Don't forget to use `make clean` when alternating between both.


## How to use

Simply launch the binary, and write an expression on its standard input. The
binary can parse exactly one expression per line, and reports parsing errors if
they happen.

Example use:

```none
42sh$ ./evalexpr
1 + 2 * 3 - 3!
1
```
