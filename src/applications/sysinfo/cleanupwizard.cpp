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

#include "cleanupwizard.h"

#include <QSoftMenuBar>
#include <QtopiaApplication>
#include <qtopialog.h>
#include <QtopiaServiceRequest>
#include <QTimeString>
#include <QPushButton>
#include <QHeaderView>
#include <QAbstractItemModel>
#include <QTableView>
#include <QCheckBox>
#include <QSpinBox>
#include <QContentSet>
#include <QProgressBar>
#include <QMessageBox>
#include <QTimer>
#include <QVBoxLayout>
#include <QDateEdit>
#include <QScrollArea>
#include <QWaitWidget>
#include <QKeyEvent>

static QString sizeUnit(int size)
{
    if (size < 1024)
        return QCoreApplication::translate("CleanupWizard", "%n bytes", "", QCoreApplication::CodecForTr, size);

    if (size < 1024 * 1024)
        return QCoreApplication::translate("CleanupWizard", "%1 kB", "kilobyte").arg(size / 1024);

    if (size < 10 * 1024 * 1024)
        return QCoreApplication::translate("CleanupWizard", "%1 MB", "megabyte").arg(double(size) / 1024 / 1024, 0, 'f', 1);

    return QCoreApplication::translate("CleanupWizard", "%1 MB", "megabyte").arg(size / 1024 / 1024);
}

 /*********************************************************************/
//                   CleanupWizard    Methods                         //
/*********************************************************************/

CleanupWizard::CleanupWizard(QWidget* parent) :QWizard(parent)
{
    setObjectName("cleanupWizard"); //set the name to find the html help file

    setPage(preselectionID, new PreselectionPage);
    setPage(documentTypeSelectionID, new DocumentTypeSelectionPage);
    setPage(documentListSelectionID, new DocumentListSelectionPage);
    setPage(mailSelectionID, new MailSelectionPage);
    setPage(eventSelectionID, new EventSelectionPage);
    setPage(finalSummaryID, new FinalSummaryPage);

    setStartId(preselectionID);
    setWindowTitle(tr("Cleanup Wizard"));

    connect(page(documentListSelectionID),SIGNAL(documentsDeleted(int,bool)),
            page(finalSummaryID),SLOT(documentsDeleted(int,bool)));

    // update the buttons layout each time a new page is shown
    connect(this,SIGNAL(currentIdChanged(int)),this,SLOT(updateWizard(int)));
}

void CleanupWizard::updateWizard(int id)
{
    // this method update the wizard objectname in order to display the html help file
    // for each page
    switch(id) {
        case preselectionID:
            setObjectName("cleanupPreselection");
            break;
        case mailSelectionID:
            setObjectName("cleanupMail");
            break;
        case eventSelectionID:
            setObjectName("cleanupEvent");
            break;
        case documentTypeSelectionID:
            setObjectName("cleanupTypeSelection");
            break;
        case documentListSelectionID:
            setObjectName("cleanupListSelection");
            break;
        case finalSummaryID:
            setObjectName("cleanupSummary");
            break;
        default:
            setObjectName("cleanupWizard");
    }

    // QWizard automatically gives focus to the first child of the new page,
    // but it is somehow edit focus instead of navigation focus, so refocus it
    QWizardPage *currPage = page(id);
    if (currPage) {
        QWidget *focusWidget = currPage->focusWidget();
        if (focusWidget)
            focusWidget->setEditFocus(false);
    }
}

 /*********************************************************************/
//                   PreselectionPage   Methods                       //
/*********************************************************************/

