build:
	cc -Wall -Wextra tree_parser.c -o parse -lm -g
test:
clean: 
	rm parse
	