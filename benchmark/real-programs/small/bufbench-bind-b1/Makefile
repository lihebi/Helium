
all:	nxt-bad

clean:	
	rm -f *-bad *-ok

nxt-bad: nxt-bad.c 
	gcc -g -o nxt-bad nxt-bad.c -I .  -lresolv

nxt-ok: nxt-ok.c 
	gcc -g -o nxt-ok nxt-ok.c -I .  -lresolv
