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

#include <QObject>
#include <QValueSpaceItem>
#include <QStringList>
#include "bindings.h"
#include <QSet>
#include <QDebug>
#include <QtopiaIpcEnvelope>
#include <QTimer>
#include <QTimeString>
#include <QListView>
#include <QFavoriteServicesModel>
#include <QtopiaServiceDescription>

// ###
#include <QVBoxLayout>
#include <QDesktopWidget>
#include <QApplication>
#include <QBrush>
#include <QColor>
#include <QPalette>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QDebug>
#include <QGraphicsSceneMouseEvent>
#include <QTimeLine>
#include <QFileInfo>
#include <QSvgRenderer>
#include <QPicture>

class TouchScreenLockDialog : public QDialog
{
    Q_OBJECT
public:
    TouchScreenLockDialog(QWidget *parent = 0, Qt::WFlags fl = 0);

private:
    QPixmap generatePixmap(const QString &filename, int width, int height) const;

private:
    QGraphicsScene *scene;
};


Q_DECLARE_METATYPE(QList<QString>);
Q_DECLARE_METATYPE(QVariant);

VSIWrapper::VSIWrapper(const QString& vsroot, QObject* parent)
    : QObject(parent)
    , m_item(vsroot, this)
    , m_root(vsroot)
{
    connect(&m_item, SIGNAL(contentsChanged()), this, SIGNAL(contentsChanged()));
}

ServicesWrapper::ServicesWrapper(QObject *parent)
    : QObject(parent)
{
    QStringList sl = QtopiaService::list();

    foreach (QString s, sl) {
        // Special case a few of these
        if (s == "Contacts")
            new ContactServiceWrapper(this);
#if defined(QTOPIA_TELEPHONY)
        else if (s == "CallHistory")
            new CallHistoryServiceWrapper(this);
        else if (s == "Dialer")
            new DialerServiceWrapper(this);
#endif
        else if (s == "Calendar")
            new CalendarServiceWrapper(this);
        else if (s == "Messages")
            new MessagesServiceWrapper(this);
        else
            new ServiceWrapper(this, s);
    }
    setObjectName("services");
}

ServiceWrapper::ServiceWrapper(QObject *parent, const QString& name)
    : QObject(parent)
    , m_name(name)
{
    setObjectName(name);
}

void ServiceWrapper::raise()
{
    QtopiaIpcEnvelope envelope(QString("QPE/Application/%1").arg(QtopiaService::app(m_name)), "raise()");
}


OccurrenceModelWrapper::OccurrenceModelWrapper(QObject *parent, const QDateTime& start, int count)
    : QOccurrenceModel(start, count, parent)
{
    refresh();
}

OccurrenceModelWrapper::OccurrenceModelWrapper(QObject *parent, const QDateTime& start, const QDateTime& end)
    : QOccurrenceModel(start, end, parent)
{
    refresh();
}

QVariantMap OccurrenceModelWrapper::appointment(int index)
{
    return wrapOccurrence(QOccurrenceModel::occurrence(index));
}
QVariantMap OccurrenceModelWrapper::appointment(const QString& uid, const QDate& date)
{
    QDate d(date);
    if (date.isNull())
        d = QDate::currentDate();
    return wrapOccurrence(QOccurrenceModel::occurrence(QUniqueId(uid), d));
}

QVariantMap OccurrenceModelWrapper::wrapOccurrence(const QOccurrence& a)
{
    QVariantMap ret;

    ret.insert("description", a.description());
    ret.insert("start", a.start().toUTC());
    ret.insert("startInCurrentTZ", a.startInCurrentTZ());
    ret.insert("end", a.end());
    ret.insert("endInCurrentTZ", a.endInCurrentTZ());
    ret.insert("location", a.location());
    ret.insert("isAllDay", a.appointment().isAllDay());
    ret.insert("uid", a.uid().toString());

    return ret;
}

QtExtendedWrapper::QtExtendedWrapper(QObject *parent, QWidget* widget)
    : QObject(parent), m_widget(widget), m_lockDialog(0)
{
    setObjectName("qt-extended");

    new ServicesWrapper(this);

    // Simulate Melbourne weather
    QTimer *weather = new QTimer(this);
    connect(weather, SIGNAL(timeout()), this, SIGNAL(weatherChanged()));
    weather->start(20000);
}

QObject* QtExtendedWrapper::valuespace(const QString& path)
{
    // See if we have this path already.
    VSIWrapper *ret = _vsis.value(path);
    if (!ret) {
        ret = new VSIWrapper(path, this);
        _vsis.insert(path, ret);
    }
    return ret;
}

QVariantMap QtExtendedWrapper::currentLocation()
{
    QVariantMap ret;
    ret.insert("suburb", QString("Barcelona"));
    ret.insert("postcode", QString("08015"));
    ret.insert("country", QString("Spain"));
    ret.insert("state", QString("Catalonia"));
    ret.insert("latitude", -33.45);
    ret.insert("longitude", 120.44);
    return ret;
}

