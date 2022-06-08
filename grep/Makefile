CFLAGS:=-Wall -Werror -Wextra -std=c11

GREP:=s21_grep

all: $(GREP)

$(GREP): s21_grep.c
	gcc $(CFLAGS) s21_grep.c -o $(GREP)

tests: $(GREP)
	sh t_grep.sh

style:
	cp ../../materials/linters/CPPLINT.cfg CPPLINT.cfg
	python3 ../../materials/linters/cpplint.py --extension=c *.c

clean:
	rm $(GREP)

rebuild: clean all