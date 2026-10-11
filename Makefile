build:
	cc -Wall -Wextra tree_parser.c -o build -lm -g

test:
	cc -Wall -Wextra tests.c tree_parser.c -o test -lm -g

clean:
	rm test
	rm build
