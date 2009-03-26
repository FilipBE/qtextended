CONFIG+=foo bar
foo {
    message(braces OK)
} else {
    warning(braces FAIL)
}
foo {
    message(braces2 OK)
} else:warning(braces2 FAIL)
foo:message(braces3 OK)
else {
    warning(braces3 FAIL)
}
foo:message(inline OK)
else:warning(inline FAIL)
foo|bar:message(or OK)
else:warning(or FAIL)
foo:bar:message(and OK)
else:warning(and FAIL)
message(non-scoped OK)
exists(qbuild.solution):message(func OK)
else:warning(func FAIL)
!exists(qbuild.solution):warning(func2 FAIL)
else:message(func2 OK)
