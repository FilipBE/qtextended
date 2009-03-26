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
#ifndef FIELDLIST_H
#define FIELDLIST_H

#include <QContact>
#include <QLineEdit>
#include <QIconSelector>
#include "qfielddefinition.h"

// written as if the api mattered more than the mechanics.
class QContactFieldListData;
class QFieldAction;
class QContactFieldList : public QWidget
{
    Q_OBJECT
public:
    QContactFieldList(QWidget *parent = 0);
    ~QContactFieldList();

    void setAllowedFields(const QStringList &);
    void setCommonFields(const QStringList &);
    void setSuggestedFields(const QStringList &);

    QStringList allowedFields() const;
    QStringList commonFields() const;
    QStringList suggestedFields() const;

    bool isEmpty() const;

    void setEntry(const QContact &, bool newEntry = false);
    QContact updateEntry(const QContact &) const;

    QStringList selectableFields() const;

    QString field(const QString &) const;
signals:
    void fieldChanged(const QString &, const QString &);

    void fieldActivated(const QString &, const QString &);

public slots:
    void setField(const QString &, const QString &value);

private slots:
    void maintainVisibleEdits();
    void changeEditField(const QString &);
    void removeCurrentEdit();

    void updateContextMenu();

    void forwardFieldAction();

private:
    void clearEdits();
    void addEdit(const QString &, const QString &);
    void removeEdit(int index);
    void addEmptyEdit();

    void updateSelectableFields();

private:

    QContactFieldListData *d;
};

// needs a companion line edit for the other tabs.

class FieldLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    FieldLineEdit(const QString &, QWidget *parent = 0);
    ~FieldLineEdit();

    QString field() const;
    void setField(const QString &);

signals:
    void textChanged(const QString &, const QString &value);
    void textEdited(const QString &, const QString &value);

public slots:
    void updateText(const QString &, const QString &value);

private slots:
    void q_textChanged(const QString &value);
    void q_textEdited(const QString &value);

private:
    QString mField;
};

class FieldIconSelector : public QIconSelector
{
    Q_OBJECT
public:
    FieldIconSelector(QWidget *parent = 0);
    ~FieldIconSelector();

    void clear();

    QStringList fields()  const;
    void setFields(const QStringList fields);

    void addField(const QString &);
    bool contains(const QString &) const;

    QString currentField() const;
    void setCurrentField(const QString &);

signals:
    void fieldActivated(const QString &);
private slots:
    void q_fieldActivated(int);

private:
    QStringList mFields;
};

#endif
