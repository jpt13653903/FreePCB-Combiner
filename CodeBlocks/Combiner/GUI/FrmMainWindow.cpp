//==============================================================================
// FreePCB Combiner Main Window
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

/** To do:
  * - Option to fix the drill sizes so that they are
  *   all to the nearest size in a given list
  * - Option to change the stroke width and text height of all
  *   references and values
  * - Implement all the "/ Implement /" blocks
  */

//------------------------------------------------------------------------------

#include "FrmMainWindow.h"
//------------------------------------------------------------------------------

winMemo* MessageMemo;

static void PostMessage(
 MessageType Type,
 const char* File,
 int         Line,
 const char* Message
){
 JString s;
 switch(Type){
  case mtInfo:
   s.Set("Info: ");
   break;

  case mtWarning:
   s.Set("Warning: ");
   break;

  case mtError:
   s.Set("Error: ");
   break;

  default:
   s.Set("Unknown: ");
   break;
 }

 s.Append('"');
 s.Append(File);
 s.Append('"');
 if(Line){
  s.Append(", Line ");
  s.Append(Line);
 }
 s.Append(": ");

 s.Append(Message);

 MessageMemo->AddLine(s.String);
}
//------------------------------------------------------------------------------

JFrmMainWindow::JFrmMainWindow(
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

 // Create the file list
 FileList = new winList(
  this,
    8, // Left
    8, // Top
  400, // Width
  100  // Height
 );

 FileList->HeaderAdd("FreePCB File", 100, DT_PATH_ELLIPSIS);
 FileList->HeaderAdd("Netlist"     , 100, DT_PATH_ELLIPSIS);
 FileList->HeaderAdd("Quantity"    ,  60                  );

 // Create the buttons
 AddEntry = new winButton(
  this,
  "Add Entry",
    8, // Left
    8, // Top
  400, // Width
   21  // Height
 );

 RemoveEntry = new winButton(
  this,
  "Remove Entry",
    8, // Left
    8, // Top
  400, // Width
   21  // Height
 );

 EditEntry = new winButton(
  this,
  "Edit Entry",
    8, // Left
    8, // Top
  400, // Width
   21  // Height
 );

 FixValues = new winButton(
  this,
  "Fix Values",
    8, // Left
    8, // Top
  400, // Width
   21  // Height
 );

 HideReferences = new winButton(
  this,
  "Hide References",
    8, // Left
    8, // Top
  400, // Width
   21  // Height
 );

 ShowReferences = new winButton(
  this,
  "Show References",
    8, // Left
    8, // Top
  400, // Width
   21  // Height
 );

 ReferencesToValues = new winButton(
  this,
  "References to Values",
    8, // Left
    8, // Top
  400, // Width
   21  // Height
 );

 ReferencesToText = new winButton(
  this,
  "References to Text",
    8, // Left
    8, // Top
  400, // Width
   21  // Height
 );

 Combine = new winButton(
  this,
  "Combine",
    8, // Left
    8, // Top
  400, // Width
   21  // Height
 );

 About = new winButton(
  this,
  "About FreePCB Combiner",
    8, // Left
    8, // Top
  400, // Width
   21  // Height
 );

 // Create the message box
 Messages = new winMemo(
  this,
    8, // Left
    8, // Top
  400, // Width
  100  // Height
 );
 MessageMemo = Messages;

 OnResize(GetClientWidth(), GetClientHeight());
}
//------------------------------------------------------------------------------

JFrmMainWindow::~JFrmMainWindow(){
 delete FileList;
 delete AddEntry;
 delete RemoveEntry;
 delete EditEntry;
 delete FixValues;
 delete HideReferences;
 delete ShowReferences;
 delete ReferencesToValues;
 delete ReferencesToText;
 delete Combine;
 delete About;
 delete Messages;
}
//------------------------------------------------------------------------------

