/*
 * X18NCMSstubs.c
 * - Provides stubs and dummy funcs needed when Xcms and XLocale stuff removed
 *
 * Copyright © 2003 Matthew Allum
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Matthew Allum not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Keith Packard and Compaq makes no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.
 *
 * MATTHEW ALLUM DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS 
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, 
 * IN NO EVENT SHALL KEITH PACKARD BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#include <stdlib.h>
#include "Xlibint.h"
#include "Xlcint.h"
#include <X11/Xlocale.h>
#include <X11/Xos.h>
#ifdef WIN32
#undef close
#endif
#include <X11/Xutil.h>
#include "XlcPubI.h"

#include "Xcmsint.h" 		/* for XcmsCCC type  */
#include "XlcPubI.h"            /* for XLCd type */
#include "config.h"

#ifdef DISABLE_XLOCALE

Bool
XSupportsLocale()
{
  return False; 		
}

char *
XSetLocaleModifiers(
    const char *modifiers)
{
  return NULL;
}

XLCd
_XOpenLC(
    char *name)
{
  return NULL;
}

XLCd
_XlcCurrentLC()
{
  return NULL;
}

void
_XlcVaToArgList(
    va_list var,
    int count,
    XlcArgList *args_ret)
{
  return;
}

void
_XlcCountVaList(
    va_list var,
    int *count_ret)
{
  return;
}

void
_XCloseLC(
    XLCd lcd)
{
  return;
}

XPointer
_XimGetLocaleCode ( _Xconst char* encoding_name )
{
  return NULL;
}

void XmbSetWMProperties (        /* Actually from mbWMProps.c */
    Display *dpy,
    Window w,
    _Xconst char *windowName,
    _Xconst char *iconName,
    char **argv,
    int argc,
    XSizeHints *sizeHints,
    XWMHints *wmHints,
    XClassHint *classHints)
{
  return;
}

int
XmbTextPropertyToTextList(
    Display *dpy,
    const XTextProperty *text_prop,
    char ***list_ret,
    int *count_ret)
{
  return XLocaleNotSupported;
}

int
XmbTextListToTextProperty(
    Display *dpy,
    char **list,
    int count,
    XICCEncodingStyle style,
    XTextProperty *text_prop)
{
  return XLocaleNotSupported;
}

#endif 

#ifdef DISABLE_XCMS

XcmsCCC 
XcmsCCCOfColormap(dpy, cmap)
    Display *dpy;
    Colormap cmap;
{
  return NULL;
}

Status
_XcmsResolveColorString (
    XcmsCCC ccc,
    const char **color_string,
    XcmsColor *pColor_exact_return,
    XcmsColorFormat result_format)
{
  return(XcmsFailure);
}

void
_XcmsUnresolveColor(
    XcmsCCC ccc,
    XcmsColor *pColor)
{
  return;
}

void
_XUnresolveColor(
    XcmsCCC ccc,
    XColor *pXColor)
{
  return;
}

XcmsCmapRec *
_XcmsAddCmapRec(dpy, cmap, windowID, visual)
    Display *dpy;
    Colormap cmap;
    Window windowID;
    Visual *visual;
{
  return NULL;
}

void
_XcmsRGB_to_XColor(
    XcmsColor *pColors,
    XColor *pXColors,
    unsigned int nColors)
{
  return;
}

XcmsCmapRec *
_XcmsCopyCmapRecAndFree(
    Display *dpy,
    Colormap src_cmap,
    Colormap copy_cmap)
{
  return NULL;
}

void
_XcmsDeleteCmapRec(
    Display *dpy,
    Colormap cmap)
{
  return;
}

#endif
