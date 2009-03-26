# The dependency block that is used for both direct and indirect dependencies
# include is used because a function would have issues with scope

for(dep,QTOPIA_DEPENDS) {
    containstext($$dep,*) {
        QTOPIA_DEPENDS-=$$dep
        wildcard=$$fetch_wildcard($$dep)
        isEmpty(wildcard) {
            files=$$files($$fixpath($$PROJECT_ROOT/$$dep))
            for(file,files) {
                file~=s,$$fixpath($$PROJECT_ROOT/),,q
                pro=$$file/$$tail($$file).pro
                # Make sure that the project exists
                # Make sure that the project is enabled (in PROJECTS)
                # Don't let a project depend on itself
                exists($$PROJECT_ROOT/$$pro):contains(PROJECTS,$$file):!equals(s,$$file) {
                    echo(Dependency $$dep caused dependency on $$file)
                    wildcard+=$$file
                }
            }
            store_wildcard($$dep,$$wildcard)
        } else {
            wildcard=$$split(wildcard," ")
            for(file,wildcard) {
                echo(Dependency $$dep caused dependency on $$file)
            }
        }

        QTOPIA_DEPENDS+=$$wildcard
    }
}

for(it,QTOPIA_DEPENDS) {
    contains(PROCESSED_DEPS,$$it):next()
    PROCESSED_DEPS+=$$it
    echo($$self depends on $$it)
    found=0
    foundempty=0
    founddisabled=0
    # Start looking in the current project root
    ROOTS=$$PROJECT_ROOT
    # Then look in Qtopia
    qtopia|part_of_qtopia:ROOTS+=$$QTOPIA_PROJECT_ROOT
    # Then Qtopia Sync Agent
    qtopiadesktop|part_of_qtopiadesktop:ROOTS+=$$QTOPIADESKTOP_PROJECT_ROOT
    # Then everything else
    ROOTS*=$$PROJECT_ROOTS
    for(root,ROOTS) {
        file=$$root/$$it/$$tail($$it).pro
        echo(Trying $$file)
        exists($$file) {
            !isProjectEnabled($$root,$$it) {
                foundempty=1
                founddisabled=1
                next()
            }
            profile=$$tail($$file)
            echo(Using $$file)
            found=1
            cmds=$$fromfile($$file,$$DEP_VAR)
            equals(DEP_VAR,DEP_CMDS):isEmpty(cmds) {
                found=0
                foundempty=1
                next()
            }
            for(c,cmds) {
                contains(QMAKE_BEHAVIORS,keep_quotes) {
                    c~=s/^"//
                    c~=s/"$//
                }
                # License info comes through with __SELF__ (because there's no way to check what
                # $$self would be from the other side) so we need to replace __SELF__ with $$profile
                c~=s/__SELF__/$$profile/qg
                echo(dep: $$c)
                eval($$c)
                # Handle any nowarn dependencies
                QTOPIA_DEPENDS-=$$QTOPIA_DEPENDS_NO_WARN
            }
            break()
        }
    }
    equals(found,0) {
        !equals(foundempty,1) {
            warning($$self depends on $$it/$$tail($$it).pro but it doesn't exist in $$ROOTS!)
            requires($$it/$$tail($$it).pro)
        }
        equals(founddisabled,1) {
            disabled_deps_are_non_fatal {
                warning($$self depends on $$it but it is not listed in PROJECTS!)
                requires(contains(PROJECTS,$$it))
            } else {
                error($$self depends on $$it but it is not listed in PROJECTS!)
            }
        }
    }
}

