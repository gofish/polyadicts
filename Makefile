#!/usr/bin/make

PYTHON = python3
SETUP = setup.py
SETUPOPTS ?= --quiet

.PHONY: build

all:	tags build

tags:	src/*.h src/*.c
	ctags -f $@ $?

build bdist bdist_egg:
	$(PYTHON) $(SETUP) $(SETUPOPTS) $@

clean:
	$(PYTHON) $(SETUP) $(SETUPOPTS) clean --all
	$(RM) -r dist/ *.egg-info/
