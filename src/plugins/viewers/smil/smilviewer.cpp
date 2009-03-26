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

#include "smilviewer.h"
#include <element.h>
#include <smil.h>
#include <timing.h>
#include <transfer.h>
#include <qtopialog.h>
#include <QBuffer>
#include <QKeyEvent>
#include <QMailMessage>
#include <QSoftMenuBar>
#include <QThread>
#include <QQueue>
#include <QMutex>
#include <QMutexLocker>
#include <QTimer>
#include <QVBoxLayout>
#include <QLabel>
#include <QEvent>
#include <QStackedWidget>
#include <QLabel>

class MMSWidget : public QWidget
{
    Q_OBJECT
public:
    enum DisplayMode {Loading, Playback};

public:
    MMSWidget(QWidget* parent = 0);

    void setDisplayMode(DisplayMode m);
    bool setSource(const QString& s);
    SmilElement* rootElement() const;
    void play();
    void reset();

signals:
    void transferRequested(SmilDataSource* source, const QString& sourceString);
    void transferCancelled(SmilDataSource* source, const QString& sourceString);
    void finished();

protected:
    void hideEvent(QHideEvent*);

private:
    QStackedWidget* m_widgetStack;
    SmilView* m_smilView;
    QLabel* m_loadingLabel;
};

MMSWidget::MMSWidget(QWidget* parent)
    :
    QWidget(parent),
    m_widgetStack(new QStackedWidget(this)),
    m_smilView(new SmilView(this)),
    m_loadingLabel(new QLabel("Loading...",this))
{
    m_loadingLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    m_widgetStack->addWidget(m_loadingLabel);
    m_widgetStack->addWidget(m_smilView);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(0);
    layout->addWidget(m_widgetStack);

    connect(m_smilView,
            SIGNAL(transferRequested(SmilDataSource*,QString)),
            this,
            SIGNAL(transferRequested(SmilDataSource*,QString)));
    connect(m_smilView,
            SIGNAL(transferCancelled(SmilDataSource*,QString)),
            this,
            SIGNAL(transferCancelled(SmilDataSource*,QString)));
    connect(m_smilView,
            SIGNAL(finished()),
            this,
            SIGNAL(finished()));

    setDisplayMode(Loading);
}

void MMSWidget::setDisplayMode(MMSWidget::DisplayMode m)
{
    if(m == Loading)
        m_widgetStack->setCurrentWidget(m_loadingLabel);
    else
        m_widgetStack->setCurrentWidget(m_smilView);
}

bool MMSWidget::setSource(const QString& source)
{
    return m_smilView->setSource(source);
}

SmilElement* MMSWidget::rootElement() const
{
    return m_smilView->rootElement();
}

void MMSWidget::play()
{
    setDisplayMode(Playback);
    m_smilView->play();
}

void MMSWidget::reset()
{
    setDisplayMode(Loading);
    m_smilView->reset();
}

void MMSWidget::hideEvent(QHideEvent*)
{
    reset();
}

static bool smilPartMatch(const QString identifier, const QMailMessagePart& part)
{
    // See if the identifer is a Content-ID marker
    QString id(identifier);
    if (id.toLower().startsWith("cid:"))
        id.remove(0, 4);

    return ((part.contentID() == id) || (part.displayName() == id) || (part.contentLocation() == id));
}

class TransferRequest : public QPair<SmilDataSource*,QString>
{
public:
    TransferRequest()
    :
    QPair<SmilDataSource*,QString>()
    {
        first = 0;
        second = QString();
    }

    bool isEmpty() const
    {
        return first == 0 && second.isEmpty();
    }
};

class WorkerThread : public QThread
{
    Q_OBJECT
public:
    WorkerThread(SmilDataLoader* parent);

    void stop();

signals:
    void loaded(SmilDataSource* s);

protected:
    void run();

private:
    void process(TransferRequest r);

private:
    SmilDataLoader* const loader();

private:
    volatile bool m_stopped;
};

class SmilDataLoader : public QObject
{
    Q_OBJECT
public:
    SmilDataLoader(QObject* parent = 0);
    ~SmilDataLoader();

