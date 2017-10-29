# This makefile only works on Mac OS and Linux
# Windows does not require -ledit
c-lisp: parsing.c mpc.c
	gcc -std=c99 -Wall -o c-lisp parsing.c mpc.c -ledit -lm -I.
