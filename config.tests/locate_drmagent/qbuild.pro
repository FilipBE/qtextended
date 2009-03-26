CONFIG+=embedded

printsearchdirs.TYPE=RULE
printsearchdirs.commands=$$MKSPEC.CC" -print-search-dirs"

default.TYPE=RULE
default.prerequisiteActions=printsearchdirs
