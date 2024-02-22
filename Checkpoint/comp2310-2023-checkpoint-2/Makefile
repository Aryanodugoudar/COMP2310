CC=gcc
CFLAGS=-g -Wall

all: db_engine

db_engine : hashing.c db_engine.c hashing.h
	"$(CC)" $(CFLAGS) -o $@ $< $@.c 

.PHONY: clean

clean: 
	rm -rf ./db_engine ./output/* *.dSYM 


