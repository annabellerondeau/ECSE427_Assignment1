CC=gcc
CFLAGS=
FMT=indent

mysh: shell.c interpreter.c shellmemory.c pcb.o scheduler.o
	$(CC) $(CFLAGS) -c shell.c interpreter.c shellmemory.c pcb.c scheduler.c
	$(CC) $(CFLAGS) -pthread -o mysh shell.o interpreter.o shellmemory.o pcb.o scheduler.o

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
	@for i in $$(seq 1 100); do \
     	echo "=== RUN $$i ==="; \
            for t in $$(cd ../test-cases-A2 && ls T_MT*.txt | grep -v '_result' | sed 's/.txt//'); do \
                (cd ../test-cases-A2 && ../src/mysh < $$t.txt > result.tmp; \
                if echo $$t | grep -q "^T_MT"; then \
                    [ $$(grep -o "X" result.tmp | wc -l) -eq $$(grep -o "X" $${t}_result.txt | wc -l) ] && \
                    [ $$(grep -o "YY" result.tmp | wc -l) -eq $$(grep -o "YY" $${t}_result.txt | wc -l) ] && \
                    [ $$(grep -o "ZZZ" result.tmp | wc -l) -eq $$(grep -o "ZZZ" $${t}_result.txt | wc -l) ]; \
                    if [ $$? -eq 0 ]; then echo "$$t: PASS"; \
                    else \
                        echo "$$t: FAIL";\
                        diff -y result.tmp $${t}_result.txt; \
                    fi; \
                else \
                    diff -w -q ../test-cases-A2/result.tmp ../test-cases-A2/$${t}_result.txt > /dev/null; \
                    if [ $$? -eq 0 ]; then \
                        echo "$$t: PASS"; \
                    elif [ -f $${t}_result2.txt ] && diff -w -B -q result.tmp $${t}_result2.txt > /dev/null; then \
                        echo "$$t: PASS (Matches Result 2)"; \
                    else \
                        echo "$$t: FAIL"; \
                        # diff -y result.tmp $${t}_result.txt; \
                    fi; \
                fi); \
            done; \
        done
