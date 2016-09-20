REBAR ?= ./rebar

all: compile

get-deps:
	./c_src/build_deps.sh get-deps

deps:
	${REBAR} get-deps

rm-deps:
	./c_src/build_deps.sh rm-deps

compile: deps
	${REBAR} compile
	cp priv/libprofiler.so ${BASHO_PERF_ROOT}

clean:
	${REBAR} clean

.PHONY: python

python:
	cd python; make

include tools.mk
