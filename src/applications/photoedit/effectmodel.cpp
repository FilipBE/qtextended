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
#include "effectmodel.h"
#include <Qtopia>

class EffectParameterPrivate : public QSharedData
{
public:
    EffectParameterPrivate(){ ref.ref(); }
    EffectParameterPrivate(
            const QString &key, const QString &name, QVariant::Type type,
            const QVariant &defaultValue, const QVariant &minimum, const QVariant &maximum,
            const QVariant &step)
        : key(key), name(name), type(type), defaultValue(defaultValue), minimum(minimum), maximum(maximum), step(step) {}

    QString key;
    QString name;
    QVariant::Type type;
    QVariant defaultValue;
    QVariant minimum;
    QVariant maximum;
    QVariant step;
};

Q_GLOBAL_STATIC(EffectParameterPrivate, nullEffectParameterPrivate);

EffectParameter::EffectParameter()
    : d(nullEffectParameterPrivate())
{
}

EffectParameter::EffectParameter(const QString &key, const QString &name, QVariant::Type type, const QVariant &defaultValue, const QVariant &minimum, const QVariant &maximum, const QVariant &step)
    : d(new EffectParameterPrivate(key, name, type, defaultValue, minimum, maximum, step))
{
}

EffectParameter::EffectParameter(const EffectParameter &other)
    : d(other.d)
{
}

EffectParameter &EffectParameter::operator =(const EffectParameter &other)
{
    d = other.d;

    return *this;
}

EffectParameter::~EffectParameter()
{
}

QString EffectParameter::key() const
{
    return d->key;
}

QString EffectParameter::name() const
{
    return d->name;
}

QVariant::Type EffectParameter::type() const
{
    return d->type;
}

QVariant EffectParameter::defaultValue() const
{
    return d->defaultValue;
}

QVariant EffectParameter::minimum() const
{
    return d->minimum;
}

QVariant EffectParameter::maximum() const
{
    return d->maximum;
}

QVariant EffectParameter::step() const
{
    return d->step;
}

EffectModel::EffectModel(QObject *parent)
    : QAbstractListModel(parent)
{
    foreach (QString path, Qtopia::installPaths()) {
        QDir dir(path + QLatin1String("/etc/photoedit"));

        foreach (QString fileName, dir.entryList(QDir::Files)) {
            if (!fileName.endsWith(QLatin1String(".conf")))
                continue;

            QString plugin = fileName;
            plugin.chop(5);
            QString path = dir.absoluteFilePath(fileName);

            QSettings conf(path, QSettings::IniFormat);

            conf.beginGroup(QLatin1String("Translation"));
            QString translationFile = conf.value(QLatin1String("File")).toString();
            QString translationContext = conf.value(QLatin1String("Context")).toString();
            conf.endGroup();

            foreach (QString key, conf.childGroups()) {
                if (key == QLatin1String("Translation"))
                    continue;

                conf.beginGroup(key);

                Effect *effect = new Effect;
                effect->key = key;
                effect->plugin = plugin;
                effect->path = path;
                effect->name = Qtopia::translate(
                        translationFile,
                        translationContext,
                        conf.value(QLatin1String("Name[]")).toString());
                effect->preview = QIcon(QLatin1String(":image/") + conf.value("Preview").toString());
                m_effects.append(effect);

                conf.endGroup();
            }
        }
    }

    qSort(m_effects.begin(), m_effects.end(), effectsLessThan);

    Effect *effect = new Effect;
    effect->name = tr("None");
    effect->preview = QIcon(QLatin1String(":image/photoedit/baseimage"));
    m_effects.prepend(effect);
}

EffectModel::~EffectModel()
{
    qDeleteAll(m_effects);
}

QVariant EffectModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid()) {
        switch (role) {
        case Qt::DisplayRole:
            return m_effects.at(index.row())->name;
        case Qt::DecorationRole:
            return m_effects.at(index.row())->preview;
        }
    }
    return QVariant();
}

QModelIndex EffectModel::index(int row, int column, const QModelIndex &parent) const
{
    return row >= 0 && row < m_effects.count() && column == 0 && !parent.isValid()
        ? createIndex(row, column)
        : QModelIndex();
}

int EffectModel::rowCount(const QModelIndex &parent) const
{
    return !parent.isValid()
        ? m_effects.count()
        : 0;
}

QString EffectModel::effect(const QModelIndex &index) const
{
    return index.isValid()
        ? m_effects.at(index.row())->key
        : QString();
}

QString EffectModel::plugin(const QModelIndex &index) const
{
    return index.isValid()
        ? m_effects.at(index.row())->plugin
        : QString();
}

QList<EffectParameter> EffectModel::parameters(const QModelIndex &index) const
{
    QList<EffectParameter> parameters;
    if (index.isValid() && index.row() > 0) {
        Effect *effect = m_effects.at(index.row());

        QSettings conf(effect->path, QSettings::IniFormat);

        conf.beginGroup(QLatin1String("Translation"));
        QString translationFile = conf.value(QLatin1String("TransltionFile")).toString();
        QString translationContext = conf.value(QLatin1String("TranslationContext")).toString();
        conf.endGroup();

        conf.beginGroup(m_effects.at(index.row())->key);
        conf.beginGroup(QLatin1String("Parameters"));
        foreach (QString parameterKey, conf.childGroups()) {
            conf.beginGroup(parameterKey);
            parameters.append(EffectParameter(
                parameterKey,
                Qtopia::translate(
                        translationFile, translationContext,
                        conf.value(QLatin1String("Name[]")).toString()),
                QVariant::nameToType(
                        conf.value(QLatin1String("Type")).toString().toLatin1().constData()),
                conf.value(QLatin1String("Default")),
                conf.value(QLatin1String("Minimum")),
                conf.value(QLatin1String("Maximum")),
                conf.value(QLatin1String("Step"))));
            conf.endGroup();
        }
        conf.endGroup();
        conf.endGroup();
    }
    return parameters;
}

bool EffectModel::effectsLessThan(Effect *effect1, Effect *effect2)
{
    return QString::localeAwareCompare(effect1->name, effect2->name) < 0;
}
