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

#ifndef QTEXTENTRYPROXY_H
#define QTEXTENTRYPROXY_H

#include <qtopiaglobal.h>
#include <QWidget>

class QKeyEvent;
class QInputMethodEvent;

class QTextEntryProxyData;
class QTOPIAPIM_EXPORT QTextEntryProxy : public QWidget
{
    Q_OBJECT
public:
    QTextEntryProxy(QWidget *parent, QWidget *target);
    ~QTextEntryProxy();

    // geom hinting.... focus policy etc.
    QSize sizeHint() const;

    QString text() const;
    void clear();
    void setTarget ( QWidget * target );

signals:
    void textChanged(const QString &);
protected:
    void paintEvent(QPaintEvent *);

    // To Be Deprecated
    bool processInputMethodEvent(QInputMethodEvent *);
    bool processKeyPressEvent(QKeyEvent *);

    QVariant inputMethodQuery(Qt::InputMethodQuery) const;

    bool eventFilter(QObject *obj, QEvent *event);

private slots:
    void targetDestroyed(QObject *obj);

private:
    QTextEntryProxyData *d;
};

#endif
