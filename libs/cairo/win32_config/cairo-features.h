#ifndef CAIRO_FEATURES_H
#define CAIRO_FEATURES_H

#if defined(__cplusplus) && !defined(LIBCAIRO_EXPORTS)
# define CAIRO_BEGIN_DECLS  extern "C" {
# define CAIRO_END_DECLS    }
#else
# define CAIRO_BEGIN_DECLS
# define CAIRO_END_DECLS
#endif

#define cairo_public

#define HAVE_WINDOWS_H 1

//#define CAIRO_HAS_PNG_FUNCTIONS 1
//#define CAIRO_HAS_SVG_SURFACE 1
//#define CAIRO_HAS_PDF_SURFACE 1
//#define CAIRO_HAS_PS_SURFACE 1
//#define CAIRO_HAS_WIN32_SURFACE 1
//#define CAIRO_HAS_WIN32_FONT 1
#define CAIRO_HAS_FT_FONT 1
#define CAIRO_HAS_FONT_SUBSET 1

#endif