bool JFrmMainWindow::OnMessage(UINT message, WPARAM wParam, LPARAM lParam){
 RECT* Rect;

 switch (message){
  case WM_COMMAND:
   OnCommand(wParam, lParam);
   return true;

  case WM_NOTIFY:
   OnNotify (wParam, lParam);
   return true;

  case WM_SIZING:
   Rect = (RECT*)lParam;
   if(Rect->right  - Rect->left < 400) Rect->right  = Rect->left + 400;
   if(Rect->bottom - Rect->top  < 400) Rect->bottom = Rect->top  + 400;
   break;

  case WM_SIZE:
   OnResize(LOWORD(lParam), HIWORD(lParam));
   break;

  case WM_DESTROY:
   PostQuitMessage(0);
   return true;

  default:
   break;
 }

 return false;
}
//------------------------------------------------------------------------------

void JFrmMainWindow::AddEntryClick(){
 int     j, q;
 int     w, h;
 char*   Filename;
 char*   Item;
 JString PCBFile;
 JString Netlist;
 JString Quantity;

 Messages->SetText("");

 Filename = new char[0x1000];
 if(FileList->GetItemCount()){
  Item = FileList->GetItemText(0, 0);
  for(j = 0; Item[j]; j++){
   Filename[j] = Item[j];
  }
  Filename[j] = 0;
 }else{
  Filename[0] = 0;
 }

 OPENFILENAME BoxProperties;

 BoxProperties.lStructSize       = sizeof(OPENFILENAME);
 BoxProperties.hwndOwner         = Handle;
 BoxProperties.hInstance         = 0;
 BoxProperties.lpstrFilter       = "FreePCB File\0*.fpc\0\0";
 BoxProperties.lpstrCustomFilter = 0;
 BoxProperties.nMaxCustFilter    = 0;
 BoxProperties.nFilterIndex      = 0;
 BoxProperties.lpstrFile         = Filename;
 BoxProperties.nMaxFile          = 0x1000;
 BoxProperties.lpstrFileTitle    = 0;
 BoxProperties.nMaxFileTitle     = 0;
 BoxProperties.lpstrInitialDir   = 0;
 BoxProperties.lpstrTitle        = "Open FreePCB File";
 BoxProperties.Flags             = OFN_ENABLESIZING  |
                                   OFN_FILEMUSTEXIST |
                                   OFN_PATHMUSTEXIST ;
 BoxProperties.nFileOffset       = 0;
 BoxProperties.nFileExtension    = 0;
 BoxProperties.lpstrDefExt       = ".fpc";
 BoxProperties.lCustData         = 0;
 BoxProperties.lpfnHook          = 0;
 BoxProperties.lpTemplateName    = 0;
 BoxProperties.pvReserved        = 0;
 BoxProperties.dwReserved        = 0;
 BoxProperties.FlagsEx           = 0;

 if(GetOpenFileName(&BoxProperties)){
  PCBFile.Set(Filename);
 }else{
  delete[] Filename;
  return;
 }

 q = -1;
 for(j = 0; Filename[j]; j++){
  if(Filename[j] == '.') q = j;
 }
 if(q > 0){
  q++;
  Filename[q++] = 'n';
  Filename[q++] = 'e';
  Filename[q++] = 't';
  Filename[q++] = 0;
 }

 BoxProperties.lpstrFilter = "Netlist\0*.net\0\0";
 BoxProperties.lpstrFile   = Filename;
 BoxProperties.nMaxFile    = 0x1000;
 BoxProperties.lpstrTitle  = "Open Netlist";
 BoxProperties.lpstrDefExt = ".net";

 if(GetOpenFileName(&BoxProperties)){
  Netlist.Set(Filename);
 }else{
  delete[] Filename;
  return;
 }
 delete[] Filename;

 w = 250 - GetClientWidth () + GetWidth ();
 h =  62 - GetClientHeight() + GetHeight();

 JFrmGetQuantity* FrmQuantity;
 FrmQuantity = new JFrmGetQuantity(
  Frame,
  GetLeft() + (GetWidth () - w) / 2,
  GetTop () + (GetHeight() - h) / 2,
  w,
  h,
  "Board quantity",
  Handle,
  true
 );
 if(FrmQuantity->GetQuantity(&Quantity)){
  j = FileList->GetItemCount();
  FileList->ItemAdd(0);
  FileList->SetItemText(j, 0, PCBFile .String);
  FileList->SetItemText(j, 1, Netlist .String);
  FileList->SetItemText(j, 2, Quantity.String);
  FileList->Refresh();
 }
 delete FrmQuantity;
}
//------------------------------------------------------------------------------

