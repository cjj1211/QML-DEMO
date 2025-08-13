#pragma once

#include <QtCore/qglobal.h>

#ifndef BUILD_STATIC
# if defined(NX_G3_CONTROLLER_LIB)
#  define NX_G3_CONTROLLER_EXPORT Q_DECL_EXPORT
# else
#  define NX_G3_CONTROLLER_EXPORT Q_DECL_IMPORT
# endif
#else
# define NX_G3_CONTROLLER_EXPORT
#endif
