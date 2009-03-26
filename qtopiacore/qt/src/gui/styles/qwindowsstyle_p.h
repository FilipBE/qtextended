/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the QtGui module of the Qt Toolkit.
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

#ifndef QWINDOWSSTYLE_P_H
#define QWINDOWSSTYLE_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of qapplication_*.cpp, qwidget*.cpp and qfiledialog.cpp.  This header
// file may change from version to version without notice, or even be removed.
//
// We mean it.
//

#include "qwindowsstyle.h"
#include "qcommonstyle_p.h"

#ifndef QT_NO_STYLE_WINDOWS
#include <qlist.h>
#include <qdatetime.h>
#include <qhash.h>

QT_BEGIN_NAMESPACE

class QStringList;
class QTime;
class QProgressBar;

class QIconTheme
{

public:
    QIconTheme(QHash <int, QString> dirList, QStringList parents) :
          _dirList(dirList), _parents(parents), _valid(true){ }
    QIconTheme() : _valid(false){ }

    QHash <int, QString> dirList() {return _dirList;}
    QStringList parents() {return _parents;}
    bool isValid() {return _valid;}

private:
    QHash <int, QString> _dirList;
    QStringList _parents;
    bool _valid;
};

class QWindowsStylePrivate : public QCommonStylePrivate
{
    Q_DECLARE_PUBLIC(QWindowsStyle)
public:
    QWindowsStylePrivate();
    bool hasSeenAlt(const QWidget *widget) const;
    bool altDown() const { return alt_down; }
    bool alt_down;
    QList<const QWidget *> seenAlt;
    int menuBarTimer;

    QList<QProgressBar *> bars;
    int animationFps;
    int animateTimer;
    QTime startTime;
    int animateStep;    
    QColor inactiveCaptionText;
    QColor activeCaptionColor;
    QColor activeGradientCaptionColor;
    QColor inactiveCaptionColor;
    QColor inactiveGradientCaptionColor;
    
    //icon detection on X11
    QPixmap findIcon(int size, const QString &) const;
#ifdef Q_WS_X11
    QPixmap findIconHelper(int size, const QString &, const QString &, QStringList &visited) const;
    QIconTheme parseIndexFile(const QString &themeName) const;
    mutable QString themeName;
    QStringList iconDirs;
    mutable QHash <QString, QIconTheme> themeList;
#endif
};

QT_END_NAMESPACE

#endif // QT_NO_STYLE_WINDOWS

#endif //QWINDOWSSTYLE_P_H
