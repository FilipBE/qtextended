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

//
//  W A R N I N G
//  -------------
//
// This file is part of QtUiTest and is released as a Technology Preview.
// This file and/or the complete System testing solution may change from version to
// version without notice, or even be removed.
//

#ifndef QTSCRIPT_BINDINGS_H
#define QTSCRIPT_BINDINGS_H

namespace QtScript {
    void getLocation(QScriptContext*,QString*,int*);
    void addInternalFile(QString const&);
};

class QtScriptTest : public QObject
{

public:
    QtScriptTest(QString const &testFilePath = QString(), QString const &scriptData = QString(), QScriptEngine *engine = 0);
    virtual ~QtScriptTest();

    static QMetaObject staticMetaObject;
    virtual const QMetaObject *metaObject() const;
    virtual void *qt_metacast(const char *);
    virtual int qt_metacall(QMetaObject::Call, int, void **argv);

    QString testFilePath() const { return m_testFilePath; }

    QScriptEngine* engine() { return m_engine; }

private:
    QString        m_testFilePath;
    QScriptEngine* m_engine;
};

#endif