// ### This will be replaced by a web service
QVariantMap QtExtendedWrapper::weather(QVariantMap )
{
    QVariantMap ret;
    ret.insert("high", 15);
    ret.insert("low", 5);
    ret.insert("now", 14.5);
    ret.insert("wind", 32);
    ret.insert("visibility", 5000);

    /* Conditions is one of */
    static const char *weathers[] = {
//        "unknown",
        "fine",
        "clear",
        "sunny",
        "hot",
//        "windy",
        "partlycloudy",
        "cloudy",
        "drizzle",
        "showers",
        "rain",
        "heavyrain",
        "stormy",
//        "cyclone",
//        "hurricane",
//        "smoky",
        "lightsnow",
        "snow",
        "heavysnow",
        "blizzard",
//        "icy",
        "hail",
        "sleet",
        "rainandsnow",
//        "catsanddogs"
    };

    int my_rnd = qrand() % (sizeof(weathers)/sizeof(weathers[0]));
    ret.insert("conditions", QString(weathers[my_rnd]));
    return ret;
}

void QtExtendedWrapper::showLauncher()
{
    QtopiaIpcEnvelope e("QPE/System", "showPhoneLauncher()");
}


void QtExtendedWrapper::showFavorites()
{
    //This function was not tested, as it is never called (at time of rewriting)
    QDialog dialog(m_widget);
    QFavoriteServicesModel *model = new QFavoriteServicesModel(&dialog);
    QListView *list = new QListView(&dialog);
    list->setModel(model);
    connect(list, SIGNAL(activated(QModelIndex)),
            &dialog, SLOT(accept()));
    if(dialog.exec()){
        QtopiaServiceDescription desc = model->description(list->currentIndex());
        if(!desc.isNull())
            desc.request().send();
    }
}

QObject* QtExtendedWrapper::appointments(const QDateTime& start, int count)
{
    return new OccurrenceModelWrapper(this, start, count);
}

QObject* QtExtendedWrapper::appointments(const QDateTime& start, const QDateTime& end)
{
    return new OccurrenceModelWrapper(this, start, end);
}

QString QtExtendedWrapper::formatDate(const QDate& date, const QDate& today)
{
    QDate realToday(today);
    if (!today.isValid())
        realToday = QDate::currentDate();

    /* We want either "Mon, 26 Feb 2008" or "Mon, 26 Feb" if the year is the current year */
    if (date.year() == realToday.year())
        return tr("%1, %2", "[Mon], [26 Feb]").arg(QTimeString::localDayOfWeek(date), QTimeString::localMD(date, QTimeString::Short));
    else
        return tr("%1, %2", "[Mon], [26 Feb 2007]").arg(QTimeString::localDayOfWeek(date), QTimeString::localYMD(date, QTimeString::Medium));
}

QString QtExtendedWrapper::formatDateTime(const QDateTime& dt, const QDate& today)
{
    QDate realToday(today);
    if (!today.isValid())
        realToday = QDate::currentDate();

    QDateTime ldt = dt.toLocalTime();

    QString startdate(ldt.date() == realToday ? tr("Today") : formatDate(ldt.date(), realToday));
    return tr("%1, %2", "[Mon Feb 26 2007], [3:00pm] to [6:00pm]")
            .arg(startdate,
            QTimeString::localHM(ldt.time()));
}

QString QtExtendedWrapper::tr(const QString& context, const QString& msg, const QString& comment, int n)
{
    // ### may need to use a different textcodec
    return QApplication::translate(context.toLatin1().constData(), msg.toLatin1().constData(), comment.toLatin1().constData(), QCoreApplication::CodecForTr, n);
}

void QtExtendedWrapper::setCaption(const QString& msg)
{
    // ### This is not a good api
    if (m_widget && m_widget->window())
        m_widget->window()->setWindowTitle(msg);
}

void QtExtendedWrapper::lockDevice()
{
    if (!m_lockDialog)
        m_lockDialog = new TouchScreenLockDialog(m_widget);
    m_lockDialog->show();
}

void QtExtendedWrapper::unlockDevice()
{
    if (m_lockDialog)
        m_lockDialog->close();
}

QObject *createBindingHierarchy(QObject *parent, QWidget* parentWidget)
{
    QObject *com = new QObject(parent);
    com->setObjectName("com");

    new QtExtendedWrapper(com, parentWidget);

    // And register a few metatypes
    qRegisterMetaType<QList<QString> >();
    qRegisterMetaType<QVariant>();

    return com;
}


