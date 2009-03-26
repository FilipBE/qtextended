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

#include "ringtoneeditor.h"

#include <QAction>
#include <QFileInfo>
#include <QDialog>
#ifdef MEDIA_SERVER
#  include <qsoundcontrol.h>
#  include <QSound>
#elif defined(Q_WS_QWS)
#  include <qsoundqss_qws.h>
#endif
#include <QLayout>
#include <QListWidget>
#include <QPainter>
#include <QTimer>

#include <qtopiaapplication.h>
#include <qcontent.h>
#include <qcontentset.h>
#include <qtranslatablesettings.h>
#include <qdocumentselector.h>
#include <qsoftmenubar.h>

#include <QPhoneProfile>
#include <QPhoneProfileManager>
#include <QFileSystem>

// RingToneSelect
// up to 4 most recently used user ringtones
// get from settings.
// up to ? system ringtones
// get from etc
// other... select from documents.
// play on highlight.

static const int maxCustomRingTones = 4;

class RingToneLink : public QListWidgetItem
{
public:
    RingToneLink(const QContent &d, QListWidget *l)
        : QListWidgetItem(d.name(), l), dl(d) {}

    ~RingToneLink() {}

    QString text() const { return dl.name(); }
    QContent link() const { return dl; }

    int width( const QListWidget *lb ) const;
    int height( const QListWidget *lb ) const;

    void paint( QPainter *p );

private:
    QContent dl;
};

int RingToneLink::width( const QListWidget *lb ) const
{
    return qMax( lb ? lb->viewport()->width() : 0,
        QApplication::globalStrut().width() );
}

int RingToneLink::height( const QListWidget *lb ) const
{
    if( !lb )
        return 0;
    QFontMetrics fm = lb->fontMetrics();
    return qMax( fm.boundingRect( 0, 0, width( lb ), fm.lineSpacing(), 0, text() ).height(),
        QApplication::globalStrut().height() );
}

void RingToneLink::paint( QPainter *p )
{
    const int w = width( listWidget() );
    const int h = height( listWidget() );
    p->drawText( 3, 0, w-3*2, h, 0, text() );
}

RingToneSelect::RingToneSelect(QWidget *parent, bool video)
    : QListWidget(parent), volume(0), volumeSet(false), m_video(video)
# ifdef MEDIA_SERVER
    , scontrol(0)
# elif defined(Q_WS_QWS)
    , sclient(0), soundFinished(false)
# endif
{
    init();
}

void RingToneSelect::init()
{
    setFrameStyle(QFrame::NoFrame);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

# if !defined(MEDIA_SERVER) && defined(Q_WS_QWS)
    sclient = new QWSSoundClient(this);
    connect( sclient, SIGNAL(soundCompleted(int)), this, SLOT(soundCompleted(int)) );
# endif
    stimer = new QTimer(this);
    aNone = false;
    m_currentItem = -1;

    connect(stimer, SIGNAL(timeout()), this, SLOT(hoverTimeout()));
    connect(this, SIGNAL(currentRowChanged(int)), this, SLOT(startHoverTimer(int)));
    connect(this, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(selectItem(QListWidgetItem*)));
    connect(this, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(selectItem(QListWidgetItem*)));
    // load up list of items.
    // first list of most recently accessed items, is from Settings/RingTones.conf

    QSettings c("Trolltech","RingTones"); // No tr
    c.beginGroup("Recent"); // No tr
    int num = c.value("count", 0).toInt(); // No tr
    int i;
    //int regg = 0;
    customCount = 0;
    for (i = 1; i <= num; i++)
    {
        QString linkname = c.value("item" + QString::number(i)).toString(); // No tr
        QContent dl(linkname);
        if (dl.fileKnown()) {
            // need to show either audio or vidio
            if ( !m_video && dl.type().contains("video")
                    || m_video && dl.type().contains("audio") )
                continue;
            customCount++;
            new RingToneLink(dl, this);
        }
    }

    QContentSet rtones;
    rtones.setCriteria(QContentFilter::Location, Qtopia::qtopiaDir() + "etc/SystemRingTones");
    if ( !m_video ) {
        rtones.addCriteria(QContentFilter::MimeType, "audio/*", QContentFilter::And);
    } else {
        rtones.addCriteria(QContentFilter::MimeType, "video/*", QContentFilter::And);
    }

    QContentSetModel model(&rtones);
    for(i = 0; i < model.rowCount(); i++)
    {
        const QContent &dl = model.content(i);
        if (dl.fileKnown())
            new RingToneLink(dl, this);
    }

    otherItem = new QListWidgetItem(tr("Other...", "Select Ringtone from documents"), this);
    noneItem = new QListWidgetItem(tr("<None>", "Set no ringtone selected"), 0);
}

RingToneSelect::~RingToneSelect()
{
    stopSound();
}

void RingToneSelect::showEvent( QShowEvent *e )
{
    QListWidget::showEvent( e );
    update();
}

