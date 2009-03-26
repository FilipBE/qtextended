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

void cleanOutTodolist1()
{
    QVERIFY(cleanOutTable());
    QVERIFY(checkForEmptyTable());
}

void addClientRecord1()
{
    QByteArray newRecord =
"<Task>\n"
"<Identifier localIdentifier=\"false\">Client Identifier</Identifier>\n"
"<Description>Go Home</Description>\n"
"<Priority>VeryHigh</Priority>\n"
"<Status>NotStarted</Status>\n"
"<DueDate>2007-04-27</DueDate>\n"
"<PercentCompleted>0</PercentCompleted>\n"
"<Categories>\n"
"<Category>business</Category>\n"
"<Category>personal</Category>\n"
"</Categories>\n"
"</Task>\n";
#ifdef Q_WS_QWS // Qtopia can't add categories to an item in the unittest
                // unless the category already exists in the database.
    { QCategoryManager().add("personal"); }
    { QCategoryManager().add("business"); }
#endif
    QVERIFY(addClientRecord( newRecord ));
}

void checkForAddedItem1()
{
    QByteArray expected =
"<Task>\n"
"<Identifier>Client Identifier</Identifier>\n"
"<Description>Go Home</Description>\n"
"<Priority>VeryHigh</Priority>\n"
"<Status>NotStarted</Status>\n"
"<DueDate>2007-04-27</DueDate>\n"
"<StartedDate></StartedDate>\n"
"<CompletedDate></CompletedDate>\n"
"<PercentCompleted>0</PercentCompleted>\n"
"<Notes></Notes>\n"
"<Categories>\n"
#ifdef Q_WS_QWS // Qtopia returns these reversed
"<Category>personal</Category>\n"
"<Category>business</Category>\n"
#else // Outlook returns these in the order we gave them
"<Category>business</Category>\n"
"<Category>personal</Category>\n"
#endif
"</Categories>\n"
#ifdef Q_WS_QWS
"<CustomFields>\n"
"</CustomFields>\n"
#endif
"</Task>\n";
    QVERIFY(checkForAddedItem( expected ));
}