// ### This code comes from the server :/
class KeyItem : public QObject, public QGraphicsPixmapItem
{
    Q_OBJECT

public:
    KeyItem(const QPixmap &pixmap, QGraphicsItem *parent = 0)
      : QObject(0), QGraphicsPixmapItem(pixmap, parent)
    {
        setShapeMode(QGraphicsPixmapItem::BoundingRectShape);
        b_unlocked = false;
        timeLine = new QTimeLine(500, this);
        timeLine->setFrameRange(0, 100);
        connect(timeLine, SIGNAL(frameChanged(int)), this, SLOT(move(int)));
    }

    void mouseMoveEvent(QGraphicsSceneMouseEvent *event)
    {
        qreal newPos = event->scenePos().y() - event->lastPos().y();

        qreal y1 = scene()->sceneRect().height() * 0.18;
        qreal y2 = scene()->sceneRect().height() * 0.2;

        if (newPos > y1 && newPos < initialY) {
            setPos(pos().x(), newPos);
            b_unlocked = false;
        }
        if (newPos <= y2) {
            setPos(pos().x(), y1);
            b_unlocked = true;
        }
    }

    void setInitialPos(qreal x, qreal y)
    {
        initialY = y;
        setPos(x, y);
    }

    void mousePressEvent(QGraphicsSceneMouseEvent *)
    {
        timeLine->stop();
        timeLine->setStartFrame(0);
    }

    void mouseReleaseEvent(QGraphicsSceneMouseEvent *)
    {
        if (b_unlocked) {
            emit unlocked();
            return;
        }
        d = (initialY - scenePos().y()) / 100.0;
        ini = scenePos().y();
        timeLine->start();
    }

protected Q_SLOTS:
    void move(int i)
    {
        setPos(pos().x(), ini + (i * d));
    }

Q_SIGNALS:
    void unlocked();

private:
    bool b_unlocked;
    QTimeLine *timeLine;
    qreal d;
    qreal ini;
    qreal initialY;
};

TouchScreenLockDialog::TouchScreenLockDialog(QWidget *parent, Qt::WFlags flags)
  : QDialog(parent, flags), scene(0)
{
    //set palette before setting window state so we get correct transparency
    QPalette p = palette();
    p.setBrush(QPalette::Window, QBrush(QColor(0,0,0,0)));
    setPalette(p);

    setWindowFlags(Qt::Dialog | Qt::WindowStaysOnTopHint);
    setWindowState(Qt::WindowFullScreen);
    setAttribute(Qt::WA_DeleteOnClose);

    QVBoxLayout *vb = new QVBoxLayout;
    vb->setMargin(0);
    QGraphicsView *v = new QGraphicsView(this);
    v->setFrameStyle(QFrame::NoFrame);
    v->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    v->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scene = new QGraphicsScene(this);
    QDesktopWidget *desktop = QApplication::desktop();
    scene->setSceneRect(desktop->screenGeometry(desktop->primaryScreen()));
    v->setScene(scene);
    vb->addWidget(v);
    setLayout(vb);

    int kw = (int)(scene->sceneRect().width() * 0.14);
    int kh = (int)(scene->sceneRect().height() * 0.21);
    KeyItem *key = new KeyItem(generatePixmap(":image/qpe/Key", kw, kh), 0);
    QObject::connect (key, SIGNAL(unlocked()), this, SLOT(close()));

    int lw = (int)(scene->sceneRect().width() * 0.27);
    int lh = (int)(scene->sceneRect().height() * 0.29);
    QGraphicsPixmapItem *lock = new QGraphicsPixmapItem(generatePixmap(":image/qpe/Lock", lw, lh), 0);
    scene->addItem(key);
    scene->addItem(lock);

    int dw = desktop->screenGeometry(desktop->primaryScreen()).width();
    int dh = desktop->screenGeometry(desktop->primaryScreen()).height();

    lock->setPos(dw/2 - lock->boundingRect().width()/2, 0);
    lock->setZValue(2);

    key->setInitialPos(dw/2 - key->boundingRect().width()/2, dh - key->boundingRect().height());
    key->setFlag(QGraphicsItem::ItemIsMovable);
    lock->setZValue(1);

    activateWindow();
    raise();
}

QPixmap TouchScreenLockDialog::generatePixmap(const QString &filename, int width, int height) const
{
    QFileInfo fileInfo(filename);

    if (!fileInfo.exists())
        return QPixmap();

    QImage image(width, height, QImage::Format_ARGB32_Premultiplied);
    image.fill(0);
    QPainter painter(&image);

#ifndef QT_NO_PICTURE
    QPicture picture;
    picture.load(fileInfo.filePath());
    QRect br = picture.boundingRect();
    painter.scale(qreal(width) / br.width(), qreal(height) / br.height());
    painter.drawPicture(0, 0, picture);
#else
    QSvgRenderer renderer(fileInfo.filePath());
    renderer.render(&painter);
#endif
    painter.end();
    return QPixmap::fromImage(image);
}


#include "bindings.moc"
