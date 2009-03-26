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

#ifndef QSPEEDDIAL_H
#define QSPEEDDIAL_H

#include <qtopiaglobal.h>
#include "qfavoriteservicesmodel.h"
#include <QtopiaServiceDescription>
#include <QSmoothList>
#include <QList>

class QSpeedDial;
class QSpeedDialList;
class QSpeedDialListPrivate;


class QTOPIA_EXPORT QSpeedDialList : public QSmoothList
{
    Q_OBJECT
    Q_PROPERTY(int count READ count)
    Q_PROPERTY(int currentRow READ currentRow WRITE setCurrentRow)
    Q_PROPERTY(QString currentInput READ currentInput)
    friend class QSpeedDialAddDialog;

public:
    explicit QSpeedDialList(QWidget* parent=0);
    ~QSpeedDialList();

    QString currentInput() const;
    QString rowInput(int row) const;

    void setCurrentInput ( const QString & input );
    void setCurrentRow(int row);
    int currentRow() const;

    int count() const;

public slots:
    void addItem();
    void editItem(int row);
    void editItem();
    void clearItem(int row);
    void clearItem();
    void reload ( const QString & input );

signals:
    void currentRowChanged(int row);
    void rowClicked(int row);

protected:
    void scrollContentsBy(int dx, int dy);
    void keyPressEvent(QKeyEvent* e);
    void disableEditMenu();
    void timerEvent(QTimerEvent*);
    void setSelector(int startPos, const QString &label, const QString &icon);
    QString selectorInput();

private slots:
    void select(const QModelIndex& index);
    void click(const QModelIndex& index);
    void sendRowChanged();
    void clearKeyInput();

private:
    QSpeedDialListPrivate *d;
};



class QTOPIA_EXPORT QSpeedDial
{
public:
    static QString addWithDialog(const QString& label, const QString& icon,
        const QtopiaServiceRequest& action, QWidget* parent);
    static QString selectWithDialog(QWidget* parent);
    static QList<QString> assignedInputs();
    static QList<QString> possibleInputs();
    static QtopiaServiceDescription* find(const QString& input);
    static void remove(const QString& input);
    static void set(const QString& input, const QtopiaServiceDescription&);
protected:
    static int firstAvailableSlot();

};

#endif
