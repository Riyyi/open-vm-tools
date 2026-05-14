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
#include "guestCopyPaste.hh"
#include "guestDnDCPMgr.hh"

extern "C" {
#include "dndClipboard.h"
}

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

mCP->srcRecvClipChanged.connect(
      sigc::mem_fun(this, &CopyPasteUIWayland::GetRemoteClipboardCB));
   mCP->destRequestClipChanged.connect(
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

   if (!CPClipboard_ItemExists(clip, CPFORMAT_TEXT)) {
      return;
   }

   void *buf = nullptr;
   size_t sz = 0;
   if (!CPClipboard_GetItem(clip, CPFORMAT_TEXT, &buf, &sz)) {
      return;
   }

   std::string text(static_cast<const char *>(buf), sz);
   mClipboardEmpty = false;

   Glib::RefPtr<Gdk::Display> display = Gdk::Display::get_default();
   if (!display) {
      return;
   }

   Glib::RefPtr<Gdk::Clipboard> clipboard = display->get_clipboard();
   if (!clipboard) {
      return;
   }

#if GTKMM_VERSION_MAJOR >= 4
   clipboard->set_text(text);
#else
   clipboard->set_text(text);
#endif
}


void
CopyPasteUIWayland::GetLocalClipboard(void)
{
   if (!mCP) {
      return;
   }

   Glib::RefPtr<Gdk::Display> display = Gdk::Display::get_default();
   if (!display) {
      SendClipNotChanged();
      return;
   }

   Glib::RefPtr<Gdk::Clipboard> clipboard = display->get_clipboard();
   if (!clipboard) {
      SendClipNotChanged();
      return;
   }

   clipboard->read_text_async(sigc::bind(
      sigc::mem_fun(this, &CopyPasteUIWayland::OnReadTextAsync),
      clipboard));
}


void
CopyPasteUIWayland::OnReadTextAsync(const Glib::RefPtr<Gdk::Clipboard> &clipboard,
                                     const Glib::ustring &text)
{
   if (!mCP) {
      return;
   }

   if (text.empty()) {
      SendClipNotChanged();
      return;
   }

   CPClipboard clip;
   CPClipboard_Init(&clip);

   const char *textStr = text.c_str();
   size_t textLen = text.length();

   if (!CPClipboard_SetItem(&clip, CPFORMAT_TEXT, textStr, textLen)) {
      CPClipboard_Destroy(&clip);
      SendClipNotChanged();
      return;
   }

   mCP->DestUISendClip(&clip);
   CPClipboard_Destroy(&clip);
}


void
CopyPasteUIWayland::SendClipNotChanged(void)
{
   if (mCP) {
      mCP->DestUISendClipNotChanged();
   }
}