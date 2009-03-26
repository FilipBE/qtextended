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

#include "qtbrushmanager.h"
#include <QtGui/QPixmap>
#include <QtGui/QPainter>

QT_BEGIN_NAMESPACE

namespace qdesigner_internal {

class QtBrushManagerPrivate
{
    QtBrushManager *q_ptr;
    Q_DECLARE_PUBLIC(QtBrushManager)
public:
    QMap<QString, QBrush> theBrushMap;
    QString theCurrentBrush;
};

QtBrushManager::QtBrushManager(QObject *parent)
    : QDesignerBrushManagerInterface(parent)
{
    d_ptr = new QtBrushManagerPrivate;
    d_ptr->q_ptr = this;

}

QtBrushManager::~QtBrushManager()
{
    delete d_ptr;
}

QBrush QtBrushManager::brush(const QString &name) const
{
    if (d_ptr->theBrushMap.contains(name))
        return d_ptr->theBrushMap[name];
    return QBrush();
}

QMap<QString, QBrush> QtBrushManager::brushes() const
{
    return d_ptr->theBrushMap;
}

QString QtBrushManager::currentBrush() const
{
    return d_ptr->theCurrentBrush;
}

QString QtBrushManager::addBrush(const QString &name, const QBrush &brush)
{
    if (name.isNull())
        return QString();

    QString newName = name;
    QString nameBase = newName;
    int i = 0;
    while (d_ptr->theBrushMap.contains(newName)) {
        newName = nameBase + QString::number(++i);
    }
    d_ptr->theBrushMap[newName] = brush;
    emit brushAdded(newName, brush);

    return newName;
}

void QtBrushManager::removeBrush(const QString &name)
{
    if (!d_ptr->theBrushMap.contains(name))
        return;
    if (currentBrush() == name)
        setCurrentBrush(QString());
    emit brushRemoved(name);
    d_ptr->theBrushMap.remove(name);
}

void QtBrushManager::setCurrentBrush(const QString &name)
{
    QBrush newBrush;
    if (!name.isNull()) {
        if (d_ptr->theBrushMap.contains(name))
            newBrush = d_ptr->theBrushMap[name];
        else
            return;
    }
    d_ptr->theCurrentBrush = name;
    emit currentBrushChanged(name, newBrush);
}

QPixmap QtBrushManager::brushPixmap(const QBrush &brush) const
{
    int w = 64;
    int h = 64;

    QImage img(w, h, QImage::Format_ARGB32_Premultiplied);
    QPainter p(&img);
    p.setCompositionMode(QPainter::CompositionMode_Source);
    p.fillRect(QRect(0, 0, w, h), brush);
    return QPixmap::fromImage(img);
}

}  // namespace qdesigner_internal

QT_END_NAMESPACE
