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

#include "copyPasteUIWayland.h"
#include "guestCopyPasteMgr.hh"
#include "guestDnDCPMgr.h"

CopyPasteUIWayland::CopyPasteUIWayland()
   : mCP(nullptr),
     mClipboardEmpty(true)
{
}


CopyPasteUIWayland::~CopyPasteUIWayland()
{
}


bool
CopyPasteUIWayland::Init()
{
   mCP = GuestCopyPasteMgr::GetInstance();
   if (!mCP) {
      return false;
   }

   mCP->SetSrcCallback(
      sigc::mem_fun(this, &CopyPasteUIWayland::GetRemoteClipboardCB));
   mCP->SetDestCallback(
      sigc::mem_fun(this, &CopyPasteUIWayland::GetLocalClipboard));

   return true;
}


void
CopyPasteUIWayland::VmxCopyPasteVersionChanged(RpcChannel *chan, uint32 version)
{
}


void
CopyPasteUIWayland::SetCopyPasteAllowed(bool isCopyPasteAllowed)
{
   if (mCP) {
      mCP->SetCopyPasteAllowed(isCopyPasteAllowed);
   }
}


void
CopyPasteUIWayland::Reset(void)
{
}


void
CopyPasteUIWayland::SetBlockControl(DnDBlockControl *blockCtrl)
{
}


void
CopyPasteUIWayland::GetRemoteClipboardCB(const CPClipboard *clip)
{
   if (!clip || !mCP) {
      return;
   }

   if (clip->format == CPFORMAT_TEXT) {
      mClipboardEmpty = false;
   }
}


void
CopyPasteUIWayland::GetLocalClipboard(void)
{
   if (mCP) {
      SendClipNotChanged();
   }
}


void
CopyPasteUIWayland::SendClipNotChanged(void)
{
   if (mCP) {
      mCP->DestUISendClipNotChanged();
   }
}