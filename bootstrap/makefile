-include .env
export

compile: src/bootstrap.c src/config/config.c src/logger/logger.c src/peers/peers.c src/utils/utils.c src/orchestrate/orchestrate.c
	gcc -o bin/bootstrap src/bootstrap.c src/config/config.c src/logger/logger.c src/peers/peers.c src/utils/utils.c src/orchestrate/orchestrate.c -I.

dev: compile
	bin/bootstrap

execute: bin/bootstrap
	bin/bootstrap
