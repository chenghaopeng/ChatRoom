GCC = gcc
SERVER = ./Server/server.c
CLIENT = ./Client/client.c

_SERVER = $(GCC) -o server $(SERVER)
_CLIENT = $(GCC) -o client $(CLIENT)

server:
	$(_SERVER)

client:
	$(_CLIENT)

all:
	$(_SERVER)
	$(_CLIENT)	

debug:
	$(_SERVER) -D DEBUG
	$(_CLIENT) -D DEBUG

rm:
	rm server client
