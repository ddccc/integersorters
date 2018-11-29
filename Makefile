all: integersorters

integersorters: compare2.c
	gcc -Wall -Werror -lm -o $@ $^

clean:
	$(RM) integersorters


