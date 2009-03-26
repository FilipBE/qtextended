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

#ifndef PROFILEEVALUATOR_H
#define PROFILEEVALUATOR_H
#include "proitems.h"
#include "abstractproitemvisitor.h"
#include <QtCore/QIODevice>
#include <QtCore/QMap>
#include <QtCore/QStringList>
#include <QtCore/QStack>

QT_BEGIN_NAMESPACE

class ProFile;

class ProFileEvaluator : public AbstractProItemVisitor {
public:
    typedef enum {
        TT_Unknown = 0,
        TT_Application,
        TT_Library,
        TT_Subdirs
    } TemplateType;

    typedef enum {
        MT_Error            = 1,    // Catastrophic error, processing stops
        MT_DebugLevel1,             // serious errors
        MT_DebugLevel2,             // default
        MT_DebugLevel3,             // less serious
        MT_ProMessage       = 100,  // output of the profiles' message(string).
        MT_ProError,                // output of error(string). In this implementation, processing will not stop.
    } MessageType;

    struct LogMessage {
        QString m_msg;
        QString m_filename;
        int m_linenumber;
        MessageType m_type;
    };

    ProFileEvaluator();
    ~ProFileEvaluator();

    /*
     * INHERITED from AbstractProItemVisitor
     */
    bool visitBeginProBlock(ProBlock * block);
    bool visitEndProBlock(ProBlock * block);
    bool visitBeginProVariable(ProVariable *variable);
    bool visitEndProVariable(ProVariable * /*variable*/);
    bool visitBeginProFile(ProFile * value);
    bool visitEndProFile(ProFile * value);
    bool visitProValue(ProValue *value);
    bool visitProFunction(ProFunction *function);
    bool visitProOperator(ProOperator * oper);
    bool visitProCondition(ProCondition * cond);

    ProFileEvaluator::TemplateType templateType();
    bool contains(const QString &variableName) const;
    QStringList values(const QString &variableName) const;
    QStringList absFileNames(const QString &variableName);
    QStringList absFileName(const QString &name);

protected:
    virtual ProFile *queryProFile(const QString &filename);
    virtual void releaseProFile(ProFile *pro);
    virtual QString propertyValue(const QString &val) const;

    virtual void logMessage(const LogMessage &msg);

private:
    QStringList expandPattern(const QString &pattern);
    void logMessage(const QString &msg, MessageType mt = MT_DebugLevel2);
    void logMessage(MessageType mt, const char *msg, ...);
    QStringList expandVariableReferences(const QString &value);
    QStringList evaluateExpandFunction(const QByteArray &func, const QString &arguments);

    bool evaluateConditionalFunction(const QByteArray &function, const QString &arguments, bool *result);
    bool evaluateFile(const QString &fileName, bool *result);
    bool evaluateFeatureFile(const QString &fileName, bool *result);

    bool isActiveConfig(const QByteArray &config, bool regex = false);

    QString currentFileName() const;
    QString getcwd() const;
    ProFile *currentProFile() const;

    QStringList qmake_feature_paths();
    QByteArray m_lastVarName;
    ProVariable::VariableOperator m_variableOperator;

    QMap<QByteArray, QStringList> m_valuemap;       // VariableName must be us-ascii, the content however can be non-us-ascii.
    QStack<ProFile*> m_profileStack;                // To handle 'include(a.pri), so we can track back to 'a.pro' when finished with 'a.pri'
    int m_lineNo;                                   // Error reporting
    QString m_oldPath;                              // To restore the current path to the path
    QString m_origfile;

    bool m_condition;
    bool m_invertNext;

}; //class ProFileEvaluator


QT_END_NAMESPACE
#endif // PROFILEEVALUATOR_H

