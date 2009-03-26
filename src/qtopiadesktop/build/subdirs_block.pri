# include is used because a function would have issues with scope

for(s,SUBDIRS) {
    contains(PROCESSED_SUBDIRS,$$s):next()
    PROCESSED_SUBDIRS+=$$s
    file=$$fixpath($$SRCDIR/$$s/$$tail($$s).pro)
    target=$$maketarget($$s)
    !exists($$file) {
        file=$$fixpath($$s/$$tail($$s).pro)
        # .pro files are in the depot, not the shadow
        file~=s,$$fixpath($$QPEDIR/),$$fixpath($$QTOPIA_DEPOT_PATH/),q
        foundroot=0
        for(root,PROJECT_ROOTS) {
            containstext($$file,$$fixpath($$root/)) {
                foundroot=1
                tmp=$$s
                tmp~=s,$$fixpath($root/),,q
                target=$$maketarget($$tmp)
                break()
            }
        }
        !equals(foundroot,1):error(Could not find directory $$s in any of the project roots (looking for $$tail($$s).pro))
    }
    !exists($$file):error(Missing $$file)
    cachedinfo($$file,QTOPIA_DEPENDS,CONFIG)
    # This is just too brittle to work correctly. The cache file can't be trusted for dependencies while
    # the depends() statement can be conditional or BlackMagic.pm can't process conditionals.
    #message(cache.QTOPIA_DEPENDS $$cache.QTOPIA_DEPENDS)
    #equals(cache.QTOPIA_DEPENDS,",fake") {
        cache.QTOPIA_DEPENDS=$$fromfile($$file,QTOPIA_DEPENDS)
    #}
    #contains(subdirconf,syncqtopia):message(syncqtopia:YES $$file)
    #else:message(syncqtopia: NO $$file)
    QTOPIA_DEPENDS=$$cache.QTOPIA_DEPENDS
    noprintdeps=
    for(dep,QTOPIA_DEPENDS) {
        containstext($$dep,*) {
            echo($$s depends on $$dep)
            !equals(printsubdirsdependrules.commands,$$COMMAND_HEADER):printsubdirsdependrules.commands+=$$LINE_SEP
            printsubdirsdependrules.commands+=\
                echo DEPENDS: $$s depends on $$LITERAL_SQUOTE$$dep$$LITERAL_SQUOTE
            QTOPIA_DEPENDS-=$$dep
            wildcard=$$fetch_wildcard($$dep)
            isEmpty(wildcard) {
                files=$$files($$fixpath($$PROJECT_ROOT/$$dep))
                for(file,files) {
                    file=$$fixpath($$file)
                    file~=s,$$fixpath($$PROJECT_ROOT/),,q
                    dir=$$file
                    pro=$$fixpath($$file/$$tail($$file).pro)
                    # Make sure that the project exists
                    # Don't let a project depend on itself
                    exists($$fixpath($$PROJECT_ROOT/$$pro)):!equals(s,$$file) {
                        wildcard+=$$unixpath($$file)
                        dir=$$unixpath($$dir)
                        echo($$s depends on $$dir)
                        printsubdirsdependrules.commands+=$$LINE_SEP\
                            echo DEPENDS: $$LITERAL_SQUOTE$$dep$$LITERAL_SQUOTE depends on $$dir
                        noprintdeps*=$$dir
                    }
                }
                store_wildcard($$dep,$$wildcard)
            } else {
                wildcard=$$split(wildcard," ")
                for(dir,wildcard) {
                    echo($$s depends on $$dir)
                    printsubdirsdependrules.commands+=$$LINE_SEP\
                        echo DEPENDS: $$LITERAL_SQUOTE$$dep$$LITERAL_SQUOTE depends on $$dir
                    noprintdeps*=$$dir
                }
            }
            #message(QTOPIA_DEPENDS+=$$wildcard)
            QTOPIA_DEPENDS+=$$wildcard
        }
    }
    subdirconf=$$cache.CONFIG
    for(dep,QTOPIA_DEPENDS) {
        !isEmpty(search):dep~=s,^$$search,,
        deptarget=$$maketarget($$dep)
        !contains(noprintdeps,$$dep) {
            echo($$s depends on $$dep)
            printsubdirsdependrules.commands+=$$LINE_SEP\
                echo DEPENDS: $$s depends on $$dep
        }
        # If we can see the dependency, make sure it is being built
        #exists($$SRCDIR/$$dep/$$tail($$dep).pro):SUBDIRS*=$$dep
        exists($$SRCDIR/$$dep/$$tail($$dep).pro):!contains(SUBDIRS,$$dep):echo($$dep is visible but it's not in SUBDIRS)

        # Our rules depend on the rules for the projects we depend on.
        # This will create some bad entries but they will be pruned later.
        for(t,TARGETS) {
            qt=$$t
            contains(QMAKE_TARGETS,$$t):qt=qtopia_$$t

            equals(t,all) {
                thisrule=$$target
                deprule=$$deptarget
            } else {
                # We only do syncqtopia for projects with CONFIG+=syncqtopia (ie. libs and subdirs)
                contains(SYNCQTOPIA,$$t):!contains(subdirconf,syncqtopia):next()
                thisrule=$${target}_$$qt
                deprule=$${deptarget}_$$qt
            }
            # sub-[dir]_foo -> sub-[dep]_foo
            eval($${thisrule}.depends*=\$$deprule)
        }
    }

    for(t,TARGETS) {
        commands=
        qt=$$t
        contains(QMAKE_TARGETS,$$t):qt=qtopia_$$t

        equals(t,all) {
            commands+=@$$MAKE -C $$fixpath($$s)
            ignore_subdir_errors:commands+=|| $$DUMMY_COMMAND
            eval($${target}.commands=\$$commands)
            QMAKE_EXTRA_TARGETS+=$$target
            thisrule=$$target
        } else {
            do=1
            # first_syncqtopia has a dot printed (it runs at configure time)
            equals(t,first_syncqtopia):commands+=$$PRINTDOT $$LINE_SEP_VERBOSE
            # We only do CONFIG+=syncqtopia (libs and subdirs) for syncqtopia
            !contains(SYNCQTOPIA,$$t)|contains(subdirconf,syncqtopia) {
                commands+=@$$MAKE -C $$fixpath($$s) $$t
                ignore_subdir_errors:commands+=|| $$DUMMY_COMMAND
            }
            eval($${target}_$${qt}.commands=\$$commands)
            QMAKE_EXTRA_TARGETS+=$${target}_$${qt}
            thisrule=$${target}_$$qt
        }
        # make foo -> sub-[dir]_foo
        eval($${qt}.depends*=\$$thisrule)
    }
}

