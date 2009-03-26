/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the Qt Assistant of the Qt Toolkit.
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

#ifndef QHELP_GLOBAL_H
#define QHELP_GLOBAL_H

#include <QtCore/qglobal.h>
#include <QtCore/QString>
#include <QtCore/QObject>
#include <QtCore/QRegExp>
#include <QtCore/QMutexLocker>
#include <QtGui/QTextDocument>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Help)

#if !defined(QT_SHARED) && !defined(QT_DLL)
#   define QHELP_EXPORT
#elif defined(QHELP_LIB)
#   define QHELP_EXPORT Q_DECL_EXPORT
#else
#   define QHELP_EXPORT Q_DECL_IMPORT
#endif

class QHelpGlobal {
public:
    static QString uniquifyConnectionName(const QString &name, void *pointer)
    {
        static int counter = 0;
        static QMutex mutex;

        QMutexLocker locker(&mutex);
        if (++counter > 1000)
            counter = 0;
        
        return QString::fromLatin1("%1-%2-%3")
            .arg(name).arg(long(pointer)).arg(counter);
    };

    static QString documentTitle(const QString &content)
    {
        QString title = QObject::tr("Untitled");
        if (!content.isEmpty()) {
            int start = content.indexOf(QLatin1String("<title>"), 0, Qt::CaseInsensitive) + 7;
            int end = content.indexOf(QLatin1String("</title>"), 0, Qt::CaseInsensitive);
            if ((end - start) > 0) {
                title = content.mid(start, end - start);
                if (Qt::mightBeRichText(title) || title.contains(QLatin1Char('&'))) {
                    QTextDocument doc;
                    doc.setHtml(title);
                    title = doc.toPlainText();
                }
            }
        }
        return title;
    };

    static QString charsetFromData(const QByteArray &data)
    {
        QString encoding;
        int start = data.indexOf("<meta", 0);
        if (start > 0) {
            int end = data.indexOf('>', start);
            QString meta = QString::fromLatin1(data.mid(start +5, end - start));
            meta = meta.toLower();
            QRegExp r(QLatin1String("charset=([^\"\\s]+)"));
            if (r.indexIn(meta) != -1) {
                encoding = r.cap(1);
            }
        }

        if (encoding.isEmpty())
            return QLatin1String("utf-8");

        return encoding;
    }
};

QT_END_NAMESPACE

QT_END_HEADER

#endif // QHELP_GLOBAL_H
