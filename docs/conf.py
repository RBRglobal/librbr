# Copyright (c) 2026 RBR Ltd.
# SPDX-License-Identifier: Apache-2.0

# Sphinx configuration for the libRBR documentation.
#
# Build with `make html` in this directory. Doxygen must run first (the
# Makefile handles this) to produce the XML consumed by Breathe.

import os
from pathlib import Path

from docutils import nodes
from sphinx import addnodes

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


def _as_fields(app, doctree):
    """Lay out \\command, \\return, and \\see sections like Parameters.

    Breathe renders \\command (a "Command:" or "Commands:" paragraph, from the
    Doxyfile alias) as a definition list, \\see as an admonition box, and each
    \\return as a field of its own; show each as a single field listing its
    entries, which takes far less room. Descriptions nest (a struct's members
    sit inside the struct's), so each section is moved only within the
    description it was written in.
    """
    for content in doctree.findall(addnodes.desc_content):

        def own(nodes_):
            return [n for n in nodes_ if _owner(n) is content]

        fields = []
        for item in own(content.findall(nodes.definition_list_item)):
            title = item[0].astext().rstrip(":")
            if title in ("Command", "Commands"):
                fields.append((title, item[1].children))
                _remove(item)
        see = []
        for node in own(content.findall(addnodes.seealso)):
            see.extend(node.children)
            _remove(node)
        if see:
            fields.append(("See also", see))
        lists = [n for n in content.children if isinstance(n, nodes.field_list)]
        if not lists and fields:
            lists = [nodes.field_list()]
            content.append(lists[0])
        for title, body in fields:
            field = nodes.field("", nodes.field_name("", title), nodes.field_body("", *body))
            if title == "See also":
                lists[0].append(field)
            else:
                lists[0].insert(0, field)
        for field_list in lists:
            # Sphinx names the field "Return" for a function pointer typedef.
            returns = [f for f in field_list.children if f[0].astext() in ("Returns", "Return")]
            for field in returns[1:]:
                returns[0][1].extend(field[1].children)
                field_list.remove(field)


def _owner(node):
    """Return the description which directly contains a node."""
    node = node.parent
    while not isinstance(node, addnodes.desc_content):
        node = node.parent
    return node


def _remove(node):
    """Remove a node, and the paragraph which held only it."""
    parent = node.parent
    parent.remove(node)
    if isinstance(parent, nodes.paragraph) and not parent.children:
        parent.parent.remove(parent)


def setup(app):
    app.connect("doctree-read", _as_fields)
