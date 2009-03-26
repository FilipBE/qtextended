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

#include "webfeed.h"
#include "mediafeed.h"
#include "webliteimg.h"
#include "math.h"
#include <QScreen>
#include <QtopiaApplication>
#include <QPainter>
#include <QBitmap>
#include <QLabel>
#include <QDirectPainter>
#include <QTimer>
#include <QHBoxLayout>
#include <QLinearGradient>
#include <QXmlStreamReader>
#include <QListWidget>
#include <QItemDelegate>
#include <QVBoxLayout>
#include <QProgressBar>
#include <private/homewidgets_p.h>

struct WebFeedWeatherInfo : public WebFeedElement
{
    QString day;
    QDateTime date;
    int low, high;
    QString text;
    int code;
    virtual int typeId() const { return qMetaTypeId<WebFeedWeatherInfo>(); };
};
Q_DECLARE_METATYPE(WebFeedWeatherInfo)

class WebFeedWeatherParser : public WebFeedExtensionParser
{
    virtual void parse (WebFeedElement* parent,
                        QXmlStreamReader & reader)
    {
        if (reader.isStartElement())
        {
            QString name = reader.name().toString();
            if (name == "forecast" || name == "condition")
            {
                WebFeedWeatherInfo* cur = new WebFeedWeatherInfo;
                parent->add(cur);
                QXmlStreamAttributes attr = reader.attributes();
                cur->text = attr.value("text").toString();
                cur->date = QDateTime::fromString(attr.value("date").toString(),"dd MMM yyyy");
                cur->code = attr.value("code").toString().toInt();
                if (name == "forecast")
                {
                    cur->day = attr.value("day").toString();
                    cur->low = attr.value("low").toString().toInt();
                    cur->high = attr.value("high").toString().toInt();
                }
                else
                {
                    cur->day = "Now";
                    cur->high = cur->low = attr.value("temp").toString().toInt();
                }
            }
        }
    }

    virtual QStringList namespaceUris () const
    {
        QStringList l;
        l.append("http://xml.weather.yahoo.com/ns/rss/1.0");

        return l;
    }
};


class WeatherListItem : public QListWidgetItem
{
    public:
        WebFeedWeatherInfo weatherInfo;
        WebLiteImg* imgLoader;

        WeatherListItem (WebFeedWeatherInfo* inf, QListWidget* w = NULL) : QListWidgetItem(w,QListWidgetItem::UserType)
        {
            weatherInfo = *inf;
            imgLoader = new WebLiteImg(w);
            imgLoader->setUrl(QUrl(QString("http://l.yimg.com/us.yimg.com/i/us/we/52/%1.gif").arg(inf->code)));
            QObject::connect (imgLoader, SIGNAL(decoded()), w, SLOT(update()));
            imgLoader->load ();
        }
        virtual ~WeatherListItem ()
        {
            imgLoader->deleteLater();
        }
};

class WeatherListDelegate : public QItemDelegate
{
    Q_OBJECT

    QListWidget* listWdg;
    public:
        WeatherListDelegate(QListWidget* lw) : QItemDelegate(lw),listWdg(lw)
        {
            lw->setItemDelegate(this);
        }
        virtual QSize sizeHint ( const QStyleOptionViewItem & , const QModelIndex & index ) const
        {
            return QSize(QScreen::instance()->width()*0.3,QScreen::instance()->height()*0.3);
        }

    virtual void paint ( QPainter * painter, const QStyleOptionViewItem & opt, const QModelIndex & index ) const
    {
        painter->save();
        painter->setPen(QPen(Qt::white));
        WeatherListItem* item = (WeatherListItem*)listWdg->item(index.row());

        WebFeedWeatherInfo* inf = &item->weatherInfo;
        QFont f = painter->font ();
        QRect r = opt.rect;
        QFontMetrics fm (f);
        painter->drawText(r.left(),r.top(),QScreen::instance()->width()*0.3,fm.height(),Qt::AlignCenter|Qt::AlignBottom,inf->day);
        QImage img = item->imgLoader->image ();

        if (!img.isNull())
        {
            painter->drawPixmap(r.left()+QScreen::instance()->width()*0.06,r.top()+QScreen::instance()->height()*0.06,QPixmap (":image/weblite/weather/weather-bg.png"));
            painter->drawImage(r.left()+QScreen::instance()->width()*0.075,r.top()+QScreen::instance()->height()*0.083,img);

        }
        painter->setPen(QPen(QColor(0xa9cad9)));

        QString text = (inf->low == inf->high) ? QString("%1").arg(inf->low) : QString ("%1-%2").arg(inf->low).arg(inf->high);
        painter->drawText(r.left(),r.top()+QScreen::instance()->height()*0.33,QScreen::instance()->width()*0.3,QScreen::instance()->height()*0.06,Qt::AlignCenter|Qt::AlignBottom,fm.elidedText(inf->text,Qt::ElideRight,90));
        painter->drawText(r.left(),r.top()+QScreen::instance()->height()*0.4,QScreen::instance()->width()*0.4,QScreen::instance()->height()*0.06,Qt::AlignCenter|Qt::AlignBottom,text);

        painter->restore();
    }
};