    void setSource(const QMailMessage* source);
    const QMailMessage* source() const;

    void reset();
    void load();

    void requestTransfer(SmilDataSource* source, const QString& sourceString);
    void cancelTransfer(SmilDataSource* source, const QString& sourceString);

    bool isFinished() const;

signals:
    void started();
    void loaded(const QString& source);
    void finished();

private:
    TransferRequest popRequest();
    void pushRequest(TransferRequest r);
    void pushResult(SmilDataSource* source, QIODevice* device);
    QIODevice* result(SmilDataSource* source);
    int pendingTransfers();

private slots:
    void dataLoaded(SmilDataSource* s);

private:
    const QMailMessage* m_source;
    QQueue<TransferRequest> m_transferQueue;
    QMap<SmilDataSource*,QIODevice*> m_completedTransfers;
    WorkerThread* m_worker;
    QMutex m_lock;

    friend class WorkerThread;
};

WorkerThread::WorkerThread(SmilDataLoader* parent)
:
    QThread(parent),
    m_stopped(false)
{
}

void WorkerThread::run()
{
    m_stopped = false;
    while(!m_stopped && !loader()->pendingTransfers() ==  0)
    {
        TransferRequest r = loader()->popRequest();
        if(r.isEmpty())
            continue;

        process(r);

    }
    stop();
}

void WorkerThread::process(TransferRequest r)
{
    SmilDataSource* dataSource = r.first;
    const QString sourceString = r.second;

    for ( uint i = 0; i < loader()->m_source->partCount(); i++ ) {
        const QMailMessagePart &part = loader()->m_source->partAt( i );
        if (smilPartMatch(sourceString, part)) {
            dataSource->setMimeType(part.contentType().content());

            const QString filename(part.attachmentPath());
            if (filename.isEmpty()) {
                QBuffer *data = new QBuffer();
                data->setData(part.body().data(QMailMessageBody::Decoded));
                data->open(QIODevice::ReadOnly);
                loader()->pushResult(dataSource,data);
                emit loaded(dataSource);
            } else {
                QFile *file = new QFile(filename);
                file->open(QIODevice::ReadOnly);
                loader()->pushResult(dataSource,file);
                emit loaded(dataSource);
            }
            break;
        }
    }
}

void WorkerThread::stop()
{
    m_stopped = true;
}

SmilDataLoader* const WorkerThread::loader()
{
    return static_cast<SmilDataLoader*>(parent());
}

SmilDataLoader::SmilDataLoader(QObject* parent)
:
    QObject(parent),
    m_source(0),
    m_worker(new WorkerThread(this))
{

    connect(m_worker,SIGNAL(started()),this,SIGNAL(started()));
    connect(m_worker,SIGNAL(loaded(SmilDataSource*)),this,SLOT(dataLoaded(SmilDataSource*)));
    connect(m_worker,SIGNAL(finished()),this,SIGNAL(finished()));
}

SmilDataLoader::~SmilDataLoader()
{
    reset();
    m_worker->deleteLater();
    m_worker = 0;
}

void SmilDataLoader::setSource(const QMailMessage* source)
{
    reset();
    m_source = source;
}

const QMailMessage* SmilDataLoader::source() const
{
    return m_source;
}

void SmilDataLoader::reset()
{
    m_worker->stop();
    m_worker->wait();
    m_source = 0;
    foreach(QIODevice* d, m_completedTransfers)
        d->deleteLater();
    m_completedTransfers.clear();
    m_transferQueue.clear();
}

void SmilDataLoader::load()
{
    m_worker->start();
}

void SmilDataLoader::requestTransfer(SmilDataSource* smilsource, const QString& sourceString)
{
    TransferRequest r;
    r.first = smilsource;
    r.second = sourceString;
    pushRequest(r);
}

void SmilDataLoader::cancelTransfer(SmilDataSource* smilSource, const QString& sourceString)
{
    Q_UNUSED(sourceString);
    if (m_completedTransfers.contains(smilSource))
        m_completedTransfers.take(smilSource)->deleteLater();
}

