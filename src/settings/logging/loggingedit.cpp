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
#include "loggingedit.h"

#include <qtopiaapplication.h>
#include <qsoftmenubar.h>
#include <qtranslatablesettings.h>
#include <QTreeWidget>
#include <QItemDelegate>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QWhatsThis>
#include <QHelpEvent>
#include <QMenu>
#include <QtopiaIpcEnvelope>

class AllCheckDelegate : public QItemDelegate {
public:
    AllCheckDelegate(QObject *parent) : QItemDelegate(parent) {}
    bool editorEvent(QEvent *event,
                    QAbstractItemModel *model,
                    const QStyleOptionViewItem &option,
                    const QModelIndex &index)
    {
        // Avoid the requirement to check the checkbox - whole area works.
        // XXX should this be a style option supported by Qt?
        Qt::ItemFlags flags = model->flags(index);
        if (!(flags & Qt::ItemIsUserCheckable) || !(option.state & QStyle::State_Enabled)
            || !(flags & Qt::ItemIsEnabled))
            return false;
        QVariant value = index.data(Qt::CheckStateRole);
        if (!value.isValid())
            return false;
        if (event->type() == QEvent::MouseButtonRelease) {
            Qt::CheckState state = (static_cast<Qt::CheckState>(value.toInt()) == Qt::Checked
                            ? Qt::Unchecked : Qt::Checked);
            return model->setData(index, state, Qt::CheckStateRole);
        }
        return QItemDelegate::editorEvent(event,model,option,index);
    }
};


QWhatThisInMenu::QWhatThisInMenu(QWidget* parent)
{
    QMenu* menu = QSoftMenuBar::menuFor( parent );
    action = QWhatsThis::createAction(parent);
    action->setShortcut(QKeySequence()); // XXX shortcut not appropriate for Qtopia
    connect( action, SIGNAL(triggered()), this, SLOT(showWhatsThis()) );
    connect( menu, SIGNAL(aboutToShow()), this, SLOT(findWhatsThis()) );
    menu->addAction( action );
}

void QWhatThisInMenu::findWhatsThis()
{
    action->setVisible(sendWhatsThisEvent(true));
}

void QWhatThisInMenu::showWhatsThis()
{
    sendWhatsThisEvent(false);
}

bool QWhatThisInMenu::sendWhatsThisEvent(bool query) // internal
{
    QWidget* whatswhat = QApplication::focusWidget();
    if ( whatswhat ) {
        QPoint p = whatswhat->inputMethodQuery(Qt::ImMicroFocus).toRect().center();
        QPoint gp = whatswhat->mapToGlobal(p);
        QWidget *whatsreallywhat = QApplication::widgetAt(gp);
        if ( whatsreallywhat ) {
            whatswhat = whatsreallywhat;
            p = whatsreallywhat->mapFromGlobal(gp);
        }
        QHelpEvent e(query ? QEvent::QueryWhatsThis : QEvent::WhatsThis, p, gp);
        if (QApplication::sendEvent(whatswhat, &e))
            return e.isAccepted();
    }
    return false;
}

LoggingEdit::LoggingEdit( QWidget* parent, Qt::WFlags fl )
:   QDialog( parent, fl )
{
    setWindowTitle(tr("Categories"));

    QVBoxLayout* vbLayout = new QVBoxLayout(this);
    vbLayout->setMargin(0);
    vbLayout->setSpacing(0);

    new QWhatThisInMenu(this);

    // Using a tree since when the number of log streams gets large, we
    // should put them in an hierarchy.
    list = new QTreeWidget;
    list->setItemDelegate(new AllCheckDelegate(list));

    list->setRootIsDecorated(false);
    list->setColumnCount(1);
    list->header()->hide();
    list->setSelectionMode(QAbstractItemView::SingleSelection);
    list->setFrameStyle(QFrame::NoFrame);
    vbLayout->addWidget(list);

    QTranslatableSettings settings("Trolltech","Log");

    foreach (QString group, settings.childGroups()) {
        settings.beginGroup(group);
        QString name = settings.value("Name").toString();
        QStringList missing = settings.value("Requires").toStringList();

#ifdef QTOPIA_CELL
        missing.removeAll("CELL");
#endif
#ifdef QTOPIA_DRM
        missing.removeAll("DRM");
#endif
#ifndef QT_NO_SXE
        missing.removeAll("SXE");
#endif
#ifdef QTOPIA_VOIP
        missing.removeAll("VOIP");
#endif
#ifdef QTOPIA_TEST
        missing.removeAll("TEST");
#endif

        if ( name.isEmpty() )
            name = group; // Allow for displaying unadvertised values that the user has added to Log.conf
        if ( !name.isEmpty() && missing.isEmpty() ) {
            QTreeWidgetItem *i = new QTreeWidgetItem(list,QStringList() << name);
            i->setCheckState(0,qtopiaLogEnabled(group.toLatin1()) ? Qt::Checked : Qt::Unchecked);
            if (!qtopiaLogOptional(group.toLatin1())) {
                i->setFlags(i->flags() & ~Qt::ItemIsEnabled);
            }
            QString helptext = settings.value("Help").toString();
            if (!helptext.isEmpty())
                i->setWhatsThis(0,helptext);
            item.insert(group,i);
        }
        settings.endGroup();
    }

    list->sortItems( 0, Qt::AscendingOrder );
    list->setCurrentItem(list->topLevelItem(0));
    showMaximized();
}

LoggingEdit::~LoggingEdit()
{
}

void LoggingEdit::accept()
{
    QTranslatableSettings settings("Trolltech","Log");

    foreach (QString group, settings.childGroups()) {
        QTreeWidgetItem *i = item[group];
        if ( i ) {
            settings.beginGroup(group);
            settings.setValue("Enabled",item[group]->checkState(0) == Qt::Checked);
            settings.endGroup();
        }
    }

    // Write out Log.conf
    settings.sync();
    // Tell all apps to reload Log.conf (has an immediate effect on qLog() calls)
    {QtopiaIpcEnvelope e("QPE/System", "LogConfChanged()");}
    QDialog::accept();
}