bool RingToneSelect::event( QEvent *e )
{
    if ( e->type() == QEvent::WindowDeactivate ) {
        stopSound();
    }
    return QListWidget::event( e );
}

void RingToneSelect::setAllowNone(bool a)
{
    if (a == aNone)
        return;

    aNone = a;
    if (aNone) {
        // have to add none item
        insertItem(0, noneItem);
    } else {
        // have to remove none item.
        takeItem(row(noneItem));
    }

}

void RingToneSelect::closeEvent( QCloseEvent *e )
{
    stopSound();
    QListWidget::closeEvent(e);
}


RingToneLink *RingToneSelect::linkItem(int index) const
{
    if (index < 0 || index == (int)count() - 1 || (index == 0 && aNone))
        return 0;
    return (RingToneLink*)item(index);
}

RingToneLink *RingToneSelect::currentLinkItem() const
{
    return linkItem(currentRow());
}

void RingToneSelect::setCurrentTone(const QContent &d)
{
    stopSound();
    if (d.fileName().isEmpty()) {
        if (aNone)
            setCurrentRow(0);
        else
            setCurrentRow(-1);
    } else {
    // for each item, check its link.  same as d, then set as Current Item
        for (int i = aNone ? 1 : 0; i < (int)count()-1; i++) {
            QContent lnk = linkItem(i)->link();
            if (lnk.fileName() == d.fileName()) {

                if (m_currentItem == i) // Already been here, replay
                    stimer->start(200);
                else
                    setCurrentRow(i);

                break;
            }
        }
    }
}

QContent RingToneSelect::currentTone() const
{
    if (currentLinkItem())
       return currentLinkItem()->link();
    return QContent();
}

void RingToneSelect::selectItem(int pos)
{
    if (item(pos) == otherItem) // other,
        addFromDocuments();
    else {
        stopSound();
        if (linkItem(pos)) {
            emit selected(linkItem(pos)->link());

            const QFileSystem *fs = QStorageMetaInfo::instance()->fileSystemOf( linkItem(pos)->link().fileName() );
            if ( fs && fs->isRemovable() ) {
                QMessageBox::warning(this, tr("Removable media"),
                        tr("<qt>\"%1\" is from <b>removable media</b>.<br>"
                            "If the media is removed"
                            " the default ringtone will be played.</qt>", "%1 = file name")
                        .arg(linkItem(pos)->link().name()));
            }

        } else {
            emit selected(QContent());
        }
    }
}

void RingToneSelect::selectItem(QListWidgetItem *item)
{
    if( !item )
        return;
    selectItem( row( item ) );
}

void RingToneSelect::addFromDocuments()
{
    QString dlgcaption = tr("Select Ringtone");

    QDocumentSelectorDialog *dlg = new QDocumentSelectorDialog(this);
    dlg->setModal(true);
    dlg->setWindowTitle(dlgcaption);
    dlg->setWindowState( dlg->windowState() | Qt::WindowMaximized );
    dlg->setSelectPermission( QDrmRights::Play );
    dlg->setMandatoryPermissions( QDrmRights::Play | QDrmRights::Automated );

    QContentFilter audiofilter(QContent::Document);

    if ( !m_video )
        audiofilter &= QContentFilter( QContentFilter::MimeType, QLatin1String( "audio/*" ) );
    else
        audiofilter &= QContentFilter( QContentFilter::MimeType, QLatin1String( "video/*" ) );

    dlg->setFilter( audiofilter );
    dlg->disableOptions( QDocumentSelector::ContextMenu );

    if (QtopiaApplication::execDialog(dlg)) {
        addCustom( dlg->selectedDocument() );
    }
}

void RingToneSelect::addCustom( const QContent &content )
{
    QContent link = content;

    // first find out if custom is in the list already.
    for (int i = 0; i < customCount; i++) {
        QContent lnk = linkItem(aNone ? i+1 : i)->link();
        if (lnk.fileName() == link.fileName()) {
           // ok, just move it to the first of the list.
           RingToneLink *rtli = linkItem(aNone ? i+1 : i);
           takeItem(row(rtli));
           insertItem(aNone ? 1 : 0, rtli);
           setCurrentRow(aNone ? 1 : 0);
           return;
       }
    }

    if (customCount == maxCustomRingTones) {
        // need to remove the last.
        customCount--;
        takeItem(aNone ? customCount+1 : customCount);
    }
    if( !link.isValid() )
        link.commit();
    // done in two steps because it needs to go at the start.
    RingToneLink *rtli = new RingToneLink(link, 0);
    insertItem(aNone ? 1 : 0, rtli);
    setCurrentRow(aNone ? 1 : 0);
    customCount++;

    saveCustom();
}

void RingToneSelect::saveCustom()
{
    QSettings c("Trolltech","RingTones"); // No tr
    c.beginGroup("Recent"); // No tr
    c.setValue("count", customCount); // No tr
    int i;
    for (i = 0; i < customCount; i++) {
        QContent lnk = linkItem(aNone ? i+1 : i)->link();
        c.setValue("item" + QString::number(i+1), lnk.fileName());
    }
}

