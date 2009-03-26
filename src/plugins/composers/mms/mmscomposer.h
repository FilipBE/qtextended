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

#ifndef MMSCOMPOSER_H
#define MMSCOMPOSER_H

#include <qcontent.h>
#include <qwidget.h>
#include <qlist.h>
#include <qlabel.h>
#include <qtextedit.h>
#include <qpixmap.h>
#include <qpushbutton.h>
#include <qmailcomposer.h>
#include <qmailcomposerplugin.h>
#include <QQueue>

class QAction;
class QMenu;
class QStackedWidget;
class QEvent;
class QKeyEvent;
class QPaintEvent;
class QShowEvent;
class QMouseEvent;
class QStackedWidget;
class MMSComposer;
class MMSComposerInterface;
class DetailsPage;
class QTemporaryFile;

class MMSSlideImage : public QLabel
{
    Q_OBJECT
public:
    MMSSlideImage(QWidget* parent);
    ~MMSSlideImage();

    QRect contentsRect() const;

    QSize sizeHint() const;

    void setImage( const QContent &image );
    void setImage( const QPixmap &image );
    QImage image() const;

    bool isEmpty() const;

    QContent& document();

    QString mimeType() const;

    quint64 numBytes() const;

signals:
    void clicked();
    void leftPressed(); // for keypad mode only
    void rightPressed(); // ""
    void aboutToChange(bool& ok, quint64 sizeDelta);
    void changed();

public:
    static void loadImages(const QSize& maxSize);

public slots:
    void select();

protected:
    void resizeEvent( QResizeEvent *event );
    void mousePressEvent( QMouseEvent *event );
    void mouseReleaseEvent( QMouseEvent *event );
    void keyPressEvent( QKeyEvent *event );
    void paintEvent( QPaintEvent *event );

    void loadImage(const QSize& explicitSize = QSize());

    QPixmap scale( const QPixmap &src ) const;

private:
    static QQueue<MMSSlideImage*>& loadQueue();

private:
    QPixmap m_image;
    QContent m_content;
    QSize m_contentSize;
    bool m_pressed;
};

class MMSSlideVideo : public QWidget
{
    Q_OBJECT
public:
    MMSSlideVideo(QWidget* parent = 0);
    ~MMSSlideVideo();

    bool isEmpty() const;

    QContent document() const;

    void setVideo(const QContent& c);
    void setVideo( const QByteArray&, const QString& );
    QByteArray video() const;

    QString mimeType() const;

    quint64 numBytes() const;

public slots:
    void select();

protected:
    void keyPressEvent(QKeyEvent* e);

signals:
    void leftPressed();
    void rightPressed();
    void clicked();
    void aboutToChange(bool& ok,quint64 sizeDelta);
    void changed();

private:
    QContent m_videoMedia;
    QTemporaryFile* m_tempFile;
    QPushButton* m_videoWidget;

};

class MMSSlideText : public QTextEdit
{
    Q_OBJECT
public:
    MMSSlideText(QWidget *parent);

    void setText( const QString &txt );
    QString text() const;

    bool isEmpty() const;

    QSize sizeHint() const;

    const QString defaultText;

    QRect contentsRect() const;

    quint64 numBytes() const;

signals:
    void leftPressed(); // for keypad mode only
    void rightPressed(); // ""
    void aboutToChange(bool& ok, quint64 sizeDelta);
    void changed();

protected:
    virtual void mousePressEvent ( QMouseEvent * e );
    void keyPressEvent( QKeyEvent *e );
    bool event( QEvent *e );

private:
    bool m_hasFocus;
};

class MMSSlideAudio : public QPushButton
{
    Q_OBJECT
public:
    MMSSlideAudio(QWidget *parent);

    void setAudio( const QContent &fn );
    void setAudio( const QByteArray &, const QString & );
    QByteArray audio() const;

    QString mimeType() const;

    bool isEmpty() const;

    QContent& document();

    quint64 numBytes() const;

public slots:
    void select();

protected:
    void keyPressEvent( QKeyEvent *e );

signals:
    void leftPressed(); // for keypad mode only
    void rightPressed(); // ""
    void aboutToChange(bool& ok, quint64 sizeDelta);
    void changed();

private:
    QContent audioContent;
    mutable QByteArray audioData;
    QString audioName;
    mutable QString audioType;
};