void JFrmMainWindow::RemoveEntryClick(){
 /* Implement */
 FileList->Clear();
 FileList->Refresh();
}
//------------------------------------------------------------------------------

void JFrmMainWindow::EditEntryClick(){
 /* Implement */
}
//------------------------------------------------------------------------------

void JFrmMainWindow::fixvalues(const char* PCB, const char* Netlist){
 JString s;

 FREEPCB PCBFile;
 NETLIST NetFile;

 FREEPCB::PART* PCBPart;
 NETLIST::PART* NetPart;
 NETLIST::PART  Key;

 PCBFile.PostMessage = PostMessage;
 NetFile.PostMessage = PostMessage;

 if(PCBFile.LoadFile(PCB)){
  if(NetFile.LoadFile(Netlist)){
   PCBPart = PCBFile.Parts;
   while(PCBPart){
    Key.Reference.Set(PCBPart->Reference.String);
    NetPart = (NETLIST::PART*)NetFile.Parts.Find(&Key);
    if(NetPart){
     if(PCBPart->Value.Value.Compare(NetPart->Value.String)){
      s.Set   ("Changing value of part ");
      s.Append(PCBPart->Reference.String);
      s.Append(" to ");
      s.Append(NetPart->Value.String);
      PostMessage(
       mtInfo,
       PCB,
       0,
       s.String
      );
      PCBPart->Value.Value.Set(NetPart->Value.String);
     }
     if(PCBPart->Value.Value.GetLength()){
      if(!PCBPart->Value.Visibility){
       s.Set   ("Showing the value of part ");
       s.Append(PCBPart->Reference.String);
       PostMessage(
        mtInfo,
        PCB,
        0,
        s.String
       );
       PCBPart->Value.Visibility = true;
      }
      if(PCBPart->Value.Height < 254000){ // 10 mil
       s.Set   ("Changing height of the value of part ");
       s.Append(PCBPart->Reference.String);
       s.Append(" to 40 mil");
       PostMessage(
        mtInfo,
        PCB,
        0,
        s.String
       );
       PCBPart->Value.Height = 1016000; // 40 mil
      }
      if(PCBPart->Value.LineWidth < 101600){ // 4 mil
       s.Set   ("Changing line width of the value of part ");
       s.Append(PCBPart->Reference.String);
       s.Append(" to 4 mil");
       PostMessage(
        mtInfo,
        PCB,
        0,
        s.String
       );
       PCBPart->Value.LineWidth = 101600; // 4 mil
      }
     }
    }
    PCBPart = PCBPart->Next;
   }

   if(PCBFile.SaveFile(PCB)){
    PostMessage(
     mtInfo,
     PCB,
     0,
     "Values fixed and shown."
    );
   }
  }
 }
}
//------------------------------------------------------------------------------

void JFrmMainWindow::FixValuesClick(){
 int i;

 Messages->SetText("");

 for(i = 0; i < FileList->GetItemCount(); i++){
  fixvalues(
   FileList->GetItemText(i, 0),
   FileList->GetItemText(i, 1)
  );
 }
}
//------------------------------------------------------------------------------

void JFrmMainWindow::HideReferencesClick(){
 int i;
 char*   PCB;
 FREEPCB PCBFile;

 PCBFile.PostMessage = PostMessage;

 Messages->SetText("");

 for(i = 0; i < FileList->GetItemCount(); i++){
  PCB = FileList->GetItemText(i, 0);
  if(PCBFile.LoadFile(PCB)){
   PCBFile.HideReferences();
   if(PCBFile.SaveFile(PCB)){
    PostMessage(
     mtInfo,
     PCB,
     0,
     "References hidden."
    );
   }
  }
 }
}
//------------------------------------------------------------------------------

