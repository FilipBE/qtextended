TEMPLATE=app
TARGET=qpe

CONFIG+=qtopia
enable_singleexec:CONFIG+=qt_static_plugins

STRING_LANGUAGE=en_US
AVAILABLE_LANGUAGES=$$QTOPIA_AVAILABLE_LANGUAGES
LANGUAGES=$$QTOPIA_LANGUAGES
TS_DIR=$$path(main,project)

pkg [
    name=server
    desc="Server for Qt Extended."
    version=$$QTOPIA_VERSION
    license=$$QTOPIA_LICENSE
    maintainer=$$QTOPIA_MAINTAINER
]

# Pull in server projects
<script>
var projects = project.property("SERVER_PROJECTS").value();
var server_pwd = project.property("SERVER_PWD");
var vpath = project.property("VPATH");
for ( var ii in projects ) {
    var proj = projects[ii];
    var path = "/src/"+proj;
    proj = path+"/server.pri";
    server_pwd.setValue(path);
    vpath.append(path);
    qbuild.invoke("include", proj);
    var reqs = project.property("REQUIRES").value();
    for ( var ii in reqs ) {
        requires(reqs[ii], "required by "+proj);
    }
    project.property("REQUIRES").setValue("");
}
var depends = project.property("SERVER_DEPS");
var list = depends.value();
depends.setValue("");
var bad = new Array;
for ( var ii in list ) {
    var proj = list[ii].replace(/^\/src\//, "").replace(/::.*/, "");
    if ( !project.property("SERVER_PROJECTS").contains(proj) )
        bad.push(proj);
}
if ( bad.length )
    qbuild.invoke("disable_project", "Required server projects are not enabled ("+bad.join(", ")+").");
</script>

# Pull in server.pri (legacy)
!isEmpty(device) {
    # If server.pri exists, pull it in
    exists($$DEVICE_SOURCE_PATH/server/server.pri) {
        #info(Using server/server.pri is deprecated. You should create a project and list it in SERVER_PROJECTS instead.)
        SERVER_PWD=$$DEVICE_SOURCE_PATH/server
        VPATH+=$$SERVER_PWD
        include($$DEVICE_SOURCE_PATH/server/server.pri)
    } else {
        pull_in_all=1
    }
}
# Pull in server/*.cpp (legacy)
<script>
if ( project.isProperty("device") && project.property("pull_in_all").strValue() == "1" ) {
    var forms = project.files("/devices/"+project.property("device").strValue()+"/server/*.ui");
    var headers = project.files("/devices/"+project.property("device").strValue()+"/server/*.h");
    var sources = project.files("/devices/"+project.property("device").strValue()+"/server/*.cpp");
    if ( headers.length || sources.length || forms.length ) {
        project.info("Pulling in server/*.ui, server/*.h and server/*.cpp is deprecated. You should create a project and list it in SERVER_PROJECTS instead.");
        for(var ii in forms)
            project.property("FORMS").append(forms[ii].path());
        for(var ii in headers)
            project.property("HEADERS").append(headers[ii].path());
        for(var ii in sources)
            project.property("SOURCES").append(sources[ii].path());
        project.property("INCLUDEPATH").append("/devices/"+project.property("device").strValue()+"/server");
    }
}
</script>

# Not needed anymore
SERVER_PWD=

# Install all headers into a single directory (makes commandlines shorter)
NON_PUBLIC_HEADERS+=$$HEADERS
PUBLIC_HEADERS_OVERRIDE+=$$HEADERS
HEADERS=
HEADERS_NAME=server
CONFIG+=headers

# The lupdate rule (for UNIFIED_NCT_LUPDATE)
nct_lupdate [
    TYPE=RULE
    commands="#(ve)[ ""$VERBOSE_SHELL"" = 1 ] && set -x
        QTOPIA_DEPOT_PATH="""$$path(/,project)"""
        QPEDIR="""$$path(/,generated)"""
        TS_DIR="""$$TS_DIR"""
        FIND_FILES="""$$path(find_files,project)"""
        AVAILABLE_LANGUAGES="""$$AVAILABLE_LANGUAGES"""
        STRING_LANGUAGE="""$$STRING_LANGUAGE"""
        cd $TS_DIR
        $FIND_FILES $QTOPIA_DEPOT_PATH |\
        $QPEDIR/src/build/bin/nct_lupdate\
            -nowarn\
            -depot\
            ""$QTOPIA_DEPOT_PATH""\
            ""$AVAILABLE_LANGUAGES""\
            ""$STRING_LANGUAGE""\
            -
    "
]
i18n_depend_on_qt(nct_lupdate)
lupdate.TYPE=RULE
lupdate.prerequisiteActions+=nct_lupdate

# Install .directory files
for(l,LANGUAGES) {
    file=$$path(/i18n/$$l/.directory,existing)
    isEmpty(file):warning("Cannot locate /i18n/"$$l"/.directory")
    directory.commands+=\
        "#(e)$$MKSPEC.MKDIR $$QTOPIA_IMAGE/i18n/"$$l\
        "$$MKSPEC.INSTALL_FILE "$$file" $$QTOPIA_IMAGE/i18n/"$$l
}
directory.hint=image

enable_singleexec {
    DEFINES+=SINGLE_EXEC
    <script>
    var projects = project.property("PROJECTS").value();
    for ( var ii in projects ) {
        var proj = projects[ii];
        if ( proj != "server" && !proj.match(/^server\/./) ) {
            var path = "/src/"+proj;
            proj = project.sproject(path).project();
            if (proj.config("enable_singleexec") && proj.config("singleexec")) {
                project.property("DEPENDS").unite(path);
            }
        }
    }
    </script>

    patchqt [
        commands=\
            "$$path(/src/build/bin/patchqt,generated) $$QTOPIA_IMAGE/bin/qpe $$QTOPIA_PREFIX"
        depends=install_target
        hint=image
    ]
}

# Let all symbols be visible (for plugins)
!enable_singleexec:MKSPEC.LFLAGS+=-rdynamic

EXTRA_TS_FILES=\
    QtopiaApplications QtopiaGames QtopiaSettings QtopiaI18N QtopiaServices\
    QtopiaNetworkServices QtopiaBeaming QtopiaColorSchemes QtopiaDefaults\
    QtopiaRingTones QtopiaThemes Categories-Qtopia

