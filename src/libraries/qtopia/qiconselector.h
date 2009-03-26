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

#ifndef QICONSELECTOR_H
#define QICONSELECTOR_H

#include <qtopiaglobal.h>
#include <QToolButton>

class QIconSelectorPrivate;
class QTOPIA_EXPORT QIconSelector : public QToolButton
{
    Q_OBJECT
public:
    explicit QIconSelector( QWidget *parent = 0 );
    explicit QIconSelector( const QIcon &icn, QWidget *parent = 0 );
    ~QIconSelector();

    uint count() const;

    void insertItem( const QIcon &icn, const QString &text = QString() );
    void removeIndex( int index );
    void clear();
    int currentIndex() const;

    QIcon icon() const;
    void setIcon( const QIcon &icn );
    QSize sizeHint() const;

    QString text() const;

signals:
    void activated(int);

public slots:
    void setCurrentIndex( int index );

protected slots:
    void popup();
    void popdown();
private slots:
    void itemChanged(int index);
    void workAreaResized();

protected:
    void keyPressEvent( QKeyEvent *e );
    void itemSelected( int index );
    virtual bool eventFilter( QObject *obj, QEvent *e );

private:
    void init();
    QIconSelectorPrivate *d;
};

#endif
