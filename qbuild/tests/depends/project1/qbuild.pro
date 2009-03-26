MODULE_NAME=project1

DEPEND.TYPE=DEPENDS IMMEDIATE
DEPEND.EVAL="FOO*=bar"

DEP2.TYPE=DEPENDS PERSISTED
DEP2.EVAL="BAR*=foo"

DEP3.TYPE=DEPENDS PERSISTED
DEP3.EVAL=\
    "FOO=foo bar"\
    "BAR=""foo bar"""\
    "contains(FOO,foo):message(FOO)"\
    "contains(BAR,foo):message(BAR)"
MODULES=project6

