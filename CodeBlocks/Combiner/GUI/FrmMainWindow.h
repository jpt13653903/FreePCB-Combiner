//==============================================================================
// FreePCB Combiner Main Window Header
//
// Copyright (C) John-Philip Taylor
// jpt13653903@gmail.com
//
// This file is part of FreePCB Combiner
//
// FreePCB Combiner is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>
//==============================================================================

#ifndef FRMMAINWINDOW_H
#define FRMMAINWINDOW_H
//------------------------------------------------------------------------------

#include "winWindow.h"
#include "winButton.h"
#include "winMemo.h"
#include "winList.h"
#include "resource.h"
//------------------------------------------------------------------------------

#include "FrmGetQuantity.h"
//------------------------------------------------------------------------------

#include "FreePCB.h"
#include "Netlist.h"
//------------------------------------------------------------------------------

class JFrmMainWindow : public winWindow{
 private:
  winList  * FileList;
  winButton* AddEntry;
  winButton* RemoveEntry;
  winButton* EditEntry;
  winButton* FixValues;
  winButton* HideReferences;
  winButton* ShowReferences;
  winButton* ReferencesToValues;
  winButton* ReferencesToText;
  winButton* Combine;
  winButton* About;
  winMemo  * Messages;

  LRESULT OnCommand(WPARAM wParam, LPARAM lParam);
  LRESULT OnNotify (WPARAM wParam, LPARAM lParam);
  void    OnResize (int ClientWidth, int ClientHeight);

  void fixvalues(const char* PCB, const char* Netlist);

  void AddEntryClick          ();
  void RemoveEntryClick       ();
  void EditEntryClick         ();
  void FixValuesClick         ();
  void HideReferencesClick    ();
  void ShowReferencesClick    ();
  void ReferencesToValuesClick();
  void ReferencesToTextClick  ();
  void CombineClick           ();
  void AboutClick             ();

  int  ToInt     (const char* String);
  void MakePrefix(int Count, JString* Prefix);

 public:
  JFrmMainWindow(
   WindowType  Type,
   int         Left, // MAIN_SCREEN_CENTER => screen center
   int         Top,  // MAIN_SCREEN_CENTER => screen center
   int         Width,
   int         Height,
   const char* Caption,
   HWND        Parent  = HWND_DESKTOP,
   bool        TaskBar = true
  );
  ~JFrmMainWindow();

  bool OnMessage(UINT message, WPARAM wParam, LPARAM lParam);
};
//------------------------------------------------------------------------------

#endif
//------------------------------------------------------------------------------
