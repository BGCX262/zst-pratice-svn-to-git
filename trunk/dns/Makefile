dns:dns.c
	gcc $(CFLAGS) dns.c ../public/public.c -o dns -lsqlite -lfwlog -I../public -Wall
	strip dns

install:
	cp -f dns $(CLI_ROOT)/usr/local/bin/dns

clean:
	rm -f dns