PreselectionPage::PreselectionPage(QWidget* parent) : QWizardPage(parent)
{
    QVBoxLayout* vb = new QVBoxLayout(this);
    QLabel* title = new QLabel(tr("What would you like to cleanup?"));
    title->setWordWrap(true);
    vb->addWidget(title);
    QFrame *line = new QFrame( this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken );
    vb->addWidget(line);
    mailCheckBox     = new QCheckBox(tr("Messages"));
    vb->addWidget(mailCheckBox);
    eventCheckBox    = new QCheckBox(tr("Events"));
    vb->addWidget(eventCheckBox);
    documentCheckBox = new QCheckBox(tr("Documents"));
    vb->addWidget(documentCheckBox);
    QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Minimum,
    QSizePolicy::Expanding);
    vb->addItem(spacer);
    registerField("documentToCleanup", documentCheckBox);
    registerField("mailToCleanup",mailCheckBox);
    registerField("eventToCleanup",eventCheckBox);

    connect(mailCheckBox,SIGNAL(clicked()),this,SIGNAL(completeChanged()));
    connect(eventCheckBox,SIGNAL(clicked()),this,SIGNAL(completeChanged()));
    connect(documentCheckBox,SIGNAL(clicked()),this,SIGNAL(completeChanged()));
}

bool PreselectionPage::isComplete() const
{
    // enable the Next> button only if the user has selected
    // Documents or Messages or Events
    if(mailCheckBox->checkState() == Qt::Unchecked  &&
        eventCheckBox->checkState() == Qt::Unchecked &&
         documentCheckBox->checkState() == Qt::Unchecked ) {
        return false;
    } else {
        return true;
    }
}

int PreselectionPage::nextId() const
{
    if (mailCheckBox->isChecked()) {
        return CleanupWizard::mailSelectionID;
    } else {
        if(eventCheckBox->isChecked()) {
            return CleanupWizard::eventSelectionID;
        } else {
           if(documentCheckBox->isChecked()) {
               return CleanupWizard::documentTypeSelectionID;
           } else {
               return CleanupWizard::finalSummaryID;
           }
        }
    }
}

 /*********************************************************************/
//               DocumentTypeSelectionPage   Methods                  //
/*********************************************************************/

DocumentTypeSelectionPage::DocumentTypeSelectionPage(QWidget* parent) : QWizardPage(parent)
{
    QVBoxLayout *vb = new QVBoxLayout;
    QScrollArea *scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setFocusPolicy(Qt::NoFocus);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_ui = new Ui::DocumentTypeSelector;
    m_ui->setupUi(scroll);

    vb->addWidget(scroll);

    setLayout(vb);

    registerField("documentsTypeSelectionPage_minimumSize", m_ui->minimumSize);
    registerField("documentsTypeSelectionPage_allLocations", m_ui->allLocations, "checked");
    registerField("documentsTypeSelectionPage_location", m_ui->location, "installationPath");
    registerField("documentsTypeSelectionPage_allDocuments", m_ui->allDocuments);
    registerField("documentsTypeSelectionPage_audioDocuments", m_ui->audioDocuments);
    registerField("documentsTypeSelectionPage_imageDocuments", m_ui->imageDocuments);
    registerField("documentsTypeSelectionPage_textDocuments", m_ui->textDocuments);
    registerField("documentsTypeSelectionPage_videoDocuments", m_ui->videoDocuments);

    connect(m_ui->allDocuments, SIGNAL(clicked(bool)), this, SLOT(alltypes(bool)));
    connect(m_ui->allDocuments, SIGNAL(clicked()), this, SIGNAL(completeChanged()));
    connect(m_ui->audioDocuments, SIGNAL(clicked()), this, SIGNAL(completeChanged()));
    connect(m_ui->imageDocuments, SIGNAL(clicked()), this, SIGNAL(completeChanged()));
    connect(m_ui->textDocuments, SIGNAL(clicked()), this, SIGNAL(completeChanged()));
    connect(m_ui->videoDocuments, SIGNAL(clicked()), this, SIGNAL(completeChanged()));
}

bool DocumentTypeSelectionPage::isComplete() const
{
    return (m_ui->imageDocuments->isChecked() || m_ui->audioDocuments->isChecked() ||
            m_ui->textDocuments->isChecked() || m_ui->videoDocuments->isChecked() ||
            m_ui->allDocuments->isChecked());
}

