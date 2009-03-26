/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the tools applications of the Qt Toolkit.
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

#ifndef GRADIENTMANAGER_H
#define GRADIENTMANAGER_H

#include <QtCore/QObject>
#include <QtCore/QMap>
#include <QtCore/QSize>
#include <QtXml/QDomDocument>
#include <QtXml/QDomElement>
#include <QtGui/QGradient>

QT_BEGIN_NAMESPACE

class QGradient;
class QPixmap;
class QColor;

class QtGradientManager : public QObject
{
    Q_OBJECT
public:
    QtGradientManager(QObject *parent = 0);

    QMap<QString, QGradient> gradients() const;

    QString uniqueId(const QString &id) const;

public slots:

    QString addGradient(const QString &id, const QGradient &gradient);
    void renameGradient(const QString &id, const QString &newId);
    void changeGradient(const QString &id, const QGradient &newGradient);
    void removeGradient(const QString &id);

    //utils
    void clear();

signals:

    void gradientAdded(const QString &id, const QGradient &gradient);
    void gradientRenamed(const QString &id, const QString &newId);
    void gradientChanged(const QString &id, const QGradient &newGradient);
    void gradientRemoved(const QString &id);

private:

    QMap<QString, QGradient> m_idToGradient;
};

QT_END_NAMESPACE

#endif
