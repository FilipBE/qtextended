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

#ifndef QHELPENGINE_P_H
#define QHELPENGINE_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API. It exists for the convenience
// of the help generator tools. This header file may change from version
// to version without notice, or even be removed.
//
// We mean it.
//

#include <QtCore/QMap>
#include <QtCore/QStringList>
#include <QtCore/QObject>

QT_BEGIN_NAMESPACE

class QSqlQuery;

class QHelpEngineCore;
class QHelpDBReader;
class QHelpContentModel;
class QHelpContentWidget;
class QHelpIndexModel;
class QHelpIndexWidget;
class QHelpSearchEngine;
class QHelpCollectionHandler;

class QHelpEngineCorePrivate : public QObject
{
    Q_OBJECT

public:
    QHelpEngineCorePrivate();
    virtual ~QHelpEngineCorePrivate();

    virtual void init(const QString &collectionFile,
        QHelpEngineCore *helpEngineCore);

    void clearMaps();
    bool setup();

    QMap<QString, QHelpDBReader*> readerMap;
    QMap<QString, QHelpDBReader*> fileNameReaderMap;
    QMultiMap<QString, QHelpDBReader*> virtualFolderMap;
    QStringList orderedFileNameList;

    QHelpCollectionHandler *collectionHandler;
    QString currentFilter;
    QString error;
    bool needsSetup;

protected:
    QHelpEngineCore *q;

private slots:
    void errorReceived(const QString &msg);
};


class QHelpEnginePrivate : public QHelpEngineCorePrivate
{
    Q_OBJECT

public:
    QHelpEnginePrivate();
    ~QHelpEnginePrivate();

    void init(const QString &collectionFile, 
        QHelpEngineCore *helpEngineCore);   

    QHelpContentModel *contentModel;
    QHelpContentWidget *contentWidget;

    QHelpIndexModel *indexModel;
    QHelpIndexWidget *indexWidget;

    QHelpSearchEngine *searchEngine;

    void stopDataCollection();

    friend class QHelpContentProvider;
    friend class QHelpContentModel;
    friend class QHelpIndexProvider;
    friend class QHelpIndexModel;

public slots:
    void setContentsWidgetBusy();
    void unsetContentsWidgetBusy();
    void setIndexWidgetBusy();
    void unsetIndexWidgetBusy();

private slots:
    void applyCurrentFilter();
};

QT_END_NAMESPACE

#endif