void DocumentTypeSelectionPage::alltypes(bool allChecked)
{
    m_ui->audioDocuments->setEnabled(!allChecked);
    m_ui->imageDocuments->setEnabled(!allChecked);
    m_ui->textDocuments->setEnabled(!allChecked);
    m_ui->videoDocuments->setEnabled(!allChecked);
}

int DocumentTypeSelectionPage::nextId() const
{
    return CleanupWizard::documentListSelectionID;
}

 /*********************************************************************/
//               DocumentListSelectionpage Methods                    //
/*********************************************************************/

class DocumentListModel : public QContentSetModel
{
public:
    DocumentListModel(const QContentSet *cls, QObject *parent = 0);
    ~DocumentListModel();

    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

private:
    QMap<QContentId, Qt::CheckState> checkState;
};

DocumentListModel::DocumentListModel(const QContentSet *cls, QObject *parent)
    : QContentSetModel(cls, parent)
{
}

DocumentListModel::~DocumentListModel()
{
}

Qt::ItemFlags DocumentListModel::flags(const QModelIndex &index) const
{
    return QAbstractItemModel::flags(index) | Qt::ItemIsUserCheckable;
}

QVariant DocumentListModel::data(const QModelIndex &index, int role) const
{
    if (index.column() == 1) {
        if (role == Qt::DisplayRole)
            return qVariantFromValue(sizeUnit(content(index).size()));

        return QVariant();
    }

    if (role == Qt::CheckStateRole)
        return qVariantFromValue(int(checkState.value(contentId(index), Qt::Checked)));

    return QContentSetModel::data(index, role);
}

bool DocumentListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role == Qt::CheckStateRole && index.column() == 0) {
        checkState.insert(contentId(index), Qt::CheckState(value.toInt()));
        emit dataChanged(index, index);
        return true;
    }

    return QContentSetModel::setData(index, value, role);
}

int DocumentListModel::columnCount(const QModelIndex &) const
{
    return 2;
}

DocumentListSelectionPage::DocumentListSelectionPage(QWidget* parent)
    : QWizardPage(parent), waitWidget(0)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    header = new QLabel;
    header->setWordWrap(true);
    mainLayout->addWidget(header);

    documents = new QContentSet(QContentSet::Asynchronous, this);
    model = new QContentSetModel(documents, this);
    connect(model, SIGNAL(updateFinished()), this, SLOT(updateFinished()));

    largeDocuments = new QContentSet(this);
    largeModel = new DocumentListModel(largeDocuments, this);
    connect(largeModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
            this, SLOT(dataChanged(QModelIndex,QModelIndex)));

    list = new QTableView();
    list->setModel(largeModel);
    list->setAlternatingRowColors(true);
    list->setShowGrid(false);
    list->horizontalHeader()->setStretchLastSection(true);
    list->setEditTriggers(QAbstractItemView::NoEditTriggers);
    list->horizontalHeader()->hide();
    list->verticalHeader()->hide();
    list->setSelectionBehavior(QAbstractItemView::SelectRows);
    list->setSelectionMode(QAbstractItemView::SingleSelection);
    mainLayout->addWidget(list);

    documentsCleanupDialog = new DocumentsCleanupDialog();
}

void DocumentListSelectionPage::initializePage()
{
    selectedSize = 0;
    selectedDocuments = 0;
    docDeleted = 0;
    cleanupStopped = false;

    waitWidget = new QWaitWidget(this);
    waitWidget->show();

    documents->clear();

    if (!field("documentsTypeSelectionPage_allDocuments").toBool()) {
        documents->setCriteria(QContentFilter::MimeType, "none");

        if (field("documentsTypeSelectionPage_audioDocuments").toBool())
            documents->addCriteria(QContentFilter::MimeType, "audio/*", QContentFilter::Or);
        if (field("documentsTypeSelectionPage_textDocuments").toBool())
            documents->addCriteria(QContentFilter::MimeType, "text/*", QContentFilter::Or);
        if (field("documentsTypeSelectionPage_videoDocuments").toBool())
            documents->addCriteria(QContentFilter::MimeType, "video/*", QContentFilter::Or);
        if (field("documentsTypeSelectionPage_imageDocuments").toBool())
            documents->addCriteria(QContentFilter::MimeType, "image/*", QContentFilter::Or);
    }

    if (field("documentsTypeSelectionPage_allLocations").toBool()) {
        documents->addCriteria(QContentFilter::Location,
                               field("documentsTypeSelectionPage_location").toString(),
                               QContentFilter::And);
    }

    documents->addCriteria(QContentFilter::Role, "Document", QContentFilter::And);
}

