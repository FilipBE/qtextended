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

#include "e2_taskmanager.h"
#include "e2_bar.h"

#include <QWidget>
#include <QBoxLayout>
#include <QTreeWidget>
#include <QFile>
#include <QPainter>
#include <QPen>
#include <QFontMetrics>
#include <QLabel>
#include <QContentSet>
#include <QtopiaIpcEnvelope>
#include <QHeaderView>

class E2MemoryBar : public QWidget
{
public:
    E2MemoryBar(QWidget* par) : QWidget(par), m_percent(0) {
        QSizePolicy sp = sizePolicy();
        setSizePolicy(QSizePolicy(QSizePolicy::Expanding, sp.verticalPolicy()));
        QFontMetrics fm(font());
        int h = fm.height() + 2*2+2;
        setFixedHeight(h);
    }

    void setPercent(int p) {
        m_percent = p;
        update();
    }

    QSize sizeHint() const {
        QSize s;
        if(parentWidget())
            s = QSize(parentWidget()->width(), height());
        else
            s = QSize(50,height());
        return s;
    }

protected:
    void paintEvent(QPaintEvent*) {
        QPainter p(this);
        QPen pen = p.pen();
        pen.setColor(Qt::red);
        p.setPen(pen);
        int usedSteps = m_percent * (width()-3) / 100;
        int freeSteps = (width()-3)-usedSteps;
        p.setBrush(QBrush(Qt::green));
        p.drawRect(0, 0, usedSteps, height()-1);

        p.setBrush(QBrush(Qt::gray));
        p.drawRect(usedSteps, 0, freeSteps, height()-1);

        pen.setColor(Qt::blue);
        p.setPen(pen);
        p.drawText(rect(), Qt::AlignCenter, QString::number(m_percent)+QString("%"));
    }

private:
    int m_percent;
};

E2TaskManagerService::E2TaskManagerService(QObject* par)
    : TaskManagerService(par)
{
}

void E2TaskManagerService::multitask()
{
}

void E2TaskManagerService::showRunningTasks()
{
    E2TaskManager::instance()->show();
}

E2TaskManager::E2TaskManager(QWidget* par)
    : E2PopupFrame(par)
{
    QVBoxLayout* vlayout = new QVBoxLayout(this);

    QLabel* label = new QLabel(this);
    label->setText(tr("System Memory Status"));
    vlayout->addWidget(label);

    m_memoryBar = new E2MemoryBar(this);
    vlayout->addWidget(m_memoryBar);
    memoryUpdate(); // force update

    m_memoryTimer = new QTimer(this);
    m_memoryTimer->setInterval(30000);
    connect(m_memoryTimer, SIGNAL(timeout()), this, SLOT(memoryUpdate()));

    m_taskList = new QTreeWidget(this);
    m_taskList->header()->setResizeMode(QHeaderView::Interactive);
    m_taskList->setRootIsDecorated(false);
    m_taskList->setItemsExpandable(false);
    QStringList labels;
    labels << QString("") << QString(tr("Name")) << QString("");
    m_taskList->setHeaderLabels(labels);
    m_taskList->header()->setDefaultAlignment(Qt::AlignCenter);
    m_taskList->header()->setStretchLastSection(true);
    vlayout->addWidget(m_taskList);

    m_bar = new E2Bar(this);
    m_bar->setFixedHeight(24);
    vlayout->addWidget(m_bar);
    E2Button* button = new E2Button(m_bar);
    button->setText(tr("Switch", "Switch task"));
    connect(button, SIGNAL(clicked()), this, SLOT(switchToTask()));
    m_bar->addButton(button,0);
    button = new E2Button(m_bar);
    button->setText(tr("End", "End task"));
    connect(button, SIGNAL(clicked()), this, SLOT(endTask()));
    m_bar->addButton(button,0);
    button = new E2Button(m_bar);
    button->setText(tr("Cancel"));
    connect(button, SIGNAL(clicked()), this, SLOT(close()));
    m_bar->addButton(button,0);

    QObject::connect(&m_appMonitor, SIGNAL(applicationRunning(QString)),
                     this, SLOT(doUpdate()));
    QObject::connect(&m_appMonitor, SIGNAL(applicationClosed(QString)),
                     this, SLOT(doUpdate()));
}

