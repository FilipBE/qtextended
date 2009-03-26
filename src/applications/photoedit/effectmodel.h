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
#ifndef EFFECTMODEL_H
#define EFFECTMODEL_H

#include <QAbstractListModel>
#include <QSharedData>
#include <QSettings>
#include <QIcon>
#include <QtAlgorithms>

class EffectParameterPrivate;

class EffectParameter
{
public:
    EffectParameter();
    EffectParameter(
            const QString &key, const QString &name, QVariant::Type type,
            const QVariant &defaultValue, const QVariant &minimum = QVariant(),
            const QVariant &maximum = QVariant(), const QVariant &step = QVariant());
    EffectParameter(const EffectParameter &other);
    EffectParameter &operator =(const EffectParameter &other);
    ~EffectParameter();

    QString key() const;
    QString name() const;
    QVariant::Type type() const;
    QVariant defaultValue() const;
    QVariant minimum() const;
    QVariant maximum() const;
    QVariant step() const;

private:
    QExplicitlySharedDataPointer<EffectParameterPrivate> d;
};

class EffectModel : public QAbstractListModel
{
    Q_OBJECT
public:
    EffectModel(QObject *parent = 0);
    ~EffectModel();

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;

    QString effect(const QModelIndex &index) const;
    QString plugin(const QModelIndex &index) const;
    QList<EffectParameter> parameters(const QModelIndex &index) const;

private:
    struct Effect
    {
        QString key;
        QString plugin;
        QString path;
        QString name;
        QIcon preview;
    };

    static bool effectsLessThan(Effect *effect1, Effect *effect2);

    QList<Effect *> m_effects;
};


#endif
