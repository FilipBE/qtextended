requires(enable_qvfb)
system_qt {
TEMPLATE=app
CONFIG+=qt
TARGET=qvfb
TARGETDIR=QtopiaSdk:/qtopiacore/host/bin

SOURCEPATH=\
    /qtopiacore/qt/tools/qvfb\
    /qtopiacore/qt/src/gui/embedded\
    /qtopiacore/qt/tools/shared/deviceskin

<script>
function fetchvarsfromfile(file)
{
    var proj = project.sproject(file).fileModeProject();
    var variables = new Array("FORMS", "HEADERS", "SOURCES", "RESOURCES", "INCLUDEPATH", "LIBS");
    for ( var ii in variables ) {
        var varname = variables[ii];
        if ( proj.isProperty(varname) )
            project.property(varname).unite(proj.property(varname).value());
    }
}
fetchvarsfromfile("qvfb.pri");
</script>

DEFINES+=QT_BEGIN_NAMESPACE=
DEFINES+=QT_END_NAMESPACE=
}

# Symlink the skins into the directory runqtopia is expecting them to be in
<script>
var destpath = qbuild.invoke("path", "QtopiaSdk:/src/tools/qt/qvfb", "generated");
var qvfbpath = qbuild.invoke("path", "/qtopiacore/qt/tools/qvfb", "existing");
var devicepath = qbuild.invoke("path", "/devices", "project");

var symlink_skins = project.rule("symlink_skins");

var skinfiles = new Array();

function dopath(path)
{
    var pi = new PathIterator(path);
    // We only accept .skin directories (not the ancient .skin files)
    var ret = pi.paths("*.skin");
    for ( var ii in ret ) {
       var path = pi.filesystemPath()[0];
       path += "/"+ret[ii];
       // Only get skins that have a .skin file in them!
       if ( pi.cd(ret[ii]).files(ret[ii]).length )
           skinfiles.push(path)
    }
}

dopath(qvfbpath);
pi = new PathIterator(devicepath);
var devices = pi.paths();
for ( var ii in devices ) {
    var dpi = pi.cd(devices[ii]);
    dopath(dpi.filesystemPath()[0]);
}

var processedSkins = new Object();
processedSkins.contains = function(key) {
    for ( var ii in this ) {
        if ( ii == key )
            return 1;
    }
    return 0;
};

for ( var ii in skinfiles ) {
    var skin = skinfiles[ii];
    var skinname = basename(skin);
    if ( processedSkins.contains(skinname) ) {
        project.warning("Skin "+skinname+" exists in multiple locations:\n  "+skin+"\n  "+processedSkins[skinname]);
        continue;
    }
    processedSkins[skinname] = skin;

    var rule = project.rule();
    rule.inputFiles.append(skin);
    rule.outputFiles.append(destpath+"/"+skinname);
    rule.other.append(destpath);
    rule.commands.append("#(e)$$MKSPEC.MKDIR $$[OTHER.0]");
    rule.commands.append("#(eh)echo \"\"symlink $$[OUTPUT.0]\"\"");
    rule.commands.append("#(e)rm -f $$[OUTPUT.0]");
    rule.commands.append("#(e)ln -s $$[INPUT.0] $$[OUTPUT.0]");

    symlink_skins.prerequisiteActions.append(rule.name);
}
</script>
default.TYPE=RULE
default.prerequisiteActions+=symlink_skins

image.TYPE=RULE
image.prerequisiteActions+=symlink_skins
