
all:	sig-bad create

clean:	
	rm -f *-bad *-ok create

sig-bad: sig-bad.c
	gcc -g -o sig-bad sig-bad.c -I . -lresolv

sig-ok: sig-ok.c
	gcc -g -o sig-ok sig-ok.c -I . -lresolv

create:	create_msg_file.c
	gcc -g -o create create_msg_file.c -lresolv