bool SmilDataLoader::isFinished() const
{
    return m_worker->isFinished();
}

TransferRequest SmilDataLoader::popRequest()
{
    QMutexLocker lock(&m_lock);
    if(m_transferQueue.isEmpty())
        return TransferRequest();
    else
        return m_transferQueue.dequeue();
}

void SmilDataLoader::pushRequest(TransferRequest r)
{
    QMutexLocker lock(&m_lock);
    if(!m_transferQueue.contains(r))
        m_transferQueue.enqueue(r);
}

void SmilDataLoader::pushResult(SmilDataSource* data, QIODevice* device)
{
    QMutexLocker lock(&m_lock);
    m_completedTransfers[data] = device;
}

QIODevice* SmilDataLoader::result(SmilDataSource* s)
{
    QMutexLocker lock(&m_lock);
    return m_completedTransfers[s];
}

int SmilDataLoader::pendingTransfers()
{
    QMutexLocker lock(&m_lock);
    return m_transferQueue.count();
}

void SmilDataLoader::dataLoaded(SmilDataSource* s)
{
    //only set the device from the main thread
    //since the smil library has timers etc that can only be started from it
    QIODevice* d = result(s);
    if(d) s->setDevice(d);
}

SmilViewer::SmilViewer( QWidget* parent )
    : QMailViewerInterface( parent ),
      menuKey( QSoftMenuBar::menuKey() ),
      mail( 0 ),
      m_loader(new SmilDataLoader(this)),
      m_mmsWidget(new MMSWidget())
{
    m_mmsWidget->setGeometry(parent->rect());
    m_mmsWidget->setFocusPolicy(Qt::StrongFocus);

    connect(m_mmsWidget, SIGNAL(transferRequested(SmilDataSource*,QString)),
            this, SLOT(requestTransfer(SmilDataSource*,QString)));
    connect(m_mmsWidget, SIGNAL(transferCancelled(SmilDataSource*,QString)),
            this, SLOT(cancelTransfer(SmilDataSource*,QString)));

    connect(m_mmsWidget, SIGNAL(finished()),
            this, SIGNAL(finished()));

    widget()->installEventFilter(this);

    connect(m_loader,SIGNAL(started()),this,SLOT(loadingStarted()));
    connect(m_loader,SIGNAL(finished()),this,SLOT(loadingFinished()));
}

SmilViewer::~SmilViewer()
{
    m_loader->reset();
    m_mmsWidget->reset();
}

QWidget* SmilViewer::widget() const
{
    return m_mmsWidget;
}

// This logic is replicated in the MMS composer...
static QString smilStartMarker(const QMailMessage& mail)
{
    QMailMessageContentType type(mail.headerField("X-qtopia-internal-original-content-type"));
    if (type.isNull()) {
        type = QMailMessageContentType(mail.headerField("Content-Type"));
    }
    if (!type.isNull()) {
        QString startElement = type.parameter("start");
        if (!startElement.isEmpty())
            return startElement;
    }

    return QString("<presentation-part>");
}

static uint smilStartIndex(const QMailMessage& mail)
{
    QString startMarker(smilStartMarker(mail));

    for (uint i = 0; i < mail.partCount(); ++i)
        if (mail.partAt(i).contentID() == startMarker)
            return i;

    return 0;
}

bool SmilViewer::setMessage(const QMailMessage& msg)
{
    mail = &msg;

    m_loader->setSource(mail);

    uint smilPartIndex = smilStartIndex(*mail);

    const QMailMessagePart &part = mail->partAt( smilPartIndex );
    QString smil(part.body().data());

      if(m_mmsWidget->setSource(smil) && m_mmsWidget->rootElement()) {
        tweakView();
        m_loader->load();
        return true;
    }

    return false;
}

bool SmilViewer::eventFilter(QObject* watched, QEvent* event)
{
    if (event->type() == QEvent::KeyPress) {
        if (QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event)) {
            if (keyEvent->key() == Qt::Key_Back) {
                emit finished();
                return true;
              } else if (watched == m_mmsWidget) {
                if (keyEvent->key() == Qt::Key_Select) {
                    if(m_loader->isFinished())
                        advanceSlide();
                    return true;
                } else if (keyEvent->key() == menuKey) {
                    // The menu should not be activated while we're active
                    return true;
                }
            }
        }
    }

    return false;
}

