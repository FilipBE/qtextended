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

#ifndef SERVERTHEMEVIEW_H
#define SERVERTHEMEVIEW_H

#include <themedview.h>
#include <QWidget>
#include <QSet>

class PhoneThemedView : public ThemedView {
Q_OBJECT
public:
    PhoneThemedView(QWidget *parent=0, Qt::WFlags f=0);
    virtual ~PhoneThemedView();

    static QSet<PhoneThemedView *> themedViews();
    QWidget *newWidget(ThemeWidgetItem *, const QString &);

private slots:
    void myItemClicked(ThemeItem *item);

private:
    static QSet<PhoneThemedView *> m_themedViews;
};

#endif
