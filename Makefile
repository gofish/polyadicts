#!/usr/bin/make

PYTHON = python3
SETUP = setup.py
SETUPOPTS ?= --quiet
CTAGS = ctags

.PHONY: build test clean

all:	tags build test

tags:	src/*.h src/*.c
	$(CTAGS) -f $@ $?

build bdist bdist_egg:
	$(PYTHON) $(SETUP) $(SETUPOPTS) $@

test:   build
	$(PYTHON) test/

clean:
	$(PYTHON) $(SETUP) $(SETUPOPTS) clean --all

distclean: clean
	$(RM) -r dist/ *.egg-info/
