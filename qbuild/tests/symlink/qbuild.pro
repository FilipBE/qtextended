#message(including foo/bar/bar.pri)
include(foo/bar/bar.pri)
#message(including bar/bar.pri)
include(bar/bar.pri)
<script>
var proj;

proj = project.sproject("foo/bar/bar.pri").fileModeProject()
project.message("bar.pri is "+proj.property("BAR").strValue());
proj = project.sproject("bar/bar.pri").fileModeProject()
project.message("bar.pri is "+proj.property("BAR").strValue());

proj = project.sproject("foo/bar/bar.pri").project()
project.message("bar.pri is "+proj.property("BAR").strValue());
proj = project.sproject("bar/bar.pri").project()
project.message("bar.pri is "+proj.property("BAR").strValue());
</script>
