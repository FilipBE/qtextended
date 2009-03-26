/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the tools applications of the Qt Toolkit.
**
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License versions 2.0 or 3.0 as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file.  Please review the following information
** to ensure GNU General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.  In addition, as a special
** exception, Nokia gives you certain additional rights. These rights
** are described in the Nokia Qt GPL Exception version 1.3, included in
** the file GPL_EXCEPTION.txt in this package.
**
** Qt for Windows(R) Licensees
** As a special exception, Nokia, as the sole copyright holder for Qt
** Designer, grants users of the Qt/Eclipse Integration plug-in the
** right for the Qt/Eclipse Integration to link to functionality
** provided by Qt Designer and its related libraries.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
****************************************************************************/

#ifndef DRIVER_H
#define DRIVER_H

#include "option.h"
#include <QtCore/QHash>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QTextStream>

QT_BEGIN_NAMESPACE

class QTextStream;
class DomUI;
class DomWidget;
class DomSpacer;
class DomLayout;
class DomLayoutItem;
class DomActionGroup;
class DomAction;

class Driver
{
public:
    Driver();
    virtual ~Driver();

    // tools
    bool printDependencies(const QString &fileName);
    bool uic(const QString &fileName, QTextStream *output = 0);
    bool uic(const QString &fileName, DomUI *ui, QTextStream *output = 0);

    // configuration
    inline QTextStream &output() const { return *m_output; }
    inline Option &option() { return m_option; }

    // initialization
    void reset();

    // error
    inline QStringList problems() { return m_problems; }
    inline void addProblem(const QString &problem) { m_problems.append(problem); }

    // utils
    static QString headerFileName(const QString &fileName);
    QString headerFileName() const;

    static QString normalizedName(const QString &name);
    static QString qtify(const QString &name);
    QString unique(const QString &instanceName=QString(),
                   const QString &className=QString());

    // symbol table
    QString findOrInsertWidget(DomWidget *ui_widget);
    QString findOrInsertSpacer(DomSpacer *ui_spacer);
    QString findOrInsertLayout(DomLayout *ui_layout);
    QString findOrInsertLayoutItem(DomLayoutItem *ui_layoutItem);
    QString findOrInsertName(const QString &name);
    QString findOrInsertActionGroup(DomActionGroup *ui_group);
    QString findOrInsertAction(DomAction *ui_action);

    inline bool hasName(const QString &name) const
    { return m_nameRepository.contains(name); }

    DomWidget *widgetByName(const QString &name) const;
    DomSpacer *spacerByName(const QString &name) const;
    DomLayout *layoutByName(const QString &name) const;
    DomActionGroup *actionGroupByName(const QString &name) const;
    DomAction *actionByName(const QString &name) const;

    // pixmap
    void insertPixmap(const QString &pixmap);
    bool containsPixmap(const QString &pixmap) const;

private:
    Option m_option;
    QTextStream m_stdout;
    QTextStream *m_output;

    QStringList m_problems;

    // symbol tables
    QHash<DomWidget*, QString> m_widgets;
    QHash<DomSpacer*, QString> m_spacers;
    QHash<DomLayout*, QString> m_layouts;
    QHash<DomActionGroup*, QString> m_actionGroups;
    QHash<DomAction*, QString> m_actions;
    QHash<QString, bool> m_nameRepository;
    QHash<QString, bool> m_pixmaps;
};

QT_END_NAMESPACE

#endif // DRIVER_H
