/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the Qt3Support module of the Qt Toolkit.
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

#ifndef Q3SQLFORM_H
#define Q3SQLFORM_H

#include <QtCore/qobject.h>
#include <QtCore/qmap.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Qt3Support)

#ifndef QT_NO_SQL_FORM

class QSqlField;
class QSqlRecord;
class Q3SqlEditorFactory;
class Q3SqlPropertyMap;
class QWidget;
class Q3SqlFormPrivate;

class Q_COMPAT_EXPORT Q3SqlForm : public QObject
{
    Q_OBJECT
public:
    Q3SqlForm(QObject * parent = 0);
    ~Q3SqlForm();

    virtual void insert(QWidget * widget, const QString& field);
    virtual void remove(const QString& field);
    int         count() const;

    QWidget *   widget(int i) const;
    QSqlField * widgetToField(QWidget * widget) const;
    QWidget *   fieldToWidget(QSqlField * field) const;

    void        installPropertyMap(Q3SqlPropertyMap * map);

    virtual void setRecord(QSqlRecord* buf);

public Q_SLOTS:
    virtual void readField(QWidget * widget);
    virtual void writeField(QWidget * widget);
    virtual void readFields();
    virtual void writeFields();

    virtual void clear();
    virtual void clearValues();

protected:
    virtual void insert(QWidget * widget, QSqlField * field);
    virtual void remove(QWidget * widget);
    void clearMap();

private:
    Q_DISABLE_COPY(Q3SqlForm)

    virtual void sync();
    Q3SqlFormPrivate* d;
};

#endif // QT_NO_SQL_FORM

QT_END_NAMESPACE

QT_END_HEADER

#endif // Q3SQLFORM_H
