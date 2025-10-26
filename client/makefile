-include .env
export

compile: src/client.c src/config/config.c src/logger/logger.c src/utils/utils.c
	gcc -o bin/client src/client.c src/config/config.c src/logger/logger.c src/utils/utils.c -I.

dev: compile
	bin/client

execute: bin/client
	bin/client
