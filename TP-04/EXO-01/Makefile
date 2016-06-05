.PHONY : all clean check
.ONESHELL:

CHECK_PATCH=../../../../biblio/eudyptula/torvalds/scripts/checkpatch.pl

all : cron_func

libfunc.o : nothing.c
	gcc -Wall -fPIC -c $< -o libfunc.o

libfunc.so : libfunc.o
	gcc -shared $< -o libfunc.so

%.o : %.c
	gcc -Wall -c $<

cron_func : cron_func.o
	gcc -Wall $^ -o $@ 

check :
	for f in *.c *.h ; do
		$(CHECK_PATCH) -f $$f
	done

clean :
	rm -rf *.o cron_func
