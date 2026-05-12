/*********************************************************
 * Copyright (c) 2025 VMware, Inc. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation version 2.1 and no later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the Lesser GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA.
 *
 *********************************************************/

/**
 * @file copyPasteUIWayland.h
 *
 * This class implements the methods that allows Copy/Paste
 * between host and guest using version 3+ of the protocol on Wayland.
 *
 */

#ifndef __COPYPASTE_UI_WAYLAND_H__
#define __COPYPASTE_UI_WAYLAND_H__

#include "stringxx/string.hh"
#include "dnd.h"
#include "str.h"

extern "C" {
#include "debug.h"
#include "dndClipboard.h"
#include "../dnd/dndFileContentsUtil.h"
#include "vmware/tools/guestrpc.h"
}

#include "guestCopyPaste.hh"
#include "vmware/guestrpc/tclodefs.h"

#include <gtkmm.h>
#include <gdkmm.h>

class CopyPasteUIWayland
{
public:
   CopyPasteUIWayland();
   virtual ~CopyPasteUIWayland();
   bool Init();
   void VmxCopyPasteVersionChanged(RpcChannel *chan, uint32 version);
   void SetCopyPasteAllowed(bool isCopyPasteAllowed);
   void Reset(void);
   void SetBlockControl(DnDBlockControl *blockCtrl);

private:
   void GetRemoteClipboardCB(const CPClipboard *clip);
   void GetLocalClipboard(void);
   void SendClipNotChanged(void);

   GuestCopyPasteMgr *mCP;
   bool mClipboardEmpty;
};

#endif // __COPYPASTE_UI_WAYLAND_H__