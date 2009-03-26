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

#ifndef QSOFTKEYLABELHELPER_H
#define QSOFTKEYLABELHELPER_H

#include <QObject>
#include <QWidget>
#include "qsoftkeylabelhelper_p.h"
#include <QByteArray>
#include <QLineEdit>
#include <QDateTimeEdit>
#include <QSoftMenuBar>
#include "contextkeymanager_p.h"

class QSoftKeyLabelHelperManager;
class QSoftKeyLabelHelperPrivate;

class QTOPIA_EXPORT QAbstractSoftKeyLabelHelper : public QObject
{
    Q_OBJECT
    friend class QSoftKeyLabelHelperPrivate;

public:
    QAbstractSoftKeyLabelHelper(QWidget* widget);
    QAbstractSoftKeyLabelHelper(const QString &className);
    virtual ~QAbstractSoftKeyLabelHelper();

    virtual bool focusIn(QWidget*) = 0;
    virtual bool focusOut(QWidget*);
    virtual bool enterEditFocus(QWidget*);
    virtual bool leaveEditFocus(QWidget*);

    virtual QString className() const;
    void setCurrentWidget(QWidget* widget);

public slots:
    virtual void updateAllLabels(QWidget* widget = 0) = 0;

protected:
    virtual void setClass(const QString& className);
    virtual void currentWidgetChangeNotification(QWidget* newWidget, QWidget* oldWidget);
    QWidget* currentWidget();

    // Label manipulation functions
    void setStandardLabel(int key,  QSoftMenuBar::StandardLabel);
    void setLabelText(int key, const QString &text);
    void setLabelPixmap(int key, const QString &pm);
    void clearLabel(int key);

private:
    QSoftKeyLabelHelperPrivate *d;
    QWidget* m_widget;
    QString m_className;
};

class QTOPIA_EXPORT QSoftKeyLabelHelper : public QAbstractSoftKeyLabelHelper
{
    Q_OBJECT
public:
    QSoftKeyLabelHelper(QWidget* widget);
    QSoftKeyLabelHelper(const QString &className);
    virtual ~QSoftKeyLabelHelper();
    virtual bool focusIn(QWidget*);
    virtual bool focusOut(QWidget*);
    virtual bool enterEditFocus(QWidget*);
    virtual bool leaveEditFocus(QWidget*);
    virtual void updateAllLabels(QWidget *widget);
private:
    QWidget* m_widget;
};

class QTOPIA_EXPORT QLineEditSoftKeyLabelHelper : public QAbstractSoftKeyLabelHelper
{
    Q_OBJECT
public:
    QLineEditSoftKeyLabelHelper(QString className="QLineEdit");
    QLineEditSoftKeyLabelHelper(QLineEdit* l);
    ~QLineEditSoftKeyLabelHelper();

    virtual bool focusIn(QWidget*);
    virtual bool focusOut(QWidget*);
    virtual bool enterEditFocus(QWidget*);
    virtual bool leaveEditFocus(QWidget*);

public slots:
    virtual void updateAllLabels(QWidget* widget=0);
    virtual void textChanged(QString newText);
    virtual void cursorPositionChanged(int old, int current);
protected:
    virtual void currentWidgetChangeNotification(QWidget* newWidget, QWidget* oldWidget);
    virtual bool eventFilter(QObject* watched, QEvent *event);
    QLineEdit* m_l;
private:
    bool m_preeditTextFlag;
};

class QTOPIA_EXPORT QDateTimeEditSoftKeyLabelHelper : public QAbstractSoftKeyLabelHelper
{
    Q_OBJECT
public:
    QDateTimeEditSoftKeyLabelHelper(QString className="QDateTimeEdit");
    QDateTimeEditSoftKeyLabelHelper(QDateTimeEdit* de);
    ~QDateTimeEditSoftKeyLabelHelper();

    virtual bool focusIn(QWidget*);
    virtual bool focusOut(QWidget*);
    virtual bool enterEditFocus(QWidget*);
    virtual bool leaveEditFocus(QWidget*);

public slots:
    virtual void updateAllLabels(QWidget* widget=0);
protected:
    virtual bool eventFilter(QObject* watched, QEvent *event);
    QDateTimeEdit* m_dte;
};
#endif
