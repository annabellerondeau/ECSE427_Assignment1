CC=gcc
CFLAGS=
FMT=indent

mysh: shell.c interpreter.c shellmemory.c
	$(CC) $(CFLAGS) -c shell.c interpreter.c shellmemory.c
	$(CC) $(CFLAGS) -o mysh shell.o interpreter.o shellmemory.o

style: shell.c shell.h interpreter.c interpreter.h shellmemory.c shellmemory.h
	$(FMT) $?

clean: 
	$(RM) mysh; $(RM) *.o; $(RM) *~

test: mysh
	@for t in badcommand echo ls mkdir oneline oneline2 prompt run set source; do \
		./mysh < ../test-cases/$$t.txt > result.tmp; \
		diff -w -q result.tmp ../test-cases/$${t}_result.txt > /dev/null; \
		if [ $$? -eq 0 ]; then \
			echo "$$t: PASS"; \
		else \
			echo "$$t: FAIL"; \
		fi; \
	done
	@rm -f result.tmp
