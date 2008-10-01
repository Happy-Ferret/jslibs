#define GTK_PREFIX "gdk_pixbuf_prefix"

/* config.h.win32.in. Merged from two versions generated by configure for gcc and MSVC.  */
/* config.h.  Generated from config.h.in by configure.  */
/* config.h.in.  Generated from configure.in by autoheader.  */

/* always defined to indicate that i18n is enabled */
#undef ENABLE_NLS

/* The prefix for our gettext translation domains. */
#define GETTEXT_PACKAGE "gtk20"

/* Define if debugging is enabled */
#define GTK_COMPILED_WITH_DEBUGGING "yes"

/* Define the location where the catalogs will be installed */
#define GTK_LOCALEDIR "c:/devel/target/HEAD/share/locale"

/* Define to 1 if you have the `bind_textdomain_codeset' function. */
#define HAVE_BIND_TEXTDOMAIN_CODESET 1

/* Is the wctype implementation broken */
/* #undef HAVE_BROKEN_WCTYPE */

/* Define to 1 if you have the <crt_externs.h> header file. */
/* #undef HAVE_CRT_EXTERNS_H */

/* Define to 1 if CUPS 1.2 API is available */
/* #undef HAVE_CUPS_API_1_2 */

/* Define to 1 if you have the `dcgettext' function. */
#undef HAVE_DCGETTEXT

/* Define to 1 if you have the <dlfcn.h> header file. */
/* #undef HAVE_DLFCN_H */

/* Define to 1 if you have the `flockfile' function. */
/* #undef HAVE_FLOCKFILE */

/* Define to 1 if you have the <ftw.h> header file. */
/* #undef HAVE_FTW_H */

/* Define to 1 if you have the `getc_unlocked' function. */
/* #undef HAVE_GETC_UNLOCKED */

/* Define to 1 if you have the `getpagesize' function. */
#ifndef _MSC_VER
#define HAVE_GETPAGESIZE 1
#else
/* #undef HAVE_GETPAGESIZE */
#endif

/* Define to 1 if you have the `getresuid' function. */
/* #undef HAVE_GETRESUID */

/* Define if the GNU gettext() function is already present or preinstalled. */
#undef HAVE_GETTEXT

/* Have GNU ftw */
/* #undef HAVE_GNU_FTW */

/* Define to 1 if you have the `httpGetAuthString' function. */
/* #undef HAVE_HTTPGETAUTHSTRING */

/* Define if cups http_t authstring field is accessible */
/* #undef HAVE_HTTP_AUTHSTRING */

/* Define to 1 if you have the <inttypes.h> header file. */
#ifndef _MSC_VER
#define HAVE_INTTYPES_H 1
#else
/* #undef HAVE_INTTYPES_H */
#endif

/* Define to 1 if ipc.h is available */
/* #undef HAVE_IPC_H */

/* Define if your <locale.h> file defines LC_MESSAGES. */
/* #undef HAVE_LC_MESSAGES */

/* Define to 1 if you have the <locale.h> header file. */
#define HAVE_LOCALE_H 1

/* Define to 1 if you have the `localtime_r' function. */
/* #undef HAVE_LOCALTIME_R */

/* Define to 1 if you have the `lstat' function. */
/* #undef HAVE_LSTAT */

/* Define to 1 if you have the `mallinfo' function. */
/* #undef HAVE_MALLINFO */

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Define to 1 if you have the `mkstemp' function. */
/* #undef HAVE_MKSTEMP */

/* Define to 1 if you have a working `mmap' system call. */
/* #undef HAVE_MMAP */

/* Define to 1 is libjpeg supports progressive JPEG */
/* #undef HAVE_PROGRESSIVE_JPEG */

/* Define to 1 if you have the <pwd.h> header file. */
/* #undef HAVE_PWD_H */

/* Have the Xrandr extension library */
/* #undef HAVE_RANDR */

/* Define to 1 if the XShape extension is available */
/* #undef HAVE_SHAPE_EXT */

/* Define to 1 if shm.h is available */
/* #undef HAVE_SHM_H */

/* Define to 1 if sigsetjmp is available */
/* #undef HAVE_SIGSETJMP */

/* Have the sockaddr_un.sun_len member */
/* #undef HAVE_SOCKADDR_UN_SUN_LEN */

/* Define to 1 if solaris xinerama is available */
/* #undef HAVE_SOLARIS_XINERAMA */

/* Define to 1 if you have the <stdint.h> header file. */
#ifndef _MSC_VER
#define HAVE_STDINT_H 1
#else
/* #undef HAVE_STDINT_H */
#endif

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if sys/select.h is available */
/* #undef HAVE_SYS_SELECT_H */

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if sys/sysinfo.h is available */
/* #undef HAVE_SYS_SYSINFO_H */

