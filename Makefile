PROJECT := test asynpool trace simple-server md5-server dstalgo \
	io-multp udp-server

ifeq ($(MAKECMDGOALS),)
all:$(PROJECT)
else
$(MAKECMDGOALS):$(PROJECT)
endif

io-multp:trace
md5-server:trace dstalgo
simple-server:trace
test:trace
asynpool:trace
dstalgo:trace
udp-server:trace

$(PROJECT):FORCE
	make -C $@ $(MAKECMDGOALS)
FORCE:
