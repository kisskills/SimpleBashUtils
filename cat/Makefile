CFLAGS:=-Wall -Werror -Wextra -std=c11

CAT:=s21_cat

all: $(CAT)

$(CAT): s21_cat.c
	gcc $(CFLAGS) s21_cat.c -o $(CAT)

tests: $(CAT)
	sh t_cat.sh

style:
	cp ../../materials/linters/CPPLINT.cfg CPPLINT.cfg
	python3 ../../materials/linters/cpplint.py --extension=c *.c

clean:
	rm $(CAT)

rebuild: clean all