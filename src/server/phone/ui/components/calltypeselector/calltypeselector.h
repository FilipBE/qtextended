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

#ifndef CALLTYPESELECTOR_H
#define CALLTYPESELECTOR_H

#include <QDialog>
#include <QList>

class CallTypeSelectorPrivate;
class CallTypeSelector : public QDialog
{
    Q_OBJECT
public:
    CallTypeSelector( QWidget *parent = 0, Qt::WFlags fl = 0 );
    ~CallTypeSelector();

    Q_INVOKABLE void setAvailablePolicyManagers( const QStringList& managers );

    Q_INVOKABLE QString selectedPolicyManager() const;

private slots:
    void itemActivated();

private:
    CallTypeSelectorPrivate *d;
};

#endif
