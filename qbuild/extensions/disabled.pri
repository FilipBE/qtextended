# This is for directories that have been disabled with disable_project()
#message(disabled.pri)
CONFIG+=projects
SUBDIRS=

<script>
project.property("MESSAGE").setValue("Project "+project.name+" is disabled.");
if ( project.disabledReason() )
    project.property("MESSAGE").append("("+project.disabledReason()+")");

// Force an error
if ( project.resetReason().contains("disable_error") )
    project.property("CONFIG").append("force_error");

// Warn about requested projects that are disabled
if ( projects_expected() ) {
    if ( project.resetReason().contains("depends") ) {
        var msg = "Project is listed in PROJECTS and was disabled due to a dependency.";
        if ( project.disabledReason() )
            msg += " ("+project.disabledReason()+")";
        project.warning(msg);
    } else if ( project.config("strict_projects") ) {
        var msg = "Project is listed in PROJECTS and was disabled.";
        if ( project.disabledReason() )
            msg += " ("+project.disabledReason()+")";
        project.warning(msg);
    }
}
</script>

# By default we print a message and die
default.TYPE=RULE
default.commands=\
    "#(ev)echo """$$MESSAGE""""\
    "$$error(Fatal error)"

# When invoked by subdirs rules we print a message (if -disabled is passed)
# This dies if force_error is set.
default_sub.TYPE=RULE
show_disabled=$$globalValue(showDisabled)
equals(show_disabled,1)|force_error:default_sub.commands="#(ev)echo """$$MESSAGE""""
force_error:default_sub.commands+="$$error(Fatal error)"

# The check_enabled rule just runs default (ie. print a message and die)
check_enabled.TYPE=RULE
check_enabled.prerequisiteActions=default

