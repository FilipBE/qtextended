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

#ifndef CAMERAFORMATCONVERTER_H
#define CAMERAFORMATCONVERTER_H

#include <QList>


/*
 * convert something to RGB32 / RGB16
 *
 */
class CameraFormatConverter
{
public:

    virtual ~CameraFormatConverter() {}

    virtual unsigned char* convert(unsigned char* src) = 0;

    static CameraFormatConverter* createFormatConverter(unsigned int format, int width, int height);
    static void releaseFormatConverter(CameraFormatConverter* converter);
    static QList<unsigned int> supportedFormats();
};


class NullConverter : public CameraFormatConverter
{
public:

    virtual unsigned char* convert(unsigned char* src);
};


class YUVConverter : public CameraFormatConverter
{
public:
    YUVConverter(unsigned int type, int width, int height);
    virtual ~YUVConverter();

    virtual unsigned char* convert(unsigned char* src);

private:

    unsigned int    m_type;
    int             m_width;
    int             m_height;
    unsigned char*  m_buf;
    int m_ui,m_vi,m_y1i,m_y2i;

};

class BayerConverter : public CameraFormatConverter
{
public:
    BayerConverter(int width, int height);
    virtual ~BayerConverter();

    virtual unsigned char* convert(unsigned char* src);

private:

    int             m_width;
    int             m_height;
    unsigned char*  m_buf;
};

#endif