void JFrmMainWindow::ShowReferencesClick(){
 int i;
 char*   PCB;
 FREEPCB PCBFile;

 PCBFile.PostMessage = PostMessage;

 Messages->SetText("");

 for(i = 0; i < FileList->GetItemCount(); i++){
  PCB = FileList->GetItemText(i, 0);
  if(PCBFile.LoadFile(PCB)){
   PCBFile.ShowReferences();
   if(PCBFile.SaveFile(PCB)){
    PostMessage(
     mtInfo,
     PCB,
     0,
     "References shown."
    );
   }
  }
 }
}
//------------------------------------------------------------------------------

void JFrmMainWindow::ReferencesToValuesClick(){
 int i;
 char*   PCB;
 FREEPCB PCBFile;

 PCBFile.PostMessage = PostMessage;

 Messages->SetText("");

 for(i = 0; i < FileList->GetItemCount(); i++){
  PCB = FileList->GetItemText(i, 0);
  if(PCBFile.LoadFile(PCB)){
   PCBFile.ReferencesToValues();
   if(PCBFile.SaveFile(PCB)){
    PostMessage(
     mtInfo,
     PCB,
     0,
     "Visible references converted to values and hidden."
    );
   }
  }
 }
}
//------------------------------------------------------------------------------

void JFrmMainWindow::ReferencesToTextClick(){
 int i;
 char*   PCB;
 FREEPCB PCBFile;

 PCBFile.PostMessage = PostMessage;

 Messages->SetText("");

 for(i = 0; i < FileList->GetItemCount(); i++){
  PCB = FileList->GetItemText(i, 0);
  if(PCBFile.LoadFile(PCB)){
   PCBFile.ReferencesToText();
   if(PCBFile.SaveFile(PCB)){
    PostMessage(
     mtInfo,
     PCB,
     0,
     "Visible references converted to text and hidden."
    );
   }
  }
 }
}
//------------------------------------------------------------------------------

int JFrmMainWindow::ToInt(const char* String){
 int i, j;

 i = 0;
 for(j = 0; String[j]; j++){
  i = 10*i + String[j] - '0';
 }

 return i;
}
//------------------------------------------------------------------------------

void JFrmMainWindow::MakePrefix(int Count, JString* Prefix){
 Prefix->Set("");

 if(!Count) Prefix->Set('A');

 while(Count){
  Prefix->Append((char)((Count % 26) + 'A'));
  Count /= 26;
 }
 Prefix->Reverse();
 Prefix->Append('_');
}
//------------------------------------------------------------------------------

