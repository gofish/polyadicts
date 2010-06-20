#!/usr/bin/make

PYTHON = python3

.PHONY: build

all:	tags build

build bdist bdist_egg:
	$(PYTHON) setup.py $@

clean:
	$(PYTHON) setup.py clean --all
	$(RM) -r dist/ *.egg-info/

tags:	*.h *.c
	ctags -f $@ $?