void SmilViewer::clear()
{
    m_mmsWidget->reset();

    mail = 0;
    m_loader->reset();
}

void SmilViewer::tweakView()
{
    // Try to make sure the layout works on our display
    SmilElement *layout = m_mmsWidget->rootElement()->findChild(QString(), "layout", true);
    if (!layout)
        return;

    QRect rl = m_mmsWidget->rect();
    SmilElement *rootLayout = layout->findChild(QString(), "root-layout");
    if (rootLayout) {
        if (rootLayout->rect().width() > m_mmsWidget->width() ||
                rootLayout->rect().height() > m_mmsWidget->height()) {
            rootLayout->setRect(QRect(0, 0, m_mmsWidget->width(), m_mmsWidget->height()));

        }
        rl = rootLayout->rect();
    }

    SmilElement *imageLayout = layout->findChild("Image", "region");
    if (!imageLayout)
        imageLayout = layout->findChild("image", "region");

    SmilElement *textLayout = layout->findChild("Text", "region");
    if (!textLayout)
        textLayout = layout->findChild("text", "region");

    if (imageLayout && textLayout) {
        QRect il = imageLayout->rect();
        QRect tl = textLayout->rect();
        if (il.bottom() > tl.top() ||
            il.right() > rl.right() ||
            tl.right() > rl.right() ||
            il.bottom() > rl.bottom() ||
            tl.bottom() > rl.bottom()) {
            // Not going to fit - use our preferred sizes.
            il = tl = rl;
            il.setBottom(il.top() + rl.height() * 2 / 3);
            tl.setTop(il.bottom() + 1);
            tl.setHeight(rl.height() - il.height());
            imageLayout->setRect(il);
            textLayout->setRect(tl);
        }
    }
}

void SmilViewer::advanceSlide()
{
    // Try to advance to the next slide
    SmilElement* smil = m_mmsWidget->rootElement();
    if (!smil)
        return;

    SmilElement *body = smil->findChild(QString(), "body", true);
    if (!body)
        return;

    SmilElementList::ConstIterator it;
    for (it = body->children().begin(); it != body->children().end(); ++it) {
        SmilElement *e = *it;
        if (e->name() == "par") {
            if (e->state() == SmilElement::Active) {
                // This should be the current active slide
                SmilTimingAttribute *at = static_cast<SmilTimingAttribute*>(e->moduleAttribute("Timing"));
                Duration d(at->startTime.elapsed());    // i.e. end now.
                e->setCurrentEnd(d);
                break;
            }
        }
    }
}

void SmilViewer::requestTransfer(SmilDataSource* dataSource, const QString &src)
{
    m_loader->requestTransfer(dataSource,src);
}

void SmilViewer::cancelTransfer(SmilDataSource *dataSource, const QString &src)
{
    m_loader->cancelTransfer(dataSource,src);
}

void SmilViewer::loadingStarted()
{
    m_mmsWidget->setDisplayMode(MMSWidget::Loading);
}

void SmilViewer::loadingFinished()
{
    m_mmsWidget->play();
}

QTOPIA_EXPORT_PLUGIN( SmilViewerPlugin )

SmilViewerPlugin::SmilViewerPlugin()
    : QMailViewerPlugin()
{
}

QString SmilViewerPlugin::key() const
{
    return "SmilViewer";
}

bool SmilViewerPlugin::isSupported(QMailMessage::ContentType type, QMailViewerFactory::PresentationType pres) const
{
    if ((pres != QMailViewerFactory::AnyPresentation) && (pres != QMailViewerFactory::StandardPresentation))
        return false;

    return (type == QMailMessage::SmilContent);
}

QMailViewerInterface *SmilViewerPlugin::create(QWidget *parent)
{
    return new SmilViewer(parent);
}

#include <smilviewer.moc>