int DocumentListSelectionPage::nextId() const
{
    return CleanupWizard::finalSummaryID;
}

void DocumentListSelectionPage::updateHeader()
{
    if (largeDocuments->isEmpty()) {
        qLog(CleanupWizard) << "No documents found";

        header->setText(tr("There are no documents matching the specified criteria."));
        header->setAlignment(Qt::AlignCenter);

        list->hide();
    } else {
        qLog(CleanupWizard) << largeDocuments->count() << "documents found";

        header->setText(tr("%n document(s) using %1.", "%n=number,%1=filesize (ie 5 kB)",
                        selectedDocuments).arg(sizeUnit(selectedSize)));
        header->setAlignment(Qt::AlignLeft | Qt::AlignTop);

        list->show();
    }

    header->show();
}

void DocumentListSelectionPage::cleanup()
{
    connect(documentsCleanupDialog->pushButton, SIGNAL(clicked()), this, SLOT(stopCleanup()));

    documentsCleanupDialog->progressBar->setRange(0, selectedSize);

    if (largeModel->rowCount() == 0)
        return;

    qLog(CleanupWizard) << "Deleting Documents...";
    for (int i = 0; i < largeModel->rowCount(); i++) {
        if (cleanupStopped)
            break;

        QModelIndex index = largeModel->index(i, 0);

        if (largeModel->data(index, Qt::CheckStateRole) == Qt::Checked) {
            QContent content = largeModel->content(index);

            documentsCleanupDialog->label->setText(tr("Deleting %1").arg(content.name()));

            qApp->processEvents();

            int contentSize = content.size();

            content.removeFiles();

            if (!content.isValid()) {
                documentsCleanupDialog->progressBar->setValue(
                    documentsCleanupDialog->progressBar->value() + contentSize);

                docDeleted++;
            }
        }
    }

    documentsCleanupDialog->pushButton->hide();
    if (cleanupStopped) {
        documentsCleanupDialog->label->setText(tr("Aborted"));
    } else {
        documentsCleanupDialog->progressBar->setValue(
            documentsCleanupDialog->progressBar->maximum());
        documentsCleanupDialog->label->setText(tr("Done"));
        documents->clear();
    }

    QTimer::singleShot(1000, documentsCleanupDialog, SLOT(close()));
    emit documentsDeleted(docDeleted, cleanupStopped);
}



bool DocumentListSelectionPage::validatePage()
{
    if(selectedSize) {
        QMessageBox box(QMessageBox::Question, tr("Warning"),
                        tr("Are you sure you want to delete these files?"),
                        (QMessageBox::Yes | QMessageBox::No | QMessageBox::Default), this);
        switch (QtopiaApplication::execDialog(&box, true))
        {
            case QMessageBox::Yes: 
                documentsCleanupDialog->showMaximized();
                cleanup();
                break;
            case QMessageBox::No: 
            default:
                return false;
        }
     }
     return true;
}

void DocumentListSelectionPage::updateFinished()
{
    largeDocuments->clear();

    selectedSize = 0;
    selectedDocuments = 0;
    for (int i = 0; i < documents->count(); i++) {
        const QContent content = documents->content(i);
        if (content.size() >= field("documentsTypeSelectionPage_minimumSize").toInt() * 1024) {
            largeDocuments->add(content);

            selectedSize += content.size();
            selectedDocuments++;
        }
    }

    list->resizeColumnsToContents();
    list->setColumnWidth(0, list->viewport()->size().width() - list->columnWidth(1) - 10);
    list->setCurrentIndex(largeModel->index(0, 0));

    updateHeader();

    if (waitWidget) {
        delete waitWidget;
        waitWidget = 0;
    }
}