void JFrmMainWindow::CombineClick(){
 int      j, q, c;
 bool     b;
 char*    Filename;
 char*    Item;
 JFile    File;
 JString  Buffer;
 int      BoardIndex;
 int      PrefixCount;
 JString  FootprintPrefix;
 JString  PartPrefix;
 FREEPCB* PCB;

 FREEPCB::RECT Rect;
 int           X, MaxX, XOffset;
 int           Y, MaxY, YOffset;

     MaxY    = 190000000; // 190 mm
 int Spacing =   3000000; //   3 mm

 Messages->SetText("");

 if(!FileList->GetItemCount()) return;

 Filename = new char[0x1000];
 Item = FileList->GetItemText(0, 0);
 q = -1;
 for(j = 0; Item[j]; j++){
  Filename[j] = Item[j];
  if(Item[j] == '\\') q = j;
 }
 if(q > 0){
  q++;
  Filename[q++] = 'C';
  Filename[q++] = 'o';
  Filename[q++] = 'm';
  Filename[q++] = 'b';
  Filename[q++] = 'i';
  Filename[q++] = 'n';
  Filename[q++] = 'e';
  Filename[q++] = 'd';
  Filename[q++] = 0;
 }else{
  Filename[j] = 0;
 }

 OPENFILENAME BoxProperties;

 BoxProperties.lStructSize       = sizeof(OPENFILENAME);
 BoxProperties.hwndOwner         = Handle;
 BoxProperties.hInstance         = 0;
 BoxProperties.lpstrFilter       = "FreePCB File\0*.fpc\0\0";
 BoxProperties.lpstrCustomFilter = 0;
 BoxProperties.nMaxCustFilter    = 0;
 BoxProperties.nFilterIndex      = 0;
 BoxProperties.lpstrFile         = Filename;
 BoxProperties.nMaxFile          = 0x1000;
 BoxProperties.lpstrFileTitle    = 0;
 BoxProperties.nMaxFileTitle     = 0;
 BoxProperties.lpstrInitialDir   = 0;
 BoxProperties.lpstrTitle        = "Save combination as...";
 BoxProperties.Flags             = OFN_ENABLESIZING    |
                                   OFN_OVERWRITEPROMPT |
                                   OFN_PATHMUSTEXIST   ;
 BoxProperties.nFileOffset       = 0;
 BoxProperties.nFileExtension    = 0;
 BoxProperties.lpstrDefExt       = ".fpc";
 BoxProperties.lCustData         = 0;
 BoxProperties.lpfnHook          = 0;
 BoxProperties.lpTemplateName    = 0;
 BoxProperties.pvReserved        = 0;
 BoxProperties.dwReserved        = 0;
 BoxProperties.FlagsEx           = 0;

 if(GetSaveFileName(&BoxProperties)){
  PCB = new FREEPCB[FileList->GetItemCount()];
  for(j = 0; j < FileList->GetItemCount(); j++){
   PCB[j].PostMessage = PostMessage;
   if(!PCB[j].LoadFile(FileList->GetItemText(j, 0))){
    delete[] PCB;
    delete[] Filename;
    return;
   }
  }

  File.SetFilename(BoxProperties.lpstrFile);
  if(File.Open(JFile::Create)){
   Buffer.Set("[options]\r\n");
   PCB[0].SaveOptions(&Buffer);

   Buffer.Append("\r\n[footprints]\r\n");
   for(j = 0; j < FileList->GetItemCount(); j++){
    MakePrefix(j, &FootprintPrefix);
    PCB[j].SaveFootprints(&Buffer, FootprintPrefix.String);
   }

   BoardIndex = 0;
   X          = 0;
   MaxX       = 0;
   Y          = 0;
   Buffer.Append("\r\n[board]\r\n");
   for(j = 0; j < FileList->GetItemCount(); j++){
    q = ToInt(FileList->GetItemText(j, 2));
    PCB[j].GetRect(&Rect);
    for(c = 0; c < q; c++){
     if(Y - Rect.Bottom + Rect.Height > MaxY){
      X = MaxX + Spacing;
      Y = 0;
     }
     XOffset = X - Rect.Left;
     YOffset = Y - Rect.Bottom;
     PCB[j].SaveBoard(&Buffer, BoardIndex, XOffset, YOffset);
     FREEPCB::BOARD* Temp = PCB[j].BoardOutline;
     while(Temp){
      BoardIndex++;
      Temp = Temp->Next;
     }
     Y += Rect.Height + Spacing;
     if(X + Rect.Width > MaxX) MaxX = X + Rect.Width;
    }
   }

   X    = 0;
   MaxX = 0;
   Y    = 0;
   Buffer.Append("\r\n[solder_mask_cutouts]\r\n");
   for(j = 0; j < FileList->GetItemCount(); j++){
    q = ToInt(FileList->GetItemText(j, 2));
    PCB[j].GetRect(&Rect);
    for(c = 0; c < q; c++){
     if(Y - Rect.Bottom + Rect.Height > MaxY){
      X = MaxX + Spacing;
      Y = 0;
     }
     XOffset = X - Rect.Left;
     YOffset = Y - Rect.Bottom;
     PCB[j].SaveSolderMask(&Buffer, XOffset, YOffset);
     Y += Rect.Height + Spacing;
     if(X + Rect.Width > MaxX) MaxX = X + Rect.Width;
    }
   }

   PrefixCount = 0;
   X    = 0;
   MaxX = 0;
   Y    = 0;
   Buffer.Append("\r\n[parts]\r\n");
   for(j = 0; j < FileList->GetItemCount(); j++){
    MakePrefix(j, &FootprintPrefix);
    q = ToInt(FileList->GetItemText(j, 2));
    PCB[j].GetRect(&Rect);
    for(c = 0; c < q; c++){
     if(Y - Rect.Bottom + Rect.Height > MaxY){
      X = MaxX + Spacing;
      Y = 0;
     }
     XOffset = X - Rect.Left;
     YOffset = Y - Rect.Bottom;
     MakePrefix(PrefixCount, &PartPrefix);
     PCB[j].SaveParts(
      &Buffer,
      FootprintPrefix.String,
      PartPrefix     .String,
      XOffset,
      YOffset
     );
     PrefixCount++;
     Y += Rect.Height + Spacing;
     if(X + Rect.Width > MaxX) MaxX = X + Rect.Width;
    }
   }

   PrefixCount = 0;
   X    = 0;
   MaxX = 0;
   Y    = 0;
   Buffer.Append("\r\n[nets]\r\n");
   for(j = 0; j < FileList->GetItemCount(); j++){
    q = ToInt(FileList->GetItemText(j, 2));
    PCB[j].GetRect(&Rect);
    for(c = 0; c < q; c++){
     if(Y - Rect.Bottom + Rect.Height > MaxY){
      X = MaxX + Spacing;
      Y = 0;
     }
     XOffset = X - Rect.Left;
     YOffset = Y - Rect.Bottom;
     MakePrefix(PrefixCount, &PartPrefix);
     PCB[j].SaveNets(&Buffer, PartPrefix.String, XOffset, YOffset);
     PrefixCount++;
     Y += Rect.Height + Spacing;
     if(X + Rect.Width > MaxX) MaxX = X + Rect.Width;
    }
   }

   X    = 0;
   MaxX = 0;
   Y    = 0;
   Buffer.Append("\r\n[texts]\r\n");
   for(j = 0; j < FileList->GetItemCount(); j++){
    q = ToInt(FileList->GetItemText(j, 2));
    PCB[j].GetRect(&Rect);
    for(c = 0; c < q; c++){
     if(Y - Rect.Bottom + Rect.Height > MaxY){
      X = MaxX + Spacing;
      Y = 0;
     }
     XOffset = X - Rect.Left;
     YOffset = Y - Rect.Bottom;
     PCB[j].SaveTexts(&Buffer, XOffset, YOffset);
     Y += Rect.Height + Spacing;
     if(X + Rect.Width > MaxX) MaxX = X + Rect.Width;
    }
   }

   Buffer.Append("\r\n[end]\r\n");

   File.WriteBuffer(Buffer.String, Buffer.GetLength(), &b);

   delete[] PCB;
   File.Close();

  }else{
   PostMessage(
    mtError,
    File.GetFilename(),
    0,
    "File could not be opened"
   );
   delete[] Filename;
   return;
  }
  PostMessage(
   mtInfo,
   File.GetFilename(),
   0,
   "File combination successful."
  );
 }

 delete[] Filename;
}
//------------------------------------------------------------------------------

