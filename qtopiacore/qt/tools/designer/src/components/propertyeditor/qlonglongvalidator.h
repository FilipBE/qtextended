/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the Qt Designer of the Qt Toolkit.
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

#ifndef QLONGLONGVALIDATOR_H
#define QLONGLONGVALIDATOR_H

#include <QtGui/QValidator>

QT_BEGIN_NAMESPACE

namespace qdesigner_internal {

class QLongLongValidator : public QValidator
{
    Q_OBJECT
    Q_PROPERTY(qlonglong bottom READ bottom WRITE setBottom)
    Q_PROPERTY(qlonglong top READ top WRITE setTop)

public:
    explicit QLongLongValidator(QObject * parent);
    QLongLongValidator(qlonglong bottom, qlonglong top, QObject * parent);
    ~QLongLongValidator();

    QValidator::State validate(QString &, int &) const;

    void setBottom(qlonglong);
    void setTop(qlonglong);
    virtual void setRange(qlonglong bottom, qlonglong top);

    qlonglong bottom() const { return b; }
    qlonglong top() const { return t; }

private:
    Q_DISABLE_COPY(QLongLongValidator)

    qlonglong b;
    qlonglong t;
};

// ----------------------------------------------------------------------------
class QULongLongValidator : public QValidator
{
    Q_OBJECT
    Q_PROPERTY(qulonglong bottom READ bottom WRITE setBottom)
    Q_PROPERTY(qulonglong top READ top WRITE setTop)

public:
    explicit QULongLongValidator(QObject * parent);
    QULongLongValidator(qulonglong bottom, qulonglong top, QObject * parent);
    ~QULongLongValidator();

    QValidator::State validate(QString &, int &) const;

    void setBottom(qulonglong);
    void setTop(qulonglong);
    virtual void setRange(qulonglong bottom, qulonglong top);

    qulonglong bottom() const { return b; }
    qulonglong top() const { return t; }

private:
    Q_DISABLE_COPY(QULongLongValidator)

    qulonglong b;
    qulonglong t;
};

}  // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // QLONGLONGVALIDATOR_H
