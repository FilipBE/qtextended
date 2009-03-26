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

#include "gsmspec.h"
#include <qdebug.h>

GSMSpec::GSMSpec(const QString& specFile)
{
    setupDictionary(specFile);
}


QString GSMSpec::getProfile(const QString& command)
{
    if(commandExists(command)){
        GSMItem gi = commandMap.value(command);
        return gi.getProfile();
    }else{
        return "";
    }
}


QStringList GSMSpec::getParameterFormat(const QString& command)
{
    if(commandExists(command)){
        GSMItem gi = commandMap.value(command);
        return gi.getParameterFormat();
    }else{
        return QStringList("format not available");
    }
}


QStringList GSMSpec::getResponseFormat(const QString& command)
{
    if(commandExists(command)){
        GSMItem gi = commandMap.value(command);
        return gi.getParameterFormat();
    }else{
        return QStringList();
    }
}


bool GSMSpec::commandExists(const QString& command)
{
    return commandMap.contains(command);
}


bool GSMSpec::validateCommand(QString format, QString pars)
{
    Q_UNUSED(pars);
    QStringList formatList = format.split(",");
    QStringList parsList = format.split(",");

    if( formatList.size() == 0 && parsList.size() == 0 ){
        return true;
    }else if( true ){
    }
    return true; // TODO: Make useful
}


bool GSMSpec::validateResponse(QString format, QString pars)
{
    Q_UNUSED(pars);
    QStringList formatList = format.split(",");
    QStringList parsList = format.split(",");

    if( formatList.size() == 0 && parsList.size() == 0 ){
        return true;
    }else if( true ){
    }
    return true; // TODO: Make useful
}

void GSMSpec::resetDictionary(const QString& filePath)
{
    if( QFile::exists(filePath) ){
        setupDictionary(filePath);
    }
}

static bool readGsmFile( GsmXmlHandler *handler, const QString& filename )
{
    QFile f( filename );
    if ( !f.open( QIODevice::ReadOnly ) )
        return false;
    QXmlStreamReader reader( &f );
    while ( !reader.atEnd() ) {
        reader.readNext();
        if ( reader.hasError() )
            break;
        if ( reader.isStartElement() ) {
            handler->startElement( reader.name().toString(), reader.attributes() );
        } else if ( reader.isEndElement() ) {
            handler->endElement();
        } else if ( reader.isCharacters() ) {
            handler->characters( reader.text().toString() );
        }
    }
    f.close();
    return !reader.hasError();
}

void GSMSpec::setupDictionary(const QString& filePath)
{

    commandMap.clear();
    QString line;
    QStringList okList = QStringList("OK");

    GsmXmlHandler *handler = new GsmXmlHandler();
    if( !readGsmFile(handler, filePath) ){
        qWarning() << "Failed to parse GSM xml file" ;
        return;
    }

    GsmXmlNode *specNode = handler->documentElement()->children;
    QString command, profile, format, response, description;

    while ( specNode != 0 ) {
        if ( specNode->tag == "spec" ) {

            GsmXmlNode *specData = specNode->children;
            while ( specData != 0 ) {

                if( specData->tag == "command" ) {
                    command = specData->contents;
                }else if( specData->tag == "profile" ) {
                    profile = specData->contents;
                }else if( specData->tag == "format" ) {
                    format = specData->contents;
                }else if( specData->tag == "response" ) {
                    response = specData->contents;
                }else if( specData->tag == "description" ) {
                    description = specData->contents;
                }else{
                    // not a valid entry
                    break;
                }
                specData = specData->next;

            }
            commandMap.insert( command, GSMItem(command, profile, format.split(";"), response.split(";"), description) );
        }
        specNode = specNode->next;
    }
    // Clean up the XML reader objects.
    delete handler;

}

GsmXmlNode::GsmXmlNode( const QString& _tag )
{
    parent = 0;
    next = 0;
    children = 0;
    attributes = 0;
    tag = _tag;
}


GsmXmlNode::~GsmXmlNode()
{
    GsmXmlNode *temp1, *temp2;
    temp1 = children;
    while ( temp1 ) {
        temp2 = temp1->next;
        delete temp1;
        temp1 = temp2;
    }
    temp1 = attributes;
    while ( temp1 ) {
        temp2 = temp1->next;
        delete temp1;
        temp1 = temp2;
    }
}


void GsmXmlNode::addChild( GsmXmlNode *child )
{
    GsmXmlNode *current = children;
    GsmXmlNode *prev = 0;
    while ( current ) {
        prev = current;
        current = current->next;
    }
    if ( prev ) {
        prev->next = child;
    } else {
        children = child;
    }
    child->next = 0;
    child->parent = this;
}


void GsmXmlNode::addAttribute( GsmXmlNode *child )
{
    GsmXmlNode *current = attributes;
    GsmXmlNode *prev = 0;
    while ( current ) {
        prev = current;
        current = current->next;
    }
    if ( prev ) {
        prev->next = child;
    } else {
        attributes = child;
    }
    child->next = 0;
    child->parent = this;
}


QString GsmXmlNode::getAttribute( const QString& name )
{
    GsmXmlNode *current = attributes;
    while ( current ) {
        if ( current->tag == name )
            return current->contents;
        current = current->next;
    }
    return QString();
}


GsmXmlHandler::GsmXmlHandler()
{
    tree = new GsmXmlNode( QString() );
    current = tree;
}


GsmXmlHandler::~GsmXmlHandler()
{
    delete tree;
}


bool GsmXmlHandler::startElement( const QString& name, const QXmlStreamAttributes& atts )
{
    GsmXmlNode *node = new GsmXmlNode( name );
    GsmXmlNode *attr;
    int index;
    current->addChild( node );
    for ( index = 0; index < atts.size(); ++index ) {
        attr = new GsmXmlNode( atts[index].name().toString() );
        attr->contents = atts[index].value().toString();
        node->addAttribute( attr );
    }
    current = node;
    return true;
}


bool GsmXmlHandler::endElement()
{
    current = current->parent;
    return true;
}


bool GsmXmlHandler::characters( const QString& ch )
{
    current->contents += ch;
    return true;
}


GsmXmlNode *GsmXmlHandler::documentElement() const
{
    if ( tree->children && tree->children->tag == "gsm" ) {
        return tree->children;
    } else {
        return tree;
    }
}
