# Copyright (c) 2026 RBR Ltd.
# SPDX-License-Identifier: Apache-2.0

# Sphinx documentation build. `make html` runs Doxygen (XML only) and then
# Sphinx; output lands in _build/html/. Requires the packages listed in
# requirements.txt plus the doxygen executable.

SPHINXOPTS    ?=
SPHINXBUILD   ?= sphinx-build
DOXYGEN       ?= doxygen
SOURCEDIR     = .
BUILDDIR      = _build
# Version shown in the documentation; conf.py reads it from the environment.
# Inherited from the root Makefile when invoked via `make docs`.
export LIB_VERSION ?= $(shell cd .. && ./tools/version.sh)

.PHONY: help doxygen clean Makefile

help:
	@$(SPHINXBUILD) -M help "$(SOURCEDIR)" "$(BUILDDIR)" $(SPHINXOPTS)

doxygen:
	$(DOXYGEN) Doxyfile

clean:
	rm -rf "$(BUILDDIR)"

# Route every other target (html, linkcheck, ...) through sphinx-build, after
# regenerating the Doxygen XML.
%: Makefile doxygen
	@$(SPHINXBUILD) -M $@ "$(SOURCEDIR)" "$(BUILDDIR)" $(SPHINXOPTS)
