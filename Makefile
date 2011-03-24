#!/usr/bin/make

PYTHON = python3
SETUP = setup.py
SETUPOPTS ?= --quiet

.PHONY: build test clean

all:	tags build test

tags:	src/*.h src/*.c
	ctags -f $@ $?

build bdist bdist_egg:
	$(PYTHON) $(SETUP) $(SETUPOPTS) $@

test:
	$(PYTHON) test/

clean:
	$(PYTHON) $(SETUP) $(SETUPOPTS) clean --all
	$(RM) -r dist/ *.egg-info/
