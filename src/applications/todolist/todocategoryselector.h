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
#ifndef TODOCATEGORYSELECTOR_H
#define TODOCATEGORYSELECTOR_H

#include "qtopia/qcategoryselector.h"

/* Todo List wide category scope variable */
static const char * TodoCategoryScope = "Todo List"; // No tr

/* Simple class used to set the scope of our QCategorySelectors when we create them
   from .ui files.  Also see the datebook/datebookcategoryselector.h file.

   This could go away if we could set the scope of a QCategorySelector after
   we create it, but currently that is a large change.
 */

class TodoCategorySelector : public QCategorySelector
{
    public:
        TodoCategorySelector(QWidget *parent = 0)
            : QCategorySelector(TodoCategoryScope, QCategorySelector::Editor | QCategorySelector::DialogView, parent)
            {
            }

};

#endif

