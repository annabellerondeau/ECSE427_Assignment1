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

testA1: mysh
	@for t in badcommand echo ls mkdir oneline oneline2 prompt run set source; do \
		cd ../test-cases-A1 && ../src/mysh < $$t.txt > result.tmp; \
		diff -w -q ../test-cases-A1/result.tmp ../test-cases-A1/$${t}_result.txt > /dev/null; \
		if [ $$? -eq 0 ]; then \
			echo "$$t: PASS"; \
		else \
			echo "$$t: FAIL"; \
		fi; \
	done
	@rm -f result.tmp

testA2: mysh
	@for t in T_AGING T_AGING2 T_AGING3 T_AGING4 T_background T_FCFS T_FCFS2 T_FCFS3 T_FCFS4 T_RR T_RR2 T_RR3 T_RR4 T_RR30 T_RR30_2 T_SJF T_SJF2 T_SJF3 T_SJF4 T_source; do \
		cd ../test-cases-A2 && ../src/mysh < $$t.txt > result.tmp; \
		diff -w -q ../test-cases-A2/result.tmp ../test-cases-A2/$${t}_result.txt > /dev/null; \
		if [ $$? -eq 0 ]; then \
			echo "$$t: PASS"; \
		elif [ -f $${t}_result2.txt ] && diff -w -B -q result.tmp $${t}_result2.txt > /dev/null; then \
			echo "$$t: PASS (Matches Result 2)"; \
		else \
			echo "$$t: FAIL"; \
			diff -y result.tmp $${t}_result.txt; \
		fi; \
	done
