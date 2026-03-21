build:
	cc -Wall -Wextra tree_parser.c -o parse -lm
test:
clean: 
	rm parse
	