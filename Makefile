#!/usr/bin/make

PYTHON = python3
SETUP = setup.py
SETUPOPTS ?= --quiet
CTAGS = ctags

.PHONY: build test clean

all:	tags build test doc

tags:	src/*.h src/*.c
	$(CTAGS) -f $@ $?

build bdist bdist_egg:
	$(PYTHON) $(SETUP) $(SETUPOPTS) $@

test:   build
	$(PYTHON) -B test/

clean:
	$(PYTHON) $(SETUP) $(SETUPOPTS) clean --all

distclean: clean
	$(RM) -r dist/ *.egg-info/ README.html

doc:    README.html

README.html: | style.css

%.html: %.md
	pandoc --standalone $(addprefix -c$(space),$(filter %.css,$|)) --from markdown --to html -o $@ $<
