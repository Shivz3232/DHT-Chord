-include .env
export

compile: src/client.c src/config/config.c src/logger/logger.c src/utils/utils.c src/bootstrap/bootstrap.c src/testcase/testcase.c
	gcc -o bin/client src/client.c src/config/config.c src/logger/logger.c src/utils/utils.c src/bootstrap/bootstrap.c src/testcase/testcase.c -I.

dev: compile
	bin/client

execute: bin/client
	bin/client