void RingToneSelect::startHoverTimer(int selectedItem)
{
    if (m_currentItem != selectedItem)
    {
        stimer->stop();

        stopSound();

        m_currentItem = selectedItem;

        if (m_currentItem != int(count()) - 1)
        {
            stimer->start(200);
        }
    }
}

void RingToneSelect::hoverTimeout()
{
    playCurrentSound();
}

void RingToneSelect::playDone()
{
    stopSound();
    stimer->start(0);
}

void RingToneSelect::playCurrentSound()
{
    // shut off - use done()->playDone() to know when to start again
    stimer->stop();

    if (currentLinkItem()) {
#ifdef MEDIA_SERVER
        if (scontrol == NULL)
        {
            scontrol = new QSoundControl( new QSound( currentLinkItem()->link().fileName() ) );

            // uncomment this to play a tone more than once
            //connect(scontrol, SIGNAL(done()), this, SLOT(playDone()));
        }

        scontrol->setPriority( QSoundControl::RingTone );
        if ( !volumeSet ) {
            QPhoneProfileManager manager;
            QPhoneProfile curProfile = manager.activeProfile();
            if( curProfile.volume() < 0 || curProfile.volume() > 5 ) {
                qWarning("BUG: Invalid volume setting for profile %d", curProfile.volume());
                volume = 5;
            } else {
                volume = curProfile.volume();
            }
            volumeSet = true;
        }
        scontrol->setVolume( volume * 20 );

        scontrol->sound()->play();
# elif defined(Q_WS_QWS)
        soundFinished = false;
        sclient->play(0, currentLinkItem()->link().fileName(), volume * 20);
#endif
    }
}

void RingToneSelect::stopSound()
{
#ifdef MEDIA_SERVER
    if (scontrol)
    {
        scontrol->sound()->stop();

        delete scontrol->sound();
        delete scontrol;

        scontrol = NULL;
    }
# elif defined(Q_WS_QWS)
    if ( sclient ) sclient->stop(0);
#endif
}

void RingToneSelect::soundCompleted(int)
{
#ifndef MEDIA_SERVER
    soundFinished = true;
#endif
}

bool RingToneSelect::isFinished()
{
    bool result = true;
#ifdef MEDIA_SERVER
    if (scontrol)
    {
        result = scontrol->sound()->isFinished();
    }
#else
    result = soundFinished;
#endif
    return result;
}

RingToneButton::RingToneButton( QWidget *parent )
    : QPushButton( parent ), rtl(0), dlg(0), aNone(false)
{
    init();
}

RingToneButton::RingToneButton( const QContent &tone, QWidget *parent )
    : QPushButton( parent ), rtl(0), dlg(0), aNone(false)
{
    setTone( tone );
}

void RingToneButton::init( bool video )
{
    setText(tr("<None>", "no ring tone selected"));
    //construction should not be used.
    if ( !video ) // might get hooked up twice.
        connect(this, SIGNAL(clicked()), this, SLOT(selectTone()));
    dlg = new QDialog(this);
    dlg->setModal(true);
    dlg->setWindowState( dlg->windowState() | Qt::WindowMaximized );
    if (!Qtopia::mousePreferred())
        QtopiaApplication::setMenuLike(dlg, true);
    QVBoxLayout *vbl = new QVBoxLayout(dlg);
    vbl->setContentsMargins(0, 0, 0, 0);
    dlg->setWindowTitle(tr("Select Ringtone"));

    rtl = new RingToneSelect(dlg, video);
    connect(rtl, SIGNAL(selected(QContent)),
            this, SIGNAL(selected(QContent)));
    if (!Qtopia::mousePreferred())
        connect(rtl, SIGNAL(selected(QContent)), dlg, SLOT(accept()));
    vbl->addWidget(rtl);
    if (aNone)
        rtl->setAllowNone(true);
}

void RingToneButton::setTone( const QContent &tone )
{
    if( !tone.fileKnown() ) {
        setText(tr("<None>", "no ring tone selected"));
        mTone = QContent();
    } else {
        mTone = tone;
        // limit total string length
        setText( fontMetrics().elidedText( mTone.name(), Qt::ElideRight, sizeHint().width() ) );
    }
}

QContent RingToneButton::tone() const
{
    return mTone;
}

void RingToneButton::setAllowNone(bool b)
{
    if ( !dlg && !rtl )
        init();

    aNone = b;
    if (rtl)
        rtl->setAllowNone(b);
}

bool RingToneButton::allowNone() const
{
    return aNone;
}

void RingToneButton::selectTone()
{
    if ( !dlg && !rtl )
        init();

    rtl->setCurrentTone( mTone );
    if (QtopiaApplication::execDialog(dlg)) {
        setTone( rtl->currentTone() );
    }
    rtl->stopSound();
}

void RingToneButton::setVideoSelector( bool b )
{
    if ( dlg )
        delete dlg;
    init( b );
}