class WebWeather : public QWidget
{
    Q_OBJECT

    public:

    QPointer<WebFeedLoader> feedLoader;
    QPointer<QListWidget> dataList;
    QPointer<QLabel> dscLbl;
    QPointer<QProgressBar> progressBar;
    WebFeedWeatherParser wparser;
    WebWeather (QWidget* w) : QWidget(w)
    {
        feedLoader = new WebFeedLoader(this);
        feedLoader->installExtension(&wparser);
        connect (feedLoader, SIGNAL(updated()), this, SLOT(updateList()));
        QVBoxLayout* mainLayout = new QVBoxLayout();
        mainLayout->setSpacing(0);
        mainLayout->setMargin(0);
        setWindowTitle(tr("Yahoo! Weather"));

        setLayout(mainLayout);
        mainLayout->setSpacing(10);
//        mainLayout->addStretch(10);
        dataList = new QListWidget();
        dataList->setFrameStyle(QFrame::NoFrame);
        dataList->setFlow(QListWidget::LeftToRight);
        dataList->setAutoFillBackground(false);
        dataList->viewport()->setAutoFillBackground(false);
        dataList->hide ();
        new WeatherListDelegate(dataList);
        dscLbl = new QLabel("Loading... Please Wait");
        dscLbl->setAlignment(Qt::AlignHCenter|Qt::AlignTop);
        mainLayout->addWidget(dscLbl,1);
//        mainLayout->addStretch(10);
        QHBoxLayout* hbl = new QHBoxLayout();
        hbl->addWidget(dataList,Qt::AlignHCenter);
        mainLayout->addLayout(hbl,10);
//        mainLayout->addStretch(1);
    }

    void paintEvent (QPaintEvent*)
    {

        QPainter p (this);
        p.fillRect (rect(), QBrush(QColor(0x44819d)));
        QPainterPath pp;
        pp.addEllipse(QRect(0 - QScreen::instance()->height()*0.125,QScreen::instance()->height()*0.27,QScreen::instance()->width()*1.3,QScreen::instance()->height()*0.83));
        p.fillPath(pp,QBrush(QColor(0x005880)));

    }
    signals:
        void titleChanged (const QString &);
    public slots:


    void updateList ()
    {
        WebFeedChannel* chan = (WebFeedChannel*)feedLoader->root();
        QList<WebFeedItem* > items = chan->items();
        emit titleChanged(chan->title);
        dataList->clear ();
        for (QList<WebFeedItem* >::iterator it = items.begin(); it != items.end(); ++it)
        {
            QString title = (*it)->title;
            QFontMetrics fm (dscLbl->font());
            dscLbl->setText(fm.elidedText(title,Qt::ElideRight,dscLbl->width()-10));
            QList<WebFeedWeatherInfo*> wi = (*it)->elementsByType<WebFeedWeatherInfo>();
            for (QList<WebFeedWeatherInfo*>::iterator wit = wi.begin(); wit != wi.end(); ++wit)
            {
                new WeatherListItem(*wit,dataList);
            }
        }
        dataList->show ();
    }

    void load()
    {
        QSettings sett ("Trolltech","weather");
        QString loc = sett.value("weather/location","94065").toString();
        feedLoader->setUrl (QUrl(QString("http://weather.yahooapis.com/forecastrss?p=%1").arg(loc)));
        feedLoader->load ();
    }

};


class WebWeatherApp : public QWidget
{
    Q_OBJECT
public:
    WebWeatherApp(QWidget* w, Qt::WindowFlags wf) : QWidget(w,wf)
    {
        WebWeather* ww = new WebWeather(NULL);
        ww->show ();
        ww->load ();
        ww->setWindowState(Qt::WindowMaximized);
        setWindowState(Qt::WindowMaximized);
        QVBoxLayout* mainLayout = new QVBoxLayout();
        QHBoxLayout* topLayout = new QHBoxLayout();
        QLabel* lbl = new QLabel(QObject::tr("Yahoo! Weather"));
        lbl->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
        QObject::connect(ww,SIGNAL(titleChanged(QString)),lbl,SLOT(setText(QString)));
        HomeActionButton *closeButton = new HomeActionButton(QObject::tr("Close"),QtopiaHome::Red);
        QObject::connect(closeButton,SIGNAL(clicked()),this,SLOT(close()));
        topLayout->addWidget(closeButton);
        topLayout->addWidget(lbl,1);
        topLayout->setMargin(0);
        topLayout->setSpacing(0);
        mainLayout->setMargin(0);
        mainLayout->setSpacing(0);
        mainLayout->addLayout(topLayout,0);
        mainLayout->addWidget(ww,1);
        setLayout(mainLayout);
    
    }
};

QSXE_APP_KEY
int main (int argc, char** argv)
{
    QtopiaApplication app(argc,argv);
    WebWeatherApp* a = new WebWeatherApp(NULL,0);
    app.setMainWidget(a);
    a->show();
    app.exec();
}


#include "weather.moc"
