#!/usr/bin/make

PYTHON = python3
SETUP = setup.py
SETUPOPTS ?= --quiet

.PHONY: build test clean

all:	build test doc

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
