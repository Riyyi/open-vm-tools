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

#include "copyPasteDnDWayland.h"
#include "copyPasteUIWayland.h"
#include "guestDnDCPMgr.hh"
#include "dnd.h"

extern "C" {
#include "debug.h"
}

CopyPasteDnDWayland::CopyPasteDnDWayland()
   : m_copyPasteUI(nullptr),
     m_ctx(nullptr)
{
}


CopyPasteDnDWayland::~CopyPasteDnDWayland()
{
   if (m_copyPasteUI) {
      delete m_copyPasteUI;
      m_copyPasteUI = nullptr;
   }
}


gboolean
CopyPasteDnDWayland::Init(ToolsAppCtx *ctx)
{
   m_ctx = ctx;
   return TRUE;
}


void
CopyPasteDnDWayland::PointerInit()
{
}


gboolean
CopyPasteDnDWayland::RegisterCP()
{
   if (m_copyPasteUI) {
      delete m_copyPasteUI;
   }

   m_copyPasteUI = new CopyPasteUIWayland();
   if (!m_copyPasteUI->Init()) {
      delete m_copyPasteUI;
      m_copyPasteUI = nullptr;
      return FALSE;
   }

   GuestDnDCPMgr *mgr = GuestDnDCPMgr::GetInstance();
   if (mgr) {
      mgr->SetCopyPasteUI(m_copyPasteUI);
   }

   return TRUE;
}


void
CopyPasteDnDWayland::UnregisterCP()
{
   if (m_copyPasteUI) {
      GuestDnDCPMgr *mgr = GuestDnDCPMgr::GetInstance();
      if (mgr) {
         mgr->SetCopyPasteUI(nullptr);
      }
      delete m_copyPasteUI;
      m_copyPasteUI = nullptr;
   }
}


gboolean
CopyPasteDnDWayland::RegisterDnD()
{
   return FALSE;
}


void
CopyPasteDnDWayland::UnregisterDnD()
{
}


void
CopyPasteDnDWayland::DnDVersionChanged(int version)
{
}


void
CopyPasteDnDWayland::CopyPasteVersionChanged(int version)
{
   if (m_copyPasteUI) {
      m_copyPasteUI->VmxCopyPasteVersionChanged(nullptr, version);
   }
}


uint32
CopyPasteDnDWayland::GetCaps()
{
   return DND_CP_CAP_VALID |
          DND_CP_CAP_CP |
          DND_CP_CAP_ACTIVE_CP |
          DND_CP_CAP_PLAIN_TEXT_CP;
}


void
CopyPasteDnDWayland::SetCopyPasteAllowed(bool allowed)
{
   if (m_copyPasteUI) {
      m_copyPasteUI->SetCopyPasteAllowed(allowed);
   }
}


void
CopyPasteDnDWayland::SetDnDAllowed(bool allowed)
{
}