void DocumentListSelectionPage::dataChanged(const QModelIndex &topLeft,
                                            const QModelIndex &bottomRight)
{
    for (int i = topLeft.row(); i <= bottomRight.row(); i++) {
        QModelIndex index = largeModel->index(i, 0);

        qint64 fileSize = largeModel->content(index).size();

        if (largeModel->data(index, Qt::CheckStateRole) == Qt::Checked) {
            selectedSize += fileSize;
            selectedDocuments++;
        } else {
            selectedSize -= fileSize;
            selectedDocuments--;
        }
    }

    updateHeader();
}

 /*********************************************************************/
//                 DocumentsCleanupDialog Methods                     //
/*********************************************************************/

DocumentsCleanupDialog::DocumentsCleanupDialog(QWidget* parent) : QDialog(parent)
{
    setObjectName("cleanupDialog");

    QVBoxLayout* layout = new QVBoxLayout(this);
    progressBar = new QProgressBar(this);
    layout->addWidget(progressBar);
    label = new QLabel(tr("Starting cleanup"), this);
    layout->addWidget(label);
    pushButton = new QPushButton(tr("Cancel"), this);
    layout->addWidget(pushButton);
    QSpacerItem *spacer = new QSpacerItem( 20, 20, QSizePolicy::Minimum,QSizePolicy::Expanding);
    layout->addItem(spacer);
}

 /*********************************************************************/
//                      MailSelectionPage  Methods                    //
/*********************************************************************/


MailSelectionPage::MailSelectionPage(QWidget* parent) : QWizardPage(parent)
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing( 6 );
    mainLayout->setMargin( 2 );
    mainLayout->addWidget(new QLabel(tr("Cleanup Messages"), this));

    QFrame *line = new QFrame( this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken );
    mainLayout->addWidget(line);

    QGridLayout *grid = new QGridLayout();
    grid->addWidget(new QLabel(tr("Older than"), this), 0, 0);
    QDate date;
    dp = new QDateEdit(date.currentDate(),this);
    grid->addWidget( dp, 0, 1 );

    grid->addWidget(new QLabel(tr("Larger than"), this), 1, 0);
    sizeBox = new QSpinBox(this);
    sizeBox->setButtonSymbols(QSpinBox::UpDownArrows);
    sizeBox->setSuffix(' ' + tr("kB", "kilobyte"));
    sizeBox->setMinimum( 0 );
    sizeBox->setMaximum( 100000 );
    sizeBox->setValue( 10 );
    sizeBox->setSingleStep( 10 );
    grid->addWidget(sizeBox, 1, 1);
    QSpacerItem *spacer = new QSpacerItem( 20, 20, QSizePolicy::Minimum,QSizePolicy::Expanding);
    mainLayout->addLayout(grid);
    mainLayout->addItem(spacer);
    registerField("MailSelectionPage_sizeBox",sizeBox);
    registerField("MailSelectionPage_dateBox",dp);
}

int MailSelectionPage::nextId() const
{
    if(field("eventToCleanup").toBool()) {
        return CleanupWizard::eventSelectionID;
    } else {
        if(field("documentToCleanup").toBool()) {
            return CleanupWizard::documentTypeSelectionID;
        } else {
            return CleanupWizard::finalSummaryID;
        }
    }
}

bool MailSelectionPage::validatePage()
{
    QtopiaServiceRequest req("Email", "cleanupMessages(QDate,int)");

    QMessageBox box(QMessageBox::Question, tr("Warning"),
                    tr("Are you sure you want to delete messages older than %1?").arg(QTimeString::localYMD(dp->date(), QTimeString::Long)),
                    (QMessageBox::Yes | QMessageBox::No | QMessageBox::Default), this);

    switch (QtopiaApplication::execDialog(&box)) {
    case QMessageBox::Yes:
        req << field("MailSelectionPage_dateBox").toDate().addDays(-1);
        req << field("MailSelectionPage_sizeBox").toInt();
        req.send();
        break;
    default:
        return false;
    }

    return true;
}
 /*********************************************************************/
