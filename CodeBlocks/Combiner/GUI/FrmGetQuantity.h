//==============================================================================
// Get Board Quantity Dialogue Box Header
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

#ifndef FRMGETQUANTITY_H
#define FRMGETQUANTITY_H
//------------------------------------------------------------------------------

#include "winWindow.h"
#include "winButton.h"
#include "winEdit.h"
#include "resource.h"
#include "JData.h"
#include "JCalc.h"
//------------------------------------------------------------------------------

class JFrmGetQuantity : public winWindow{
 private:
  winEdit  * Edit;
  winButton* Ok;
  winButton* Cancel;

  JData Handles;
  JCalc Calc;

  bool Cancelled;
  bool Done;

  LRESULT OnCommand(WPARAM wParam, LPARAM lParam);
  LRESULT OnNotify (WPARAM wParam, LPARAM lParam);
  void    OnResize (int ClientWidth, int ClientHeight);

  int ToInt(const char* String);

  void OkClick();
  void CancelClick();

 public:
  JFrmGetQuantity(
   WindowType  Type,
   int         Left, // MAIN_SCREEN_CENTER => screen center
   int         Top,  // MAIN_SCREEN_CENTER => screen center
   int         Width,
   int         Height,
   const char* Caption,
   HWND        Parent  = HWND_DESKTOP,
   bool        TaskBar = true
  );
  ~JFrmGetQuantity();

  bool GetQuantity(JString* Quantity);

  bool OnMessage(UINT message, WPARAM wParam, LPARAM lParam);
};
//------------------------------------------------------------------------------

#endif
//------------------------------------------------------------------------------
