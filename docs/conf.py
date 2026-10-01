# Copyright (c) 2026 RBR Ltd.
# SPDX-License-Identifier: Apache-2.0

# Sphinx configuration for the libRBR documentation.
#
# Build with `make html` in this directory. Doxygen must run first (the
# Makefile handles this) to produce the XML consumed by Breathe.

import os
from pathlib import Path

DOC_DIR = Path(__file__).resolve().parent

project = "libRBR"
copyright = "2018-2026, RBR Ltd."
author = "RBR Ltd."
# LIB_VERSION comes from the Makefiles (tools/version.sh); fall back to the
# VERSION file when sphinx-build is run by hand.
release = os.environ.get("LIB_VERSION") or (DOC_DIR.parent / "VERSION").read_text().splitlines()[0].strip()
version = release

extensions = [
    "breathe",
    "sphinx_rtd_theme",
]

templates_path = ["_templates"]
exclude_patterns = ["_build"]

primary_domain = "c"
highlight_language = "c"

html_theme = "sphinx_rtd_theme"
html_logo = str(DOC_DIR.parent / "res" / "rbr.svg")
html_theme_options = {
    "logo_only": True,
}

breathe_projects = {"librbr": str(DOC_DIR / "_build" / "xml")}
breathe_default_project = "librbr"
breathe_domain_by_extension = {"h": "c"}
breathe_default_members = ("members", "undoc-members")
