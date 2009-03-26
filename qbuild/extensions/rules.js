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

\extension rules

This extension allows you to create rules using QMakeScript.
*/

function rules_init()
{
###
    QMAKE.FINALIZE.rules.CALL = rules_finalize
###
}

function rules_finalize()
{
    var rules = project.find("TYPE", "RULE");

    for (var ii = 0; ii < rules.length; ++ii)
        rules_make_rule(rules[ii]);
}

function rules_make_rule(object)
{
    var rule_obj = qbuild.object(object);
    var rule_name = rule_obj.name;

    var rule = project.rule(rule_name);

    if (rule_obj.isProperty("help"))
        rule.help = rule_obj.property("help").strValue();
    if (rule_obj.isProperty("inputFiles"))
        rule.inputFiles = rule_obj.property("inputFiles").value();
    if (rule_obj.isProperty("outputFiles"))
        rule.outputFiles = rule_obj.property("outputFiles").value();
    if (rule_obj.isProperty("prerequisiteActions"))
        rule.prerequisiteActions.append(rule_obj.property("prerequisiteActions").value());
    if (rule_obj.isProperty("commands"))
        rule.commands = rule_obj.property("commands").value();
    if (rule_obj.isProperty("other"))
        rule.other = rule_obj.property("other").value();
    if (rule_obj.isProperty("tests"))
        rule.tests = rule_obj.property("tests").value();
    if (rule_obj.isProperty("serial"))
        rule.serial = true;
    if (rule_obj.isProperty("category"))
        rule.category = rule_obj.property("category").value()
}

