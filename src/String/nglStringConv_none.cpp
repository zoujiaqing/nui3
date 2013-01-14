/*
  NGL - C++ cross-platform framework for OpenGL based applications
  Copyright (C) 2000-2003 NGL Team

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "ngl.h"
#include <ngl_config.h>

#if !(defined HAVE_ICONV) && !(defined HAVE_MLANG)

#include "nglStringConv.h"


/* No charset conversion support, provide a pass-thru stub
 */

nglStringConv::nglStringConv (const nglTextEncoding From, const nglTextEncoding To, nglChar Default)
{
  mFrom    = From;
  mTo      = To;
  mDefault = Default;
}

nglStringConv::~nglStringConv()
{
}

int nglStringConv::Process (const char*& pSource, int& rToRead, char*& pTarget, int& rToWrite)
{
  int done = 0;

  while (rToRead > 0 && rToWrite > 0)
  {
    *pTarget++ = *pSource++;
    rToRead--;
    rToWrite--;
    done++;
  }

  return done;
}

#endif // !(defined HAVE_ICONV) || !(defined HAVE_MLANG)