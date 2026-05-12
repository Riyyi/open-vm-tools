/*********************************************************
 * Copyright (C) 2025 VMware, Inc. All rights reserved.
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
 * @file copyPasteDnDWayland.h
 *
 * This class provides the concrete UI implementation for the DnD and
 * copy paste abstraction, for the Wayland platform.
 */

#ifndef __COPYPASTEDNDWAYLAND_H__
#define __COPYPASTEDNDWAYLAND_H__

#include "dnd.h"
#include "vm_basic_types.h"
#include "copyPasteDnDImpl.h"

class CopyPasteUIWayland;

class CopyPasteDnDWayland : public CopyPasteDnDImpl
{
public:
   CopyPasteDnDWayland();
   ~CopyPasteDnDWayland();
   virtual gboolean Init(ToolsAppCtx *ctx);
   virtual void PointerInit();
   virtual gboolean RegisterCP();
   virtual void UnregisterCP();
   virtual gboolean RegisterDnD();
   virtual void UnregisterDnD();
   virtual void DnDVersionChanged(int version);
   virtual void CopyPasteVersionChanged(int version);
   virtual uint32 GetCaps();
   void SetCopyPasteAllowed(bool allowed);
   void SetDnDAllowed(bool allowed);
private:
   CopyPasteUIWayland *m_copyPasteUI;
   ToolsAppCtx *m_ctx;
};

#endif // __COPYPASTEDNDWAYLAND_H__