struct MMSSmilPart
{
    MMSSmilPart() : duration( 5000 ) {}
    int duration;
    QString image;
    QString text;
    QString audio;
    QString video;
};

struct MMSSmil
{
    QColor bgColor;
    QColor fgColor;
    QList<MMSSmilPart> parts;
};

class MMSSlide : public QWidget
{
    friend class MMSComposerInterface;
    friend class MMSComposer;

    Q_OBJECT
public:
    MMSSlide(QWidget *parent = 0);

    MMSSlideImage *imageContent() const;
    MMSSlideText *textContent() const;
    MMSSlideAudio *audioContent() const;
    MMSSlideVideo* videoContent() const;

    void setDuration( int t );
    int duration() const;

    quint64 numBytes() const;

    bool isEmpty() const;

signals:
    void leftPressed();
    void rightPressed();
    void durationChanged(int);
    void aboutToChange(bool& ok, quint64 sizeDelta);
    void changed();

private slots:
    void selectMedia();
    void mediaChanged();

protected:
    void showEvent(QShowEvent* e);

private:
    MMSSlideImage *m_imageContent;
    MMSSlideText *m_textContent;
    MMSSlideAudio *m_audioContent;
    int m_duration;
    QStackedWidget* m_mediaStack;
    QPushButton* m_noMediaButton;
    MMSSlideVideo* m_videoContent;
};

class MMSComposerInterface : public QMailComposerInterface
{
    Q_OBJECT

public:
    MMSComposerInterface(QWidget *parent = 0);
    virtual ~MMSComposerInterface();

    bool isEmpty() const;
    QMailMessage message() const;

    int currentSlide() const;
    uint slideCount() const;
    MMSSlide *slide( uint slide ) const;
    QRect contentsRect() const;

    void setDefaultAccount(const QMailAccountId& id);
    void setTo(const QString& toAddress);
    void setFrom(const QString& fromAddress);
    void setCc(const QString& ccAddress);
    void setBcc(const QString& bccAddress);
    void setSubject(const QString& subject);
    QString from() const;
    QString to() const;
    QString cc() const;
    QString bcc() const;
    bool isReadyToSend() const;
    bool isDetailsOnlyMode() const;
    void setDetailsOnlyMode(bool val);
    QString contextTitle() const;
    QMailAccount fromAccount() const;

    quint64 numBytes() const;

signals:
    void currentChanged( uint );

public slots:
    void setMessage( const QMailMessage &mail );
    void clear();
    virtual void attach( const QContent &lnk, QMailMessage::AttachmentsAction action = QMailMessage::LinkToAttachments );

    void addSlide();
    void addSlide( int a_slide );
    void setCurrentSlide( int slide );
    void setBackgroundColor( const QColor &col );
    void setTextColor( const QColor &col );
    QColor backgroundColor() const;
    void removeSlide();
    void removeSlide( int a_slide );
    void updateLabels();
    void nextSlide();
    void previousSlide();
    void reply(const QMailMessage& source, int action);

protected:
    void keyPressEvent(QKeyEvent *);

protected slots:
    void slideOptions();
    void elementChanged();
    void detailsPage();
    void composePage();
    void slideAboutToChange(bool& ok, quint64 sizeDelta);

private:
    void init();
    MMSSmil parseSmil( const QString &doc );
    void setContext(const QString& title);

private:
    QStackedWidget* m_widgetStack;
    QWidget* m_composerWidget;
    DetailsPage* m_detailsWidget;
    QLabel *m_slideLabel, *m_sizeLabel;
    QStackedWidget *m_slideStack;
    int m_curSlide;
    QList<MMSSlide*> m_slides;
    bool m_internalUpdate;
    QColor m_textColor, m_backgroundColor;
    QAction* m_removeSlide;
    QAction* m_nextSlide;
    QAction* m_previousSlide;
    QString m_title;
};

class MMSComposerPlugin : public QMailComposerPlugin
{
    Q_OBJECT

public:
    MMSComposerPlugin();

    QMailComposerInterface* create( QWidget* parent );
};

#endif
