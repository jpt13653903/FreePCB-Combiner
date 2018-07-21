//==============================================================================
// The Main Function
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

#include "winApplication.h"
#include "FrmMainWindow.h"
//------------------------------------------------------------------------------

int WINAPI WinMain(
 HINSTANCE hInstance,
 HINSTANCE hPrevInstance,
 LPSTR     lpCmdLine,
 int       nCmdShow
){
 Application = new winApplication(
  hInstance,
  hPrevInstance,
  lpCmdLine,
  nCmdShow,
  "FreePCB Combiner"
 );

 JFrmMainWindow* MainWindow = new JFrmMainWindow(
  winWindow::Frame,
  MAIN_SCREEN_CENTER,
  MAIN_SCREEN_CENTER,
  640,
  480,
  "FreePCB Combiner"
 );
 MainWindow->Show();

 // Run the message loop
 int Result = Application->Run();

 delete MainWindow;
 delete Application;

 return Result;
}
//------------------------------------------------------------------------------
