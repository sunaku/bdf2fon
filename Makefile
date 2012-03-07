all: bdf2fnt fnt2fon

bdf2fnt: bdf2fnt.c
	cc -o $@ -Wall -Werror $^

fnt2fon: fnt2fon.c
	cc -o $@ -Wall -Werror $^

clean:
	rm -f bdf2fnt fnt2fon
