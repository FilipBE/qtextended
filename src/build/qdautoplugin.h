#ifndef QDAUTOPLUGIN_H
#define QDAUTOPLUGIN_H

#define QD_REGISTER_PLUGIN(IMPLEMENTATION) \
    void qd_registerPlugin_AUTOPLUGIN_TARGET(const QString &id, qdPluginCreateFunc_t createFunc);\
    static QDPlugin *create_ ## IMPLEMENTATION( QObject *parent ) \
        { return new IMPLEMENTATION(parent); } \
    static qdPluginCreateFunc_t append_ ## IMPLEMENTATION() \
        { qd_registerPlugin_AUTOPLUGIN_TARGET(#IMPLEMENTATION, create_ ## IMPLEMENTATION); \
            return create_ ## IMPLEMENTATION; } \
    static qdPluginCreateFunc_t dummy_ ## IMPLEMENTATION = \
        append_ ## IMPLEMENTATION();

#endif
