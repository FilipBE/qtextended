CONFIG+=foo

# Originally, :{ evaluated to false so it was turned into a parser error
# However qmake handles :{ by silently ignoring the : so QBuild must do the same
# Now it is supposed to work the same in QBuild as it does in qmake

# The first fix broke this syntax
message(foo())

foo:{
    message("foo:{ OK")
} else {
    error("foo:{ FAIL")
}

foo: {
    message("foo: { OK")
} else {
    error("foo:{ 2 FAIL")
}

foo :{
    message("foo :{ OK")
} else {
    error("foo:{ 2 FAIL")
}

foo : {
    message("foo : { OK")
} else {
    error("foo:{ 2 FAIL")
}

bar:{
    error("bar:{ FAIL")
} else {
    message("bar:{ OK")
}

