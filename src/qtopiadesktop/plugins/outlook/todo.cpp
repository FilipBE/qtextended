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
#include "outlooksync.h"
#include "trace.h"

#include <QBuffer>
#include <QXmlStreamReader>
#include <QFile>

class OutlookTodoSync : public OutlookSyncPlugin
{
    Q_OBJECT
    QD_CONSTRUCT_PLUGIN(OutlookTodoSync,OutlookSyncPlugin)
public:
    QString displayName() { return tr("Outlook Tasks"); }

    QString dataset() { return "tasks"; }
    QByteArray referenceSchema() { return "<Task><Identifier/><Description/><Priority/><Status/><DueDate/><StartedDate/><CompletedDate/><PercentCompleted/><Notes/><Categories/></Task>"; }

    Outlook::OlDefaultFolders folderEnum() { return Outlook::olFolderTasks; }
    Outlook::OlItemType itemEnum() { return Outlook::olTaskItem; }
    bool isValidObject( IDispatchPtr dispatch )
    {
        TRACE(OutlookSyncPlugin) << "OutlookTodoSync::isValidObject";
        Outlook::_TaskItemPtr item( dispatch );
        LOG() << "The item class is" << dump_item_class(item->GetClass()) << "expecting" << dump_item_class(Outlook::olTask);
        return ( item->GetClass() == Outlook::olTask );
    }

    void getProperties( IDispatchPtr dispatch, QString &entryid, QDateTime &lastModified )
    {
        TRACE(OutlookSyncPlugin) << "OutlookTodoSync::getProperties";
        Outlook::_TaskItemPtr item( dispatch );
        try {
            entryid = bstr_to_qstring(item->GetEntryID());
        } catch (...) {
            entryid = QString();
            return;
        }
        lastModified = date_to_qdatetime(item->GetLastModificationTime());
    }

    void dump_item( IDispatchPtr dispatch, QXmlStreamWriter &stream )
    {
        TRACE(OutlookSyncPlugin) << "OutlookTodoSync::dump_item";
        Q_ASSERT( dispatch );
        Outlook::_TaskItemPtr item( dispatch );
        Outlook::UserPropertiesPtr props = item->GetUserProperties();
        Q_ASSERT(props);

        PREPARE_MAPI(Task);

        stream.writeStartElement("Task");
        DUMP_STRING(Identifier,EntryID);
        DUMP_STRING(Description,Subject);
        QString high = "High";
        QString low = "Low";
        {
            Outlook::UserPropertyPtr up = props->Find("Qtopia Priority");
            if ( up ) {
                QString priority = variant_to_qstring(up->GetValue());
                if ( priority == "VeryHigh" && item->GetImportance() == Outlook::olImportanceHigh )
                    high = "VeryHigh";
                if ( priority == "VeryLow" && item->GetImportance() == Outlook::olImportanceLow )
                    low = "VeryLow";
            }
        }
        stream.writeStartElement("Priority");
        DUMP_ENUM_V(Priority,Importance,Outlook::olImportanceHigh,high);
        DUMP_ENUM(Priority,Importance,Outlook::olImportanceNormal,Normal);
        DUMP_ENUM_V(Priority,Importance,Outlook::olImportanceLow,low);
        stream.writeEndElement();
        stream.writeStartElement("Status");
        DUMP_ENUM(Status,Status,Outlook::olTaskNotStarted,NotStarted);
        DUMP_ENUM(Status,Status,Outlook::olTaskInProgress,InProgress);
        DUMP_ENUM(Status,Status,Outlook::olTaskComplete,Completed);
        DUMP_ENUM(Status,Status,Outlook::olTaskWaiting,Waiting);
        DUMP_ENUM(Status,Status,Outlook::olTaskDeferred,Deferred);
        stream.writeEndElement();
        DUMP_DATE(DueDate,DueDate);
        DUMP_DATE(StartedDate,StartDate);
        DUMP_DATE(CompletedDate,DateCompleted);
        DUMP_INT(PercentCompleted,PercentComplete);
        DUMP_MAPI(Notes,Body);
        stream.writeStartElement("Categories");
        foreach ( const QString &category, bstr_to_qstring(item->GetCategories()).split(", ") )
            DUMP_EXPR(Category,category);
        stream.writeEndElement();
        stream.writeEndElement();
    }

    QString read_item( IDispatchPtr dispatch, const QByteArray &record )
    {
        TRACE(OutlookSyncPlugin) << "OutlookTodoSync::read_item";
        Q_ASSERT( dispatch );
        Outlook::_TaskItemPtr item( dispatch );
        Outlook::UserPropertiesPtr props = item->GetUserProperties();
        Q_ASSERT(props);

        enum State {
            Idle, Categories
        };
        State state = Idle;

        QXmlStreamReader reader(record);
        QString key;
        QXmlStreamAttributes attributes;
        QString value;
        QStringList categories;
        while (!reader.atEnd()) {
            switch(reader.readNext()) {
                case QXmlStreamReader::StartElement:
                    key = reader.qualifiedName().toString();
                    value = QString();
                    attributes = reader.attributes();
                    if ( key == "Categories" )
                        state = Categories;
                    break;
                case QXmlStreamReader::Characters:
                    value += reader.text().toString();
                    break;
                case QXmlStreamReader::EndElement:
                    key = reader.qualifiedName().toString();
                    //LOG() << "key" << key << "value" << value;
                    READ_STRING(Description,Subject);
                    READ_ENUM(Priority,Importance,Outlook::olImportanceHigh,VeryHigh);
                    READ_ENUM(Priority,Importance,Outlook::olImportanceHigh,High);
                    READ_ENUM(Priority,Importance,Outlook::olImportanceNormal,Normal);
                    READ_ENUM(Priority,Importance,Outlook::olImportanceLow,Low);
                    READ_ENUM(Priority,Importance,Outlook::olImportanceLow,VeryLow);
                    // store the Qtopia value in a custom key then
                    // return this to Qtopia instead of "changing"
                    // the value (obviously this only works if you
                    // don't change the Outlook value to something
                    // else.
                    // eg. Qtopia:HV -> Outlook:H -> Sync:VH
                    // eg. Qtopia:HV -> Outlook:H -> User:N -> Sync:N
                    READ_CUSTOM(Priority,Qtopia Priority);
                    READ_ENUM(Status,Status,Outlook::olTaskNotStarted,NotStarted);
                    READ_ENUM(Status,Status,Outlook::olTaskInProgress,InProgress);
                    READ_ENUM(Status,Status,Outlook::olTaskComplete,Completed);
                    READ_ENUM(Status,Status,Outlook::olTaskWaiting,Waiting);
                    READ_ENUM(Status,Status,Outlook::olTaskDeferred,Deferred);
                    READ_DATE(DueDate,DueDate);
                    READ_DATE(StartedDate,StartDate);
                    READ_DATE(CompletedDate,DateCompleted);
                    READ_INT(PercentCompleted,PercentComplete);
                    READ_STRING(Notes,Body);
                    if ( state == Categories ) {
                        if ( key == "Category" )
                            categories << value;
                        if ( key == "Categories" ) {
                            item->PutCategories( qstring_to_bstr(categories.join(", ")) );
                            state = Idle;
                        }
                    }
                    break;
                default:
                    break;
            }
        }

        item->Save();
        return bstr_to_qstring(item->GetEntryID());
    }

    void delete_item( IDispatchPtr dispatch )
    {
        Q_ASSERT( dispatch );
        Outlook::_TaskItemPtr item( dispatch );
        item->Delete();
    }
};

QD_REGISTER_PLUGIN(OutlookTodoSync);

#include "todo.moc"
