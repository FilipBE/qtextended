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

#ifndef FEATURE_H
#define FEATURE_H

#include <QString>
#include <QStringList>
#include <QMap>
#include <QSet>
#include <QList>

QT_BEGIN_NAMESPACE

class Feature;

class FeaturePrivate
{
public:
    FeaturePrivate(const QString &k)
	: key(k), enabled(true), selectable(true) {};
    
    const QString key;
    QString section;
    QString title;
    QString description;
    QSet<Feature*> dependencies;
    QSet<Feature*> supports; // features who depends on this one
    QSet<Feature*> relations;
    bool enabled;
    bool selectable;
};

class Feature : public QObject
{
    Q_OBJECT    

public:
    static Feature* getInstance(const QString &key);
    static void clear();

public:
    QString key() const { return d->key; }

    void setTitle(const QString &title);
    QString title() const { return d->title; }

    void setSection(const QString &section);
    QString section() const { return d->section; }

    void setDescription(const QString &description);    
    QString description() const { return d->description; };

    void addRelation(const QString &key);
    void setRelations(const QStringList &keys);
    QList<Feature*> relations() const;    

    void addDependency(const QString &dependency);
    void setDependencies(const QStringList &dependencies);
    QList<Feature*> dependencies() const;

    QList<Feature*> supports() const;    
    QString getDocumentation() const;

    void setEnabled(bool on);
    bool enabled() const { return d->enabled; };

    bool selectable() const { return d->selectable; }
    
    QString toHtml() const;    
    
    ~Feature();    

signals:
    void changed();
    
private:
    Feature(const QString &key);
    void updateSelectable();
    
    static QMap<QString, Feature*> instances;
    FeaturePrivate *d;
};

QT_END_NAMESPACE

#endif // FEATURE_H
