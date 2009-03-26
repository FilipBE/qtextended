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

/* Internal configuration for QtUiTest. */

/* Preprocessor configuration. */
preprocess = {
    /* A list of mappings from function names, to strings which should
     * be appended after the function. */
    functionAppends: {}
}

/* A list of files containing "builtin" functions to be made accessible
 * to all tests. */
builtin_files = [ "builtins.js" ];

/* code_setters: a list of special functions which have syntax like:
 *   myFunction(a,b,c) {
 *     some code;
 *   }
 * This is not valid ecmascript.  In reality, the code expands to:
 *   myFunction,a,b,c).code = function() {
 *     some code;
 *   }
 * Where a setter is defined for property "code" which makes the magic
 * happen.  The functions are implemented in builtins.js.
 */
code_setters = [
    "waitFor",
    "expectMessageBox",
    "expect"
]
for (var i = 0; i < code_setters.length; ++i)
    preprocess.functionAppends[code_setters[i]] = ".code = function() ";

