CXX = g++
CXXFLAGS = -I./src -std=c++11 -lstdc++

LEXER = src/lexer.l
PARSER = src/parser.y
MAIN = src/main.cpp
OUTPUT = compiler

LEX_SRC = src/lex.yy.c
PARSER_SRC = src/parser.tab.cc
PARSER_HEADER = src/parser.tab.hh

SRC = $(LEX_SRC) $(PARSER_SRC)

all: $(OUTPUT)

$(OUTPUT): $(SRC) $(MAIN)
	$(CXX) -o $@ $^ $(CXXFLAGS)
	rm -f $(SRC) $(PARSER_HEADER)

$(LEX_SRC): $(LEXER)
	cd src && flex lexer.l

$(PARSER_SRC) $(PARSER_HEADER): $(PARSER)
	cd src && bison -d parser.y

clean:
	rm -f $(OUTPUT) $(SRC) $(PARSER_HEADER)

.PHONY: all clean