//                   EventSelectionPage Methods                       //
/*********************************************************************/

EventSelectionPage::EventSelectionPage(QWidget* parent) : QWizardPage(parent)
{
    QVBoxLayout* vl = new QVBoxLayout(this);
    vl->setSpacing( 6 );
    vl->setMargin( 2 );
    vl->addWidget(new QLabel(tr("Cleanup Events"), this));

    QFrame *line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken );
    vl->addWidget(line);

    QGridLayout *grid = new QGridLayout();
    grid->addWidget(new QLabel(tr("Older than"), this), 0, 0);
    QDate date;
    dp = new QDateEdit(date.currentDate(),this);
    grid->addWidget( dp, 0, 1 );
    vl->addLayout(grid);
    QSpacerItem *spacer = new QSpacerItem( 20, 20, QSizePolicy::Minimum,QSizePolicy::Expanding);
    vl->addItem(spacer);
    registerField("EventSelectionPage_dateBox",dp);
}

int EventSelectionPage::nextId() const
{
    if(field("documentToCleanup").toBool()) {
        return CleanupWizard::documentTypeSelectionID;
    } else {
        return CleanupWizard::finalSummaryID;
    }
}

bool EventSelectionPage::validatePage()
{
    QtopiaServiceRequest req("Calendar", "cleanByDate(QDate)");

    QMessageBox box(QMessageBox::Question, tr("Warning"),
                    tr("Are you sure you want to delete events older than %1?").arg(QTimeString::localYMD(dp->date(), QTimeString::Long)),
                    (QMessageBox::Yes | QMessageBox::No | QMessageBox::Default), this);

    switch (QtopiaApplication::execDialog(&box)) {
    case QMessageBox::Yes:
        req << field("EventSelectionPage_dateBox").toDate().addDays(-1);
        req.send();
        break;
    default:
        return false;
    }

    return true;
}

 /*********************************************************************/
//                    FinalSummaryPage  Methods                       //
/*********************************************************************/

FinalSummaryPage::FinalSummaryPage(QWidget* parent) : QWizardPage(parent)
{
    QVBoxLayout* vb = new QVBoxLayout(this);
    header = new QLabel;
    header->setWordWrap(true);
    vb->addWidget(header);
    QFrame *line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    vb->addWidget(line);
    summary = new QLabel;
    vb->addWidget(summary);
    registerField("FinalSummaryPage_summary", summary);

    docDeleted = 0;
    cleanupAborted = false;
}

int FinalSummaryPage::nextId() const
{
    return -1;
}

void FinalSummaryPage::initializePage()
{
    QString summaryText;

    if(docDeleted) {
        summaryText.append(tr("<li>%n document(s)", "", docDeleted));
        if(cleanupAborted)
            summaryText.append("<br/>"+tr( "( cleanup aborted )"));
        summaryText.append("</li>");
    }
    if(field("mailToCleanup").toBool()) {
        // add "events deleted (MM/DD/YYYY)" to the list
        summaryText.append(tr("<li>mails (%1) </li>").arg(QTimeString::localYMD(field("MailSelectionPage_dateBox").toDate(), QTimeString::Short)));
    }
    if(field("eventToCleanup").toBool()) {
        // add "events deleted (MM/DD/YYYY)" to the summary
        summaryText.append(tr("<li>events (%1) </li>").arg(QTimeString::localYMD(field("EventSelectionPage_dateBox").toDate(), QTimeString::Short)));
    }
    if (!summaryText.isEmpty()) {
        summaryText.prepend("<ul>");
        summaryText.append("</ul>");
        summary->setText(summaryText);
        header->setText(tr("The following items have been deleted:"));
    } else {
        // reset header and summary
        header->setText(tr("No cleanup actions has been taken."));
        summary->setText(QString());
    }
}


#include "cleanupwizard.moc"