void JFrmMainWindow::AboutClick(){
 JString s;

 s.Set   ("FreePCB Combiner (pre-release version)\n"
          "Copyright (C) John-Philip Taylor\n"
          "jpt13653903@gmail.com\n"
          "\n"
          "Build ");
 s.Append(__TIME__);
 s.Append(" ");
 s.Append(__DATE__);
 s.Append("\n"
   "\n"
   "This program is free software: you can redistribute it and/or modify\n"
   "it under the terms of the GNU General Public License as published by\n"
   "the Free Software Foundation, either version 3 of the License, or\n"
   "(at your option) any later version.\n"
   "\n"
   "This program is distributed in the hope that it will be useful,\n"
   "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
   "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
   "GNU General Public License for more details.\n"
   "\n"
   "You should have received a copy of the GNU General Public License\n"
   "along with this program.  If not, see <http://www.gnu.org/licenses/>");

 MessageBox(Handle, s.String, "FreePCB Combiner", MB_ICONINFORMATION);
}
//------------------------------------------------------------------------------

LRESULT JFrmMainWindow::OnCommand(WPARAM wParam, LPARAM lParam){
 JString s;

 if((HWND)lParam == AddEntry->Handle){
  if(HIWORD(wParam) == BN_CLICKED){
   AddEntryClick();
  }

 }else if((HWND)lParam == RemoveEntry->Handle){
  if(HIWORD(wParam) == BN_CLICKED){
   RemoveEntryClick();
  }

 }else if((HWND)lParam == EditEntry->Handle){
  if(HIWORD(wParam) == BN_CLICKED){
   EditEntryClick();
  }

 }else if((HWND)lParam == FixValues->Handle){
  if(HIWORD(wParam) == BN_CLICKED){
   FixValuesClick();
  }

 }else if((HWND)lParam == HideReferences->Handle){
  if(HIWORD(wParam) == BN_CLICKED){
   HideReferencesClick();
  }

 }else if((HWND)lParam == ShowReferences->Handle){
  if(HIWORD(wParam) == BN_CLICKED){
   ShowReferencesClick();
  }

 }else if((HWND)lParam == ReferencesToValues->Handle){
  if(HIWORD(wParam) == BN_CLICKED){
   ReferencesToValuesClick();
  }

 }else if((HWND)lParam == ReferencesToText->Handle){
  if(HIWORD(wParam) == BN_CLICKED){
   ReferencesToTextClick();
  }

 }else if((HWND)lParam == Combine->Handle){
  if(HIWORD(wParam) == BN_CLICKED){
   CombineClick();
  }

 }else if((HWND)lParam == About->Handle){
  if(HIWORD(wParam) == BN_CLICKED){
   AboutClick();
  }
 }

 return 0;
}
//------------------------------------------------------------------------------

