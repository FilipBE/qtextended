foo.hint=image
foo.files=foo.txt
foo.path=/foo

bar.hint=image
bar.files=bar.txt
bar.path=/foo
bar.depends=install_foo

baz.hint=image
baz.files=baz.txt
baz.path=/foo
baz.depends=install_bar
