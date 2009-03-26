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

#ifndef E1_HEADER_H
#define E1_HEADER_H

#include <QWidget>
#include <QStringList>
#include <qvaluespace.h>
#include <QMap>

class E1HeaderAlertButton;
class E1Header : public QWidget
{
    Q_OBJECT
public:
    E1Header(QWidget * = 0, Qt::WFlags = 0);

protected:
    virtual QSize sizeHint() const;
    virtual void paintEvent(QPaintEvent *);

private slots:
    void setAlertEnabled( bool e );
    void alertClicked();
    void clicked(const QString &);
    void missedCallsChanged();
    void newMessagesChanged();

private:
    QValueSpaceItem m_missedCallsVS;
    QValueSpaceItem m_newMessagesVS;
    QStringList m_alertStack;
    E1HeaderAlertButton* m_alertButton;
};

#endif