/* Define to 1 if sys/systeminfo.h is available */
/* #undef HAVE_SYS_SYSTEMINFO_H */

/* Define to 1 if you have the <sys/time.h> header file. */
#ifndef _MSC_VER
#define HAVE_SYS_TIME_H 1
#else /* _MSC_VER */
/* #undef HAVE_SYS_TIME_H */
#endif

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have <sys/wait.h> that is POSIX.1 compatible. */
/* #undef HAVE_SYS_WAIT_H */

/* Define to 1 if you have the <unistd.h> header file. */
#ifndef _MSC_VER
#define HAVE_UNISTD_H 1
#else
/* #undef HAVE_UNISTD_H */
#endif

/* Have uxtheme.h include file */
#define HAVE_UXTHEME_H 1

/* Have wchar.h include file */
#define HAVE_WCHAR_H 1

/* Have wctype.h include file */
#define HAVE_WCTYPE_H 1

/* Define if we have X11R6 */
/* #undef HAVE_X11R6 */

/* Have the XCOMPOSITE X extension */
/* #undef HAVE_XCOMPOSITE */

/* Define to 1 if you have the `XConvertCase' function. */
/* #undef HAVE_XCONVERTCASE */

/* Have the Xcursor library */
/* #undef HAVE_XCURSOR */

/* Have the XDAMAGE X extension */
/* #undef HAVE_XDAMAGE */

/* Have the XFIXES X extension */
/* #undef HAVE_XFIXES */

/* Define to 1 if XFree Xinerama is available */
/* #undef HAVE_XFREE_XINERAMA */

/* Define to 1 is Xinerama is available */
/* #undef HAVE_XINERAMA */

/* Define to 1 if you have the `XInternAtoms' function. */
/* #undef HAVE_XINTERNATOMS */

/* Define to use XKB extension */
/* #undef HAVE_XKB */

/* Define to 1 if xshm.h is available */
/* #undef HAVE_XSHM_H */

/* Have the SYNC extension library */
/* #undef HAVE_XSYNC */

/* Define if _NL_MEASUREMENT_MEASUREMENT is available */
/* #undef HAVE__NL_MEASUREMENT_MEASUREMENT */

/* Define if _NL_PAPER_HEIGHT is available */
/* #undef HAVE__NL_PAPER_HEIGHT */

/* Define if _NL_PAPER_WIDTH is available */
/* #undef HAVE__NL_PAPER_WIDTH */

/* Define if _NL_TIME_FIRST_WEEKDAY is available */
/* #undef HAVE__NL_TIME_FIRST_WEEKDAY */

/* Define to 1 if you have the `_NSGetEnviron' function. */
/* #undef HAVE__NSGETENVIRON */

/* Define if <X11/extensions/XIproto.h> needed for xReply */
/* #undef NEED_XIPROTO_H_FOR_XREPLY */

/* Define to 1 if fd_set is not available */
#define NO_FD_SET 1

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT "http://bugzilla.gnome.org/enter_bug.cgi?product=gtk%2B"

/* Define to the full name of this package. */
#define PACKAGE_NAME "gtk+"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "gtk+ 2.14.3"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "gtk+"

/* Define to the version of this package. */
#define PACKAGE_VERSION "2.14.3"

/* Define as the return type of signal handlers (`int' or `void'). */
#define RETSIGTYPE void

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Define to 1 if gmodule works and should be used */
//#define USE_GMODULE 1
#undef USE_GMODULE

/* Whether to load modules via .la files rather than directly */
/* #undef USE_LA_MODULES */

/* Define to 1 if medialib is available and should be used */
/* #undef USE_MEDIALIB */

/* Define to 1 if medialib 2.5 is available */
/* #undef USE_MEDIALIB25 */

/* Define to 1 if XXM is available and should be used */
//#define USE_MMX 1
#undef USE_MMX 1

/* Define to 1 if no XInput should be used */
/* #undef XINPUT_NONE */

/* Define to 1 if XFree XInput should be used */
/* #undef XINPUT_XFREE */

/* Define to 1 if the X Window System is missing or not being used. */
/* #undef X_DISPLAY_MISSING */

/* Number of bits in a file offset, on hosts where this is settable. */
/* #undef _FILE_OFFSET_BITS */

/* Define for large files, on AIX-style hosts. */
/* #undef _LARGE_FILES */

/* Define to empty if `const' does not conform to ANSI C. */
/* #undef const */

/* Define to `int' if <sys/types.h> doesn't define. */
#define gid_t int

/* Define to `int' if <sys/types.h> doesn't define. */
#define uid_t int
