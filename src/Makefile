CC=gcc
CFLAGS=
FMT=indent

mysh: shell.c interpreter.c shellmemory.c pcb.o scheduler.o
	$(CC) $(CFLAGS) -c shell.c interpreter.c shellmemory.c pcb.c scheduler.c
	$(CC) $(CFLAGS) -o mysh shell.o interpreter.o shellmemory.o pcb.o scheduler.o

style: shell.c shell.h interpreter.c interpreter.h shellmemory.c shellmemory.h
	$(FMT) $?

clean: 
	$(RM) mysh; $(RM) *.o; $(RM) *~

test: mysh
	@for t in badcommand echo ls mkdir oneline oneline2 prompt run set source; do \
		cd ../test-cases && ../src/mysh < $$t.txt > result.tmp; \
		diff -w -q ../test-cases/result.tmp ../test-cases/$${t}_result.txt > /dev/null; \
		if [ $$? -eq 0 ]; then \
			echo "$$t: PASS"; \
		else \
			echo "$$t: FAIL"; \
		fi; \
	done
	@rm -f result.tmp