void E2TaskManager::switchToTask()
{
    close();
    for(int i = 0 ;i < m_items.count();++i) {
        if(m_items[i].first->checkState(0) == Qt::Checked) {
            if(m_items[i].second->isValid()) {
                m_items[i].second->execute();
                return;
            }
        }
    }
}

void E2TaskManager::endTask()
{
    close();
    for(int i = 0;i < m_items.count();++i) {
        if(m_items[i].first->checkState(0) == Qt::Checked) {
            if(m_items[i].second->isValid()) {
                QtopiaIpcEnvelope closeenv(QString("QPE/Application/")+m_items[i].second->executableName(), "close()");
            }
        }
    }
}

E2TaskManager* E2TaskManager::instance()
{
    static E2TaskManager* _this = 0;
    if(!_this)
        _this = new E2TaskManager(0);
    return _this;
}

void E2TaskManager::showEvent(QShowEvent* e)
{
    m_memoryTimer->start();
    memoryUpdate();
    E2PopupFrame::showEvent(e);
    doUpdate();
}

void E2TaskManager::doUpdate()
{
    if(isVisible()) {
        m_taskList->clear();
        while(m_items.count()) {
            QPair<QTreeWidgetItem*,QContent*> p = m_items.takeFirst();
            delete p.second;
        }
        QStringList apps = m_appMonitor.runningApplications();

        QContentSet set(QContent::Application);
        for (int ii=0; ii < apps.count(); ii++) {
            QContent app = set.findExecutable(apps.at(ii));
            if(app.isValid()) {
                QStringList labels;
                labels << QString("") << app.name() << QString("");
                QTreeWidgetItem* item = new QTreeWidgetItem(labels);
                item->setCheckState(0,Qt::Unchecked);
                m_taskList->addTopLevelItem(item);
                m_items.append(qMakePair(item, new QContent(app)));
            }
        }
    }
}


void E2TaskManager::memoryUpdate()
{
    m_memoryBar->setPercent(getMemoryPercentUsed());
}

int E2TaskManager::getMemoryPercentUsed() const
{
    QFile file( "/proc/meminfo" );
    if ( file.open( QIODevice::ReadOnly ) ) {
        QTextStream t( &file );
        QString all = t.readAll();
        int total=0, memfree=0, buffers=0, cached = 0;
        int pos = 0;
        QRegExp regexp("(MemTotal:|MemFree:|Buffers:|\\bCached:)\\s*(\\d+) kB");
        while ( (pos = regexp.indexIn( all, pos )) != -1 ) {
            if ( regexp.cap(1) == "MemTotal:" )
                total = regexp.cap(2).toInt();
            else if ( regexp.cap(1) == "MemFree:" )
                memfree = regexp.cap(2).toInt();
            else if ( regexp.cap(1) == "Buffers:" )
                buffers = regexp.cap(2).toInt();
            else if ( regexp.cap(1) == "Cached:" )
                cached = regexp.cap(2).toInt();
            pos += regexp.matchedLength();
        }

        int realUsed = total - ( buffers + cached + memfree );
        realUsed += buffers;
        memfree += cached;
        /*
        QString unit = tr("kB");
        if (total > 10240) {
            realUsed = realUsed / 1024;
            memfree = memfree / 1024;
            total = total / 1024;
            unit = tr("MB");
        }
        //data->addItem( tr("Used (%1 %2)", "%1 = number, %2 = unit").arg(realUsed).arg(unit), realUsed );
        //data->addItem( tr("Free (%1 %2)", "%1 = number, %2 = unit").arg(memfree).arg(unit), memfree );
        //totalMem->setText( tr( "Total Memory: %1 %2", "%1 = 512 %2 = MB/kB" ).arg( total ).arg( unit ));
        */
        return realUsed * 100 / total;
    }
    return 0;
}

