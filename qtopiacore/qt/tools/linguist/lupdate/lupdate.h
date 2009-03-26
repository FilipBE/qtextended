/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the Qt Linguist of the Qt Toolkit.
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

#ifndef QT_LUPDATE_H
#define QT_LUPDATE_H

#include "metatranslator.h"
#include "qconsole.h"

#include <QCoreApplication>
#include <QString>
#include <QDir>

QT_BEGIN_NAMESPACE

class QByteArray;
class QString;
class QStringList;

class LupdateApplication : public QCoreApplication
{
    Q_OBJECT
public:
    LupdateApplication(int &argc, char **argv);

    int start();

    // defined in fetchtr.cpp
    void fetchtr_cpp( const QString &fileName, MetaTranslator *tor,
                      const QString &defaultContext, bool mustExist,
                      const QByteArray &codecForSource );
    void fetchtr_ui( const QString &fileName, MetaTranslator *tor,
                     const QString &defaultContext, bool mustExist );

    // defined in fetchtrjava.cpp
    void fetchtr_java( const QString &fileName, MetaTranslator *tor,
                       const QString &defaultContext, bool mustExist,
                       const QByteArray &codecForSource );
    // defined in merge.cpp
    void merge( const MetaTranslator *tor, const MetaTranslator *virginTor,
                MetaTranslator *out, bool verbose, bool noObsolete );

    void recursiveFileInfoList( const QDir &dir, const QStringList &nameFilters,
                                QDir::Filters filter, bool recursive, QFileInfoList *fileinfolist);
    void printUsage();
    void updateTsFiles( const MetaTranslator& fetchedTor,
                               const QStringList& tsFileNames, const QString& codecForTr,
                               bool noObsolete, bool pluralOnly, bool verbose );

private:
    QString m_defaultExtensions;
};

QT_END_NAMESPACE

#endif // QT_LUPDATE_H
