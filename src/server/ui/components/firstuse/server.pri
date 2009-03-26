SERVER_DEPS*=\
    /src/server/core_server

HEADERS+=firstuse.h
SOURCES+=firstuse.cpp

dynamic.TYPE=CONDITIONAL_SOURCES
dynamic.CONDITION=!enable_singleexec
dynamic.HEADERS+=\
        ../../../../settings/systemtime/settime.h\
        ../../../../settings/language/languagesettings.h
dynamic.FORMS=\
    ../../../../settings/language/languagesettingsbase.ui
dynamic.SOURCES=\
    ../../../../settings/language/language.cpp\
    ../../../../settings/language/langmodel.cpp\
    ../../../../settings/systemtime/settime.cpp

enable_singleexec {
    HEADERS+=\
        ../../../../settings/systemtime/settime.h\
        ../../../../settings/language/languagesettings.h
    MOC_COMPILE_EXCEPTIONS+=\
        ../../../../settings/systemtime/settime.h\
        ../../../../settings/language/languagesettings.h

    <script>
        var rule = project.rule();
        rule.prerequisiteActions = "/src/settings/language/ui_languagesettingsbase.h";
        rule.prerequisiteActions.append("#(oh)ensure_uicdir");
        //rule.inputFiles = qbuild.invoke("path", "/src/settings/language/.uic/ui_languagesettingsbase.h", "generated");
        rule.outputFiles = qbuild.invoke("path", "/src/server/.uic/ui_languagesettingsbase.h", "generated");
        rule.commands = "cp $$path(/src/settings/language/.uic/ui_languagesettingsbase.h,generated) $$[OUTPUT.0.ABS]";
        project.rule("target_pre").prerequisiteActions.append(rule.name);
    </script>
}


