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

#include "testcontextlabel.h"
#include "localwidget.h"
#include "testwidgetslog.h"

#include <ThemedView>
#include <QThemedView>
#include <QThemeTextItem>
#include <Qtopia>
#include <QSoftMenuBar>
#include <QKeyEvent>
#include <QtUiTestRecorder>

TestContextLabel::TestContextLabel(QObject *_q)
: TestThemedView(_q), q(qobject_cast<QThemedView*>(_q)), optionsIndex(-1),
  recorder(0)
{
    /* Disconnect this because we need to do something more special for
     * recording of soft menu selection.
     */
    disconnect(q, SIGNAL(itemPressed(QThemeItem*)), this, 0);

    for (int i = 0; i < QSoftMenuBar::keys().count() && -1 == optionsIndex; ++i) {
        if (QSoftMenuBar::menuKey() == QSoftMenuBar::keys().at(i)) {
            optionsIndex = i;
        }
    }

    QObject* smp = LocalWidget::find("QSoftMenuBarProvider");
    if (smp) {
        if (!connect(smp, SIGNAL(keyChanged(int)), this, SLOT(on_keyChanged(int))))
            Q_ASSERT(false);
    }
}

void TestContextLabel::connectNotify(const char* signal)
{
    if (recorder) return;

    if (QLatin1String(signal) == SIGNAL(selected(QString))) {
        recorder = new QtUiTestRecorder(this);
        connect(recorder, SIGNAL(keyEvent(int,int,bool,bool)),
                this,     SLOT(on_keyEvent(int,int,bool,bool)));
    }
}

void TestContextLabel::on_keyEvent(int key,int,bool press,bool)
{
    /* Watch for selecting things from context label by key presses.
     * Note that clicking on a soft menu label actually causes a key
     * press to be generated, so we don't need to monitor mouse clicks.
     */
    if (!press || (lastEmit.elapsed() >= 0 && lastEmit.elapsed() < 100)) {
        TestWidgetsLog() << "Skipping emit:" << press << lastEmit.elapsed();
        return;
    }
    if (!q) return;

    QList<int> keys = QSoftMenuBar::keys();
    int i = -1;
    for (i = 0; i < keys.count(); ++i) {
        if (key == keys.at(i))
            break;
    }
    QThemeTextItem *item = qgraphicsitem_cast<QThemeTextItem*>(q->findItem( QString("tbutton%1").arg(i) ));
    if (item && !item->text().isEmpty()) {
        lastEmit.start();
        emit selected(item->text());
    }
}

void TestContextLabel::on_keyChanged(int key) {
    if (key != QSoftMenuBar::menuKey()) return;
    if (-1 == optionsIndex) return;
    if (!q) return;

    QThemeTextItem *item = qgraphicsitem_cast<QThemeTextItem*>(q->findItem( QString("tbutton%1").arg(optionsIndex) ));
    if (!item) return;
    QString optionsLabel = item->text();
    if ((lastOptionsLabel == "Hide") && (optionsLabel != "Hide") && (lastEmit.elapsed() > 100)) {
        lastEmit.start();
        emit selected("Hide");
    }
    lastOptionsLabel = optionsLabel;
}

QString TestContextLabel::text() const
{ return list().join("\n"); }

QStringList TestContextLabel::list() const
{
    QStringList ret;
    if (!q) return ret;

    foreach (QString button, QStringList() << "tbutton0" << "tbutton1" << "tbutton2") {
        QThemeItem* item = q->findItem(button);
        QThemeTextItem *textItem = qgraphicsitem_cast<QThemeTextItem*>(item);
        if (textItem) ret.append( textItem->text() );
        else          ret.append( QString() );
    }

    return ret;
}

bool TestContextLabel::canSelect(QString const& item) const
{
    TestWidgetsLog() << item << list();
    return list().contains(item);
}

bool TestContextLabel::select(QString const& item)
{
    TestWidgetsLog() << item;
    QStringList items = list();
    int index = items.indexOf(item);
    if (-1 == index) return false;

    /* ContextLabel cannot use the 'safe' keyClick and mouseClick,
     * because it processes events on press rather than click.
     */

    if (Qtopia::mousePreferred()) {
        /* Hide is a special case; it can take more than one click to
         * take focus away from the menu.
         */
        if (item == "Hide") {
            for (int i = 0; i < 3 && list().contains(item); ++i) {
                QtUiTest::mouseClick(mapToGlobal(visualRect(item).center()));
                QtUiTest::wait(qApp->doubleClickInterval());
            }
            return !list().contains(item);
        }
        QtUiTest::wait(qApp->doubleClickInterval());
        QtUiTest::mouseClick(mapToGlobal(visualRect(item).center()), Qt::LeftButton);
        return true;
    }

    struct GetKeys {
        static QList<Qt::Key> list() {
            QList<Qt::Key> ret;
            QSettings cfg(Qtopia::defaultButtonsFile(), QSettings::IniFormat);
            cfg.beginGroup("SoftKeys");
            int buttonCount = cfg.value("Count", 0).toInt();

            for(int ii = 0; ii < buttonCount; ++ii)
                ret << static_cast<Qt::Key>(
                       QKeySequence(cfg.value("Key" + QString::number(ii)).toString())[0]);
            return ret;
        }
    };

    static const QList<Qt::Key> keys( GetKeys::list() );

    if (index > keys.count()) {
        qWarning("QtUitest: can't figure out soft menu key configuration!");
        return false;
    }

    /*
        FIXME: there is no way to block until we have really finished selecting from the
        context label.  The context label doesn't actually _do_ anything when not clicked
        on - it's purely a label whose text changes every now and again.
        Since we have no idea of knowing what effect generating this key click will have,
        we can't wait for it.  Therefore everyone who calls this function must handle waiting
        by themselves.
    */
    QtUiTest::keyClick(keys.at(index));
    return true;
}

bool TestContextLabel::inherits(QtUiTest::WidgetType type) const
{ return (QtUiTest::SoftMenu == type); }

bool TestContextLabel::canWrap(QObject *o)
{
    while (o) {
        if (o->inherits("QAbstractContextLabel")) return true;
        o = o->parent();
    }
    return false;
}

