//==============================================================================
// Get Board Quantity Dialogue Box
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

/** To Do:
  * - Make this a proper dialogue box using CreateDialog(...)
  */
//------------------------------------------------------------------------------

#include "FrmGetQuantity.h"
//------------------------------------------------------------------------------

JFrmGetQuantity::JFrmGetQuantity(
 WindowType  Type,
 int         Left,
 int         Top,
 int         Width,
 int         Height,
 const char* Caption,
 HWND        Parent,
 bool        TaskBar
): winWindow(
 Type,
 Left,
 Top,
 Width,
 Height,
 Caption,
 Parent,
 TaskBar
){
 SetIcon(MainIcon);

 // Create the edit box
 Edit = new winEdit(
  this,
  8,
  8,
  100,
  21
 );
 Handles.Add(Edit->Handle);

 // Create the buttons
 Ok = new winButton(
  this,
  "Ok",
   8, // Left
  33, // Top
  50, // Width
  21  // Height
 );
 Handles.Add(Ok->Handle);

 Cancel = new winButton(
  this,
  "Cancel",
   8, // Left
  33, // Top
  50, // Width
  21  // Height
 );
 Handles.Add(Cancel->Handle);

 Handles.Add(Handle);

 OnResize(GetClientWidth(), GetClientHeight());
}
//------------------------------------------------------------------------------

JFrmGetQuantity::~JFrmGetQuantity(){
 delete Edit;
 delete Ok;
 delete Cancel;
}
//------------------------------------------------------------------------------

bool JFrmGetQuantity::OnMessage(UINT message, WPARAM wParam, LPARAM lParam){
 RECT* Rect;
 int   WidthBias, HeightBias;

 switch (message){
  case WM_COMMAND:
   OnCommand(wParam, lParam);
   return true;

  case WM_NOTIFY:
   OnNotify (wParam, lParam);
   return true;

  case WM_SIZING:
   WidthBias  = GetWidth () - GetClientWidth ();
   HeightBias = GetHeight() - GetClientHeight();
   Rect = (RECT*)lParam;
   if(Rect->right - Rect->left < 250 + WidthBias){
    Rect->right = Rect->left + 250 + WidthBias;
   }
   Rect->bottom = Rect->top + 62 + HeightBias;
   break;

  case WM_SIZE:
   OnResize(LOWORD(lParam), HIWORD(lParam));
   break;

  case WM_DESTROY:
   CancelClick();
   return true;

  default:
   break;
 }

 return false;
}
//------------------------------------------------------------------------------

void JFrmGetQuantity::OkClick(){
 Done = true;
}
//------------------------------------------------------------------------------

void JFrmGetQuantity::CancelClick(){
 Cancelled = true;
}
//------------------------------------------------------------------------------

int JFrmGetQuantity::ToInt(const char* String){
 int i, j;

 i = 0;
 for(j = 0; String[j]; j++){
  i = 10*i + String[j] - '0';
 }

 return i;
}
//------------------------------------------------------------------------------

LRESULT JFrmGetQuantity::OnCommand(WPARAM wParam, LPARAM lParam){
 JString s;

 if((HWND)lParam == Ok->Handle){
  if(HIWORD(wParam) == BN_CLICKED){
   OkClick();
  }

 }else if((HWND)lParam == Cancel->Handle){
  if(HIWORD(wParam) == BN_CLICKED){
   CancelClick();
  }
 }

 return 0;
}
//------------------------------------------------------------------------------

LRESULT JFrmGetQuantity::OnNotify(WPARAM wParam, LPARAM lParam){
 return 0;
}
//------------------------------------------------------------------------------

void JFrmGetQuantity::OnResize(int ClientWidth, int ClientHeight){
 int i;

 i = (ClientWidth - 20) / 2;

 Edit  ->SetWidth(ClientWidth - 16);
 Ok    ->SetWidth(i);
 Cancel->SetWidth(i);

 Cancel->SetLeft(ClientWidth - 8 - i);

 Redraw();
}
//------------------------------------------------------------------------------

bool JFrmGetQuantity::GetQuantity(JString* Quantity){
 int i;
 MSG Message;

 Show();

 Cancelled = false;
 Done      = false;

 SetFocus(Edit->Handle);

 while(!Cancelled && !Done && GetMessage(&Message, NULL, 0, 0)){
  if(Handles.Find(Message.hwnd)){ // Message is for this window
   TranslateMessage(&Message);
   DispatchMessage (&Message);
  }else{
   /* Flash the window to get the user's attention */
  }
 }

 if(Cancelled) return false;

 Edit->GetText(Quantity);
 i = roundl(Calc.Calculate(Quantity->String));
 Quantity->Set(i);
 return true;
}
//------------------------------------------------------------------------------
