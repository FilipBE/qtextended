/****************************************************************************
**
** This file is part of the Qt Extended Opensource Package.
**
** Copyright (C) 2009 Trolltech ASA.
**
** Contact: Qt Extended Information (info@qtextended.org)
**
** This file may be used under the terms of the GNU General Public License
** version 2.0 as published by the Free Software Foundation and appearing
** in the file LICENSE.GPL included in the packaging of this file.
**
** Please review the following information to ensure GNU General Public
** Licensing requirements will be met:
**     http://www.fsf.org/licensing/licenses/info/GPLv2.html.
**
**
****************************************************************************/

/*!

\extension packages

The packages extension handles installable packages.

\code
CONFIG+=packages
\endcode

Packages are objects with TYPE=PACKAGE.

\code
pkg.TYPE=PACKAGE
pkg.name=foo
pkg.desc="one line summary"
\endcode

\code
struct package {
    name       # eg. $name.ipk
    desc       # one line summary
    deps       # packages that this ipk depends on
    multi      # put multiple projects in a single package
    version    # package version
    license    # package license
    maintainer # package maintainer
    targets    # targets to run (ie. install stuff)
}
\endcode

*/

/*!

\qbuild_variable PACKAGES
\ingroup packages_extension

Enable additional packages.

*/

function packages_init()
{
    if ( project.absName.match(/\{file\}\/$/) )
        return;
###
    CONFIG*=installs
    QMAKE.FINALIZE.packages.CALL = packages_finalize
    QMAKE.FINALIZE.packages.RUN_BEFORE_ME = installs compiler i18n
###
}

function packages_finalize()
{
    if (!project.config("packages"))
        return;

### qbuild.root.node()
    PKG_PATH=$$(PKG_PATH)
    isEmpty(PKG_PATH):PKG_PATH=$$path(<0>pkg,generated)
###

    // If any pkg.foo variables are set, enable the pkg object.
    var list = ["name", "desc", "deps", "multi", "version", "license", "maintainer", "domain"];
    for (var jj in list) {
        if (project.isProperty("pkg."+list[jj]))
            project.property("pkg.TYPE").setValue("PACKAGE");
    }

    // Default values for the pkg object
###
    isEmpty(pkg.name):pkg.name=qpe-$$COMPILER.TARGET
    isEmpty(pkg.version):pkg.version=$$QPE_VERSION
    isEmpty(pkg.targets):pkg.targets=$$pkg.default_targets
    isEmpty(pkg.domain):pkg.domain=$$target.domain
    quicklaunch:pkg.quicklaunch=1
    else:pkg.quicklaunch=0
###

    var objs = project.find("TYPE", "PACKAGE");
    for (var ii in objs) {
        var obj = qbuild.object(objs[ii]);
### obj.name
        # Default values
        isEmpty(<0>.name):<0>.name=<0>
        isEmpty(<0>.version):<0>.version=1.0.0
        isEmpty(<0>.desc):<0>.desc=No description written for this package
        isEmpty(<0>.domain):<0>.domain=untrusted
        isEmpty(<0>.trust):<0>.trust=Untrusted
        isEmpty(<0>.license):<0>.license=Unspecified
        isEmpty(<0>.maintainer):<0>.maintainer=Unspecified
        isEmpty(<0>.quicklaunch):<0>.quicklaunch=0
###
        var list = obj.property("multi").value();
        var multi = new Array;
        for (var jj in list) {
            var dir = list[jj];
            if (!dir.match(/^\//))
                dir = "/src/"+dir;
            multi.push(qbuild.invoke("path", dir, "project"));
        }

        var rule = project.rule("package_"+obj.name);
        rule.commands.append("#(eh)echo mkpkg "+obj.property("name").strValue());
        rule.commands.append("#(e)$$path(QtopiaSdk:/src/build/bin/mkpkg,generated) "+
                             "$$shellQuote($$path(/bin/qbuild,existing)) "+
                             "$$shellQuote($$(FORMAT)) "+
                             "$$shellQuote($$path(.package_"+obj.name+",generated)) "+
                             "$$shellQuote($$arch) "+
                             "$$shellQuote(unused) "+
                             "$$shellQuote($$LANGUAGES) "+
                             "$$shellQuote($$PKG_PATH) "+
                             "$$shellQuote($$path(.,project)) "+
                             "$$shellQuote($$path(.,generated)) "+
                             "$$shellQuote("+obj.property("name").strValue()+") "+
                             "$$shellQuote("+obj.property("desc").strValue()+") "+
                             "$$shellQuote("+obj.property("domain").strValue()+") "+
                             "$$shellQuote("+obj.property("deps").strValue()+") "+
                             "$$shellQuote("+multi.join(" ")+") "+
                             "$$shellQuote("+obj.property("version").strValue()+") "+
                             "$$shellQuote("+obj.property("trust").strValue()+") "+
                             "$$shellQuote("+obj.property("license").strValue()+") "+
                             "$$shellQuote("+obj.property("maintainer").strValue()+") "+
                             "$$shellQuote("+obj.property("targets").strValue()+") "+
                             "$$shellQuote("+obj.property("quicklaunch").strValue()+") "+
                             "$$shellQuote($$(SPLIT_I18N))");

        var packages = packages_get_packages();
        packages.prerequisiteActions.append(rule.name);
    }

}

function packages_get_packages()
{
    var rule = project.rule("packages");
    if (!rule.help)
        rule.help = "Create packages";
    return rule;
}

