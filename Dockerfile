FROM ubuntu:22.04

RUN apt-get update
RUN apt-get install -y gcc
RUN apt-get install make

ENV APP_HOME=/usr/src/prj4

RUN mkdir -p ${APP_HOME}
WORKDIR ${APP_HOME}

RUN mkdir -p ${APP_HOME}/src
COPY src ${APP_HOME}/src

COPY hostsfile.txt ${APP_HOME}

RUN mkdir -p ${APP_HOME}/bin

COPY .env* .
COPY makefile .

RUN make compile

ENTRYPOINT ["bin/prj4"]
