-include .env
export

compile: src/peer.c src/config/config.c src/logger/logger.c src/peers/peers.c src/utils/utils.c
	gcc -o bin/peer src/peer.c src/config/config.c src/logger/logger.c src/peers/peers.c src/utils/utils.c -I.

dev: compile
	bin/peer

execute: bin/peer
	bin/peer
