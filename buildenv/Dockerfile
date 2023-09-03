FROM randomdude/gcc-cross-x86_64-elf
FROM ubuntu

RUN apt-get clean
RUN apt-get update
RUN apt-get upgrade -y

RUN apt-get install -y gcc
RUN apt-get install -y binutils
RUN apt-get install -y make
RUN apt-get install -y nasm
RUN apt-get install -y xorriso
RUN apt-get install -y grub-pc-bin
RUN apt-get install -y grub-common

VOLUME /root/env
WORKDIR /root/env