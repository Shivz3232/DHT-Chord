-include .env
export

compile: src/prj4.c src/config/config.c src/logger/logger.c src/peers/peers.c src/utils/utils.c
	gcc -o bin/bootstrap src/prj4.c src/config/config.c src/logger/logger.c src/peers/peers.c src/utils/utils.c -I.

dev: compile
	bin/bootstrap

execute: bin/bootstrap
	bin/bootstrap
