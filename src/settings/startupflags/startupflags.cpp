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

#include "startupflags.h"

#include <Qtopia>

#include <qtopia/qtopiaapplication.h>
#include <qtopia/qsoftmenubar.h>

#include <qtopiabase/qtranslatablesettings.h>
#include <qtopia/qsoftmenubar.h>

#include <QTreeWidget>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QWhatsThis>
#include <QHelpEvent>
#include <QMenu>

StartupFlags::StartupFlags( QWidget* parent, Qt::WFlags fl )
:   QDialog( parent, fl )
{
    setWindowTitle(tr("Startup Flags"));

    QVBoxLayout* vbLayout = new QVBoxLayout(this);
    vbLayout->setMargin(0);
    vbLayout->setSpacing(0);

    QMenu* menu = QSoftMenuBar::menuFor( this );

    QAction *aWhatsThis = QWhatsThis::createAction(this);
    QAction *qtopiaWhatsThis = new QAction(aWhatsThis->icon(),aWhatsThis->text(),this);
    connect(qtopiaWhatsThis, SIGNAL(triggered()), this, SLOT(showWhatsThis()));
    menu->addAction(qtopiaWhatsThis);
    delete aWhatsThis;

    list = new QTreeWidget;

    connect(list, SIGNAL(itemActivated(QTreeWidgetItem*,int)),
            this, SLOT(flagChanged(QTreeWidgetItem*,int)));
    connect(list, SIGNAL(itemClicked(QTreeWidgetItem*,int)),
            this, SLOT(flagChanged(QTreeWidgetItem*,int)));

    list->setRootIsDecorated(false);
    list->setColumnCount(1);
    list->header()->hide();
    list->setSelectionMode(QAbstractItemView::SingleSelection);
    list->setFrameStyle(QFrame::NoFrame);
    vbLayout->addWidget(list);

    loadSettings();

    list->setCurrentItem(list->topLevelItem(0));
}

StartupFlags::~StartupFlags()
{
}

void StartupFlags::showWhatsThis()
{
    QTreeWidgetItem *item = list->currentItem();
    QPoint p = list->visualItemRect(item).center();

    QWhatsThis::showText(p, item->whatsThis(0), list);
}

void StartupFlags::flagChanged(QTreeWidgetItem *item, int column)
{
    if ( item ) {
        // Check tree level
        int level = 0;
        QTreeWidgetItem *p = item;
        while (p != NULL) {
            if (p->parent()) {
                level++;
            }
            p = p->parent();
        }

        if (level == 2) {
            // Radio button
            for (int i = 0; i < item->parent()->childCount(); i++) {
                if (item == item->parent()->child(i))
                    item->setCheckState(column, Qt::Checked);
                else
                    item->parent()->child(i)->setCheckState(column, Qt::Unchecked);
            }
        }
    }
}

void StartupFlags::accept()
{
    QTranslatableSettings lSettings("Trolltech", "StartupFlags");

    foreach (QString group, lSettings.childGroups()) {
        QTreeWidgetItem *i = item[group];
        if (i) {
            lSettings.beginGroup(group);

            QString type = lSettings.value("Type").toString();
            QString context = lSettings.value("Context", "Local").toString();

            QTranslatableSettings *s = NULL;
            QString key = "State";

            // Find settings file to write to
            if (lSettings.contains("State")) {
                s = &lSettings;
            } else if (lSettings.contains("Settings")) {
                QStringList sList = lSettings.value("Settings").toString().split(",");
                switch (sList.count()) {
                case 2:
                    s = new QTranslatableSettings(sList.at(0), sList.at(1));
                    break;
                case 1: {
                    QString settingsFile = sList.at(0);
                    if (!settingsFile.startsWith('/')) {
                        if (QFileInfo(Qtopia::updateDir() + "etc/" + settingsFile).exists())
                            settingsFile.prepend(Qtopia::updateDir() + "etc/");
                        else
                            settingsFile.prepend(Qtopia::qtopiaDir() + "etc/");
                    }

                    s = new QTranslatableSettings(settingsFile, QSettings::NativeFormat);
                    break;
                }
                default:
                    break;
                }
                s->beginGroup(lSettings.value("Group").toString());
                key = lSettings.value("Key").toString();
            } else {
                continue;
            }

            // Write setting
            if (type == "Bool") {
                s->setValue(key, i->checkState(0) == Qt::Checked);
            } else if (type == "IntBool") {
                s->setValue(key, static_cast<int>(i->checkState(0) == Qt::Checked));
            } else if (type == "String") {
                QString value;

                QStringList values = lSettings.value("Values").toString().split(",");
                if (!values.isEmpty())
                    value = values.first();

                for (int j = 0; j < i->childCount(); j++) {
                    QTreeWidgetItem *child = i->child(j);
                    if (child->checkState(0) == Qt::Checked)
                        value = child->text(0);
                }
                
                s->setValue(key, value);
            }

            if (s != &lSettings) {
                s->endGroup();
                delete s;
            }

            lSettings.endGroup();
        }
    }

    QDialog::accept();
}

