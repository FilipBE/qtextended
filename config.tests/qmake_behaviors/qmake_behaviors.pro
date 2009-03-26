QMAKE_BEHAVIORS=

foo="bar"
bar=bar
!equals(foo,$$bar):QMAKE_BEHAVIORS+=keep_quotes

foo=$$quote(\n)
equals(foo,\n):QMAKE_BEHAVIORS+=quote_is_escape_expand

defineTest(foo) {
    count(1,1):QMAKE_BEHAVIORS+=func_in_join
    export(QMAKE_BEHAVIORS)
}
foo(foo bar)

defineReplace(bar) {
    foo=foo bar
    return($$foo)
}
foo=$$bar()
count(foo,1):QMAKE_BEHAVIORS+=func_out_join

foo="foo bar"
contains(QMAKE_BEHAVIORS,keep_quotes) {
    foo~=s/^"//
    foo~=s/"$//
}
bar=$$foo
count(bar,2):QMAKE_BEHAVIORS+=var_split

foo=foo bar
bar=$$foo
count(bar,1):QMAKE_BEHAVIORS+=var_join

foo=foo
bar=bar
baz=$$foo$$bar
equals(baz,foo\$$bar):QMAKE_BEHAVIORS+=var_parse

defineTest(cfoo) {
    !count(1,1):QMAKE_BEHAVIORS+=combine_func_args
}
cfoo(1,2)

message(QMAKE_BEHAVIORS: $$QMAKE_BEHAVIORS)