LRESULT JFrmMainWindow::OnNotify(WPARAM wParam, LPARAM lParam){
 return 0;
}
//------------------------------------------------------------------------------

void JFrmMainWindow::OnResize(int ClientWidth, int ClientHeight){
 int i;

 i = (ClientWidth - 16) / 3;
 FileList          ->SetWidth(ClientWidth - 16);
 AddEntry          ->SetWidth(i);
 RemoveEntry       ->SetWidth(ClientWidth - 24 - 2*i);
 EditEntry         ->SetWidth(i);
 FixValues         ->SetWidth(i);
 HideReferences    ->SetWidth(ClientWidth - 24 - 2*i);
 ShowReferences    ->SetWidth(i);
 ReferencesToValues->SetWidth(i);
 ReferencesToText  ->SetWidth(ClientWidth - 24 - 2*i);
 Combine           ->SetWidth(i);
 About             ->SetWidth(ClientWidth - 16);
 Messages          ->SetWidth(ClientWidth - 16);

 RemoveEntry     ->SetLeft(             12 + i);
 EditEntry       ->SetLeft(ClientWidth - 8 - i);
 HideReferences  ->SetLeft(             12 + i);
 ShowReferences  ->SetLeft(ClientWidth - 8 - i);
 ReferencesToText->SetLeft(             12 + i);
 Combine         ->SetLeft(ClientWidth - 8 - i);

 i = (FileList->GetWidth() - FileList->GetHeaderWidth(2)) / 2;
 FileList->SetHeaderWidth(0, i);
 FileList->SetHeaderWidth(1, i);

 i = ClientHeight / 4;
 if(i < 100) i = 100;
 Messages->SetHeight(i);

 i = ClientHeight - 120 - i;
 FileList->SetHeight(i);
 i += 12;
 AddEntry          ->SetTop(i);
 RemoveEntry       ->SetTop(i);
 EditEntry         ->SetTop(i);
 i += 25;
 FixValues         ->SetTop(i);
 HideReferences    ->SetTop(i);
 ShowReferences    ->SetTop(i);
 i += 25;
 ReferencesToValues->SetTop(i);
 ReferencesToText  ->SetTop(i);
 Combine           ->SetTop(i);
 i += 25;
 Messages          ->SetTop(i);
 i += Messages->GetHeight() + 4;
 About             ->SetTop(i);

 Redraw();
}
//------------------------------------------------------------------------------
