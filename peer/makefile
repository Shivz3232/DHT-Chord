-include .env
export

compile: src/peer.c src/config/config.c src/logger/logger.c src/peer/peer.c src/utils/utils.c src/bootstrap/bootstrap.c
	gcc -o bin/peer src/peer.c src/config/config.c src/logger/logger.c src/peer/peer.c src/utils/utils.c src/bootstrap/bootstrap.c -I.

dev: compile
	bin/peer

execute: bin/peer
	bin/peer
