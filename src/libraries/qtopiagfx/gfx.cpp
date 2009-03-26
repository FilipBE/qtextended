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

#include "gfx.h"
#include "routines.h"
#include <QDebug>
#include <QImage>
#include <math.h>
#include <dlfcn.h>
#include "gfximage.h"

bool gfx_use_qt = false;
bool gfx_report_hazards = false;

static const char *QImage_formatToString(QImage::Format f)
{
    switch(f) {
        case QImage::Format_Invalid:
            return "Invalid";
        case QImage::Format_Mono:
            return "Mono";
        case QImage::Format_MonoLSB:
            return "MonoLSB";
        case QImage::Format_Indexed8:
            return "Indexed8";
        case QImage::Format_RGB32:
            return "RGB32";
        case QImage::Format_ARGB32:
            return "ARGB32";
        case QImage::Format_ARGB32_Premultiplied:
            return "ARGB32_Premultiplied";
        case QImage::Format_RGB16:
            return "RGB16";
        default:
            return "Unknown";
    }
}

void Gfx::init(const char *_arch)
{
    static bool initComplete = false;
    if(initComplete)
        return;

    initComplete = true;
    if(!QString(getenv("GFX_USE_QT")).isEmpty())
        gfx_use_qt = true;
    if(!QString(getenv("GFX_REPORT_HAZARDS")).isEmpty())
        gfx_report_hazards = true;

    QByteArray arch;
    if(_arch) {
        arch = _arch;
    } else {
        const char *env = getenv("GFX_PLUGIN");
        if(env)
            arch = env;
    }
    if(arch.isEmpty())
        return;

    QByteArray plugin = "lib" + arch + ".so";

    void *hplugin = dlopen(plugin.constData(), RTLD_LAZY);
    if(!hplugin) {
        qWarning("Gfx::init: Unable to load plugin '%s'.  Falling back to defaults.", plugin.constData());
        qWarning("               %s", dlerror());
        return;
    }

    void (*init)(PluginRoutines *) = 
        (void (*)(PluginRoutines *))dlsym(hplugin, "gfx_init");
    if(!init) {
        qWarning("Gfx::init: Unable to resolve plugin_init() function.  Falling back to defaults.");
        return;
    }

    PluginRoutines plug = {
        &q_blurroutines,
        &q_blendroutines,
        &q_grayscaleroutines,
        &q_memoryroutines,
        &q_scaleroutines
    };

    init(&plug);
}

void Gfx::blur(GfxImageRef &img, qreal radius)
{
    if(radius <= -1.0f)
        return;

    int alpha = (int)((1<<15)*(1.0f-expf(-2.3f/(radius+1.f))));

    switch(img.format()) {
        case QImage::Format_RGB32:
        case QImage::Format_ARGB32:
        case QImage::Format_ARGB32_Premultiplied:
            q_blurroutines.blur32((uint *)img.bits(), img.width(), 
                                  img.height(), img.bytesPerLine() / 4, alpha);
            break;
        case QImage::Format_RGB16:
            q_blurroutines.blur16((ushort *)img.bits(), img.width(), 
                                  img.height(), img.bytesPerLine() / 2, alpha << 1);
            break;
        default:
            qWarning("Gfx::blur: Unable to blur image of type %s", 
                     QImage_formatToString(img.format()));
            break;
    }
}

void Gfx::blur(QImage &img, qreal radius)
{
    GfxImageRef ref(img);
    blur(ref, radius);
}