void StartupFlags::loadSettings()
{
    QTranslatableSettings settings("Trolltech", "StartupFlags");

    foreach (QString group, settings.childGroups()) {
        settings.beginGroup(group);
        QString name = settings.value("Name[]").toString();
        if ( !name.isEmpty() ) {
            QTreeWidgetItem *contextItem;

            // Add context item if it does not exist already
            QString context = settings.value("Context", "Local").toString();
            if (contexts.contains(context)) {
                contextItem = contexts.value(context);
            } else {
                contextItem = new QTreeWidgetItem(list, QStringList() << context);
                contextItem->setFlags(contextItem->flags() & ~Qt::ItemIsSelectable);
                list->expandItem(contextItem);
                contexts.insert(context, contextItem);
            }

            QTreeWidgetItem *i = new QTreeWidgetItem(contextItem,
                                                     QStringList() << name);

            if (settings.contains("Type")) {
                QString type = settings.value("Type").toString();
                if (type == "Bool" || type == "IntBool") {
                    i->setCheckState(0, currentValue(group).toBool() ? Qt::Checked : Qt::Unchecked);
                } else if (type == "String") {
                    QStringList values = settings.value("Values").toString().split(",");
                    QString curValue = currentValue(group).toString();
                    if (!values.contains(curValue))
                        values.append(curValue);

                    for (int j = 0; j < values.size(); j++) {
                        QTreeWidgetItem *v = new QTreeWidgetItem(i, QStringList() << values.at(j));
                        if (values.at(j) == curValue)
                            v->setCheckState(0, Qt::Checked);
                        else
                            v->setCheckState(0, Qt::Unchecked);

                        v->setWhatsThis(0, settings.value("Help[]").toString());
                    }
                    i->setFlags(i->flags() & ~Qt::ItemIsSelectable);
                    list->expandItem(i);
                } else {
                    i->setFlags(i->flags() & ~Qt::ItemIsSelectable);
                }
            }

            i->setWhatsThis(0, settings.value("Help[]").toString());
            item.insert(group, i);
        }
        settings.endGroup();
    }
}

QVariant StartupFlags::currentValue(QString group)
{
    QTranslatableSettings lSettings("Trolltech", "StartupFlags");

    QVariant ret;

    lSettings.beginGroup(group);

    QString type = lSettings.value("Type").toString();
    QString context = lSettings.value("Context", "Local").toString();

    QTranslatableSettings *s = NULL;
    QString key = "State";

    // Find settings file to read from
    if (lSettings.contains("State")) {
        s = &lSettings;
    } else if (lSettings.contains("Settings")) {
        QStringList sList = lSettings.value("Settings").toString().split(",");
        switch (sList.count()) {
        case 2:
            s = new QTranslatableSettings(sList.at(0), sList.at(1));
            break;
        case 1: {
            QString settingsFile = sList.at(0);
            if (!settingsFile.startsWith('/')) {
                if (QFileInfo(Qtopia::updateDir() + "etc/" + settingsFile).exists())
                    settingsFile.prepend(Qtopia::updateDir() + "etc/");
                else
                    settingsFile.prepend(Qtopia::qtopiaDir() + "etc/");
            }

            s = new QTranslatableSettings(settingsFile, QSettings::NativeFormat);
            break;
        }
        default:
            break;
        }
        s->beginGroup(lSettings.value("Group").toString());
        key = lSettings.value("Key").toString();
    }

    // Read setting
    if (s) {
        if (type == "Bool") {
            ret = s->value(key, false);
        } else if (type == "IntBool") {
            ret = s->value(key, 0);
        } else if (type == "String") {
            QStringList values = lSettings.value("Values").toString().split(",");

            if (!values.isEmpty())
                ret = s->value(key, values.first());
            else
                ret = s->value(key, QString());
        }

        if (s != &lSettings) {
            s->endGroup();
            delete s;
        }
    }

    lSettings.endGroup();

    return ret;
}

