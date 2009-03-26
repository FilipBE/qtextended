FOO=foo bar baz
OUTPUT=
f=what
for(f,FOO) {
    message($$f)
    OUTPUT+=$$f
}
message($$OUTPUT)
!equals(f,what):error(f was changed!)
