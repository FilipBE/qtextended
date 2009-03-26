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

#include "applist.h"
#include "appdetails.h"

#include <QListView>
#include <QLabel>
#include <QWaitWidget>
#include <QSoftMenuBar>
#include <QMenu>
#include <QVBoxLayout>

#include <QtopiaApplication>
#include <QTimer>

#include <QtopiaService>
#include <QTranslatableSettings>
#include <QContentSet>
#include <QPersistentModelIndex>

#include <QDebug>

class SelectableContentSetModel : public QContentSetModel
{
    Q_OBJECT

    public:
        explicit SelectableContentSetModel(const QString &service, const QContentSet *cs, QObject *parent = 0)
            : QContentSetModel(cs, parent),
              m_service(service)
        {
            connect(this, SIGNAL(modelReset()), this, SLOT(setBoundApplication()));
        }

        virtual ~SelectableContentSetModel()
        {
        }

        virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const
        {
            if (role == Qt::FontRole &&
                m_bound == index) {
                QFont f;
                f.setBold(true);
                return f;
            }

            return QContentSetModel::data(index, role);
        }

        QModelIndex boundApplication() const
        {
            return m_bound;
        }

        void setBoundApplication(const QModelIndex &index)
        {
            QModelIndex current = m_bound;
            emit dataChanged(current, current);
            m_bound = index;
            emit dataChanged(index, index);
        }

    private slots:
        void setBoundApplication()
        {
            const QString app = QtopiaService::app(m_service);

            for (int i = 0; i < rowCount(); i++) {
                QContent c = content(i);
                if (c.executableName() == app) {
                    m_bound = index(i);
                    break;
                }
            }
        }

    private:
        QPersistentModelIndex m_bound;
        QString m_service;
};

#include "applist.moc"

AppList::AppList(const QString &service, QWidget *parent, Qt::WFlags fl)
    : QDialog(parent, fl),
      m_model(0),
      m_service(service)
{
    setWindowTitle(tr("Application Services"));

    QVBoxLayout *vbox = new QVBoxLayout();

    QTranslatableSettings settings(QtopiaService::config(service), QSettings::IniFormat);
    settings.beginGroup("Service");

    const QString name = settings.value("Name", service).toString();
    QLabel *label = new QLabel(tr("%1 is provided by", "%1=<service name>").arg(name));
    label->setWordWrap(true);
    vbox->addWidget(label);

    m_applicationView = new QListView();
    m_applicationView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    vbox->addWidget(m_applicationView);
    connect(m_applicationView, SIGNAL(activated(QModelIndex)),
            this, SLOT(activated(QModelIndex)));

    setLayout(vbox);

    QMenu *menu = QSoftMenuBar::menuFor(m_applicationView);
    menu->addAction(tr("Details..."), this, SLOT(showDetails()));

    m_applications = new QContentSet;
    m_model = new SelectableContentSetModel(m_service, m_applications);
    m_applicationView->setModel(m_model);
    connect(m_applicationView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            this, SLOT(currentChanged(QModelIndex,QModelIndex)));

    QTimer::singleShot(0, this, SLOT(loadState()));
}

AppList::~AppList()
{
    delete m_applications;
    delete m_model;
}

void AppList::loadState()
{
    QWaitWidget *waitWidget = new QWaitWidget(this);
    waitWidget->show();
    QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

    loadApplications();

    delete waitWidget;
}

void AppList::loadApplications()
{
    m_applications->clear();

    QContentSet allApplications(QContent::Application);

    foreach (const QString &app, QtopiaService::apps(m_service)) {
        QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

        if (allApplications.findExecutable(app).isValid()) {
            m_applications->addCriteria(QContentFilter::FileName, app, QContentFilter::Or);
        } else {
            // fake content for system provided services
            QContent system(app, false);
            system.setFile(app);
            system.setName(tr("System"));
            system.setIcon("systemowned");
            system.setRole(QContent::Application);

            m_applications->add(system);
        }
    }

    if (m_applications->filter().isValid())
        m_applications->addCriteria(QContentFilter(QContent::Application), QContentFilter::And);
}

void AppList::activated(const QModelIndex &index)
{
    QSettings binding("Trolltech", QtopiaService::binding(m_service));
    binding.beginGroup("Service");
    binding.setValue("default", m_model->content(index).executableName());

    m_model->setBoundApplication(index);

    currentChanged(index, index);
}

void AppList::showDetails()
{
    QContent app = m_model->content(m_applicationView->currentIndex());
    AppDetails *appDetails = new AppDetails(app);
    QtopiaApplication::execDialog(appDetails);

    if (appDetails->result() == QDialog::Accepted)
        loadApplications();

    delete appDetails;
}

void AppList::currentChanged(const QModelIndex &current, const QModelIndex &)
{
    if (current == m_model->boundApplication())
        QSoftMenuBar::setLabel(m_applicationView, Qt::Key_Select, QSoftMenuBar::NoLabel);
    else
        QSoftMenuBar::setLabel(m_applicationView, Qt::Key_Select, QSoftMenuBar::Select);
}

