GITFLAGS = -m "Makefile auto commit" --no-verify --allow-empty

parser:	syntax.tab.c 
	gcc syntax.tab.c -lfl -ly -o parser
syntax.tab.c: lex.yy.c
	bison -d -v syntax.y
#scanner: lex.yy.c
#	gcc lex.yy.c -lfl -o scanner
lex.yy.c:
	flex lexical.l

# 定义伪目标
.PHONY: play clean git
clean:
	rm -f parser lex.yy.c syntax.tab.c syntax.tab.h syntax.output

play:	parser
	./parser test.txt
pull:
	-@git pull origin master
push:
	-@git push origin master
git:	
	-@git add . --ignore-errors &> /dev/null # KEEP IT
	-@git commit $(GITFLAGS) # KEEP IT


