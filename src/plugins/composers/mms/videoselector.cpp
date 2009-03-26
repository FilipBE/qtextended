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

#include "videoselector.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QDebug>
#include <QDocumentSelectorDialog>
#include <QtopiaApplication>

class VideoSourceSelector : public QWidget
{
    Q_OBJECT
public:
    VideoSourceSelector(QWidget* parent = 0);
    ~VideoSourceSelector();

    void setContent(const QContent& video);
    QContent content() const;

private slots:
    void selectVideo();
    void recordVideo();
    void removeVideo();

private:
    void init();
    void update();

private:
    QLabel* m_previewLabel;
    QPushButton* m_videoFilesButton;
    QPushButton* m_recordVideoButton;
    QPushButton* m_removeButton;
    QContent m_content;
};

VideoSourceSelector::VideoSourceSelector(QWidget* parent)
:
    QWidget(parent),
    m_videoFilesButton(0),
    m_recordVideoButton(0),
    m_removeButton(0)
{
    init();
}

VideoSourceSelector::~VideoSourceSelector()
{
}

void VideoSourceSelector::init()
{
    QVBoxLayout* l = new QVBoxLayout(this);
    l->setContentsMargins(20,20,20,20);

    m_previewLabel = new QLabel("No Video",this);
    m_previewLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    l->addWidget(m_previewLabel);

    m_videoFilesButton = new QPushButton("Video Files",this);
    connect(m_videoFilesButton,SIGNAL(clicked()),this,SLOT(selectVideo()));
    l->addWidget(m_videoFilesButton);

    m_recordVideoButton = new QPushButton("Record Video",this);
    connect(m_recordVideoButton,SIGNAL(clicked()),this,SLOT(recordVideo()));
    m_recordVideoButton->setVisible(false); //TODO
    l->addWidget(m_recordVideoButton);

    m_removeButton = new QPushButton("Remove",this);
    connect(m_removeButton,SIGNAL(clicked()),this,SLOT(removeVideo()));
    l->addWidget(m_removeButton,1);
    l->addStretch();
}

void VideoSourceSelector::update()
{
    QString previewText = m_content.isValid() ? m_content.name() : tr("No Video");
    m_previewLabel->setText(previewText);
    m_removeButton->setEnabled(m_content.isValid());
}

void VideoSourceSelector::setContent(const QContent& video)
{
    m_content = video;
    update();
}

QContent VideoSourceSelector::content() const
{
    return m_content;
}

void VideoSourceSelector::selectVideo()
{
    QDocumentSelectorDialog docSelector(this);
    docSelector.setFilter( QContentFilter( QContent::Document ) & QContentFilter( QContentFilter::MimeType, "video/*" ) );
    docSelector.disableOptions( QDocumentSelector::ContextMenu );

    if(QtopiaApplication::execDialog(&docSelector) == QDialog::Accepted)
    {
        m_content = docSelector.selectedDocument();
        update();
    }
}

void VideoSourceSelector::recordVideo()
{
    //TODO
}

void VideoSourceSelector::removeVideo()
{
    m_content = QContent();
    update();
}

VideoSourceSelectorDialog::VideoSourceSelectorDialog(QWidget* parent)
:
    QDialog(parent),
    m_videoSourceSelector(0)
{
    init();
}

VideoSourceSelectorDialog::~VideoSourceSelectorDialog()
{
}

void VideoSourceSelectorDialog::init()
{
    m_videoSourceSelector = new VideoSourceSelector(this);
    QVBoxLayout* l = new QVBoxLayout(this);
    l->setSpacing(0);
    l->setContentsMargins(0,0,0,0);

    l->addWidget(m_videoSourceSelector);
}

void VideoSourceSelectorDialog::setContent(const QContent& video)
{
    m_videoSourceSelector->setContent(video);
}

QContent VideoSourceSelectorDialog::content() const
{
    return m_videoSourceSelector->content();
}

#include <videoselector.moc>

