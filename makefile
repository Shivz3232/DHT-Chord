build-bootstrap: bootstrap
	docker build bootstrap -t prj5-bootstrap

build-client: client
	docker build client -t prj5-client

build-peer: peer
	docker build peer -t prj5-peer

build: build-bootstrap build-client build-peer
