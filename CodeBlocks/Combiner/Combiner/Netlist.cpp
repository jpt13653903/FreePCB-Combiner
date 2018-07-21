//==============================================================================
// Netlist File Abstraction Window
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

#include "Netlist.h"
//------------------------------------------------------------------------------

static int PartsCompare(void* Left, void* Right){
 NETLIST::PART* left  = (NETLIST::PART*)Left;
 NETLIST::PART* right = (NETLIST::PART*)Right;
 return left->Reference.Compare(right->Reference.String);
}
//------------------------------------------------------------------------------

NETLIST::NETLIST(){
 PostMessage   = 0;
 Parts.Compare = PartsCompare;
 Nets          = 0;
}
//------------------------------------------------------------------------------

NETLIST::~NETLIST(){
 Clear();
}
//------------------------------------------------------------------------------

bool NETLIST::GetNewLine(){
 if(Error) return false;

 while(Index < BufferSize){
  if(Buffer[Index] == ' '  ||
     Buffer[Index] == '\r' ||
     Buffer[Index] == '\t' ){
  }else if(Buffer[Index] == '\n'){
   Line++;
   Index++;
   return true;
  }else{
   return false;
  }
  Index++;
 }

 return true;
}
//------------------------------------------------------------------------------

void NETLIST::RemoveSpace(){
 if(Error) return;

 while(Index < BufferSize){
  if(Buffer[Index] == ' '  ||
     Buffer[Index] == '\r' ||
     Buffer[Index] == '\t' ){
  }else if(Buffer[Index] == '\n'){
   Line++;
  }else{
   return;
  }
  Index++;
 }
}
//------------------------------------------------------------------------------

bool NETLIST::GetString(JString* String){
 if(Error) return false;

 String->Set("");

 while(Index < BufferSize){
  if(Buffer[Index] != ' '  &&
     Buffer[Index] != '\r' &&
     Buffer[Index] != '\t' ){
   break;
  }
  Index++;
 }

 if(Index == BufferSize) return false;

 while(Index < BufferSize){
  if(Buffer[Index] == ' '  ||
     Buffer[Index] == '\r' ||
     Buffer[Index] == '\t' ||
     Buffer[Index] == '@'  ||
     Buffer[Index] == '\n' ){
   return true;
  }
  String->Append(Buffer[Index]);
  Index++;
 }
 return true;
}
//------------------------------------------------------------------------------

bool NETLIST::GetSection(JString* Section){
 if(Error) return false;

 Section->Set("");

 RemoveSpace();

 if(Buffer[Index] == '*'){
  Index++;
  while(Index < BufferSize){
   if(Buffer[Index] == '*'){
    Index++;
    if(!GetNewLine()){
     PostMessage(
      mtError,
      File.GetFilename(),
      Line,
      "Newline expected"
     );
     Error = true;
     return false;
    }
    return true;
   }else{
    Section->Append(Buffer[Index]);
   }
   Index++;
  }
  return false;
 }

 return false;
}
//------------------------------------------------------------------------------

void NETLIST::DoParts(){
 PART* Temp;

 while(!Error && Index < BufferSize){
  if(Buffer[Index] == '*') return;

  Temp = new PART;

  if(!GetString(&Temp->Reference)){
   PostMessage(
    mtError,
    File.GetFilename(),
    Line,
    "String (reference) expected."
   );
   Error = true;
   delete Temp;
   return;
  }

  if(!GetString(&Temp->Package)){
   PostMessage(
    mtError,
    File.GetFilename(),
    Line,
    "String (value or package) expected."
   );
   Error = true;
   delete Temp;
   return;
  }

  if(Index < BufferSize && Buffer[Index] == '@'){
   Index++;
   Temp->Value.Set(Temp->Package.String);
   if(!GetString(&Temp->Package)){
    PostMessage(
     mtError,
     File.GetFilename(),
     Line,
     "String (package) expected."
    );
    Error = true;
    delete Temp;
    return;
   }
  }

  if(!GetNewLine()){
   PostMessage(
    mtError,
    File.GetFilename(),
    Line,
    "Newline expected."
   );
   Error = true;
   delete Temp;
   return;
  }

  if(!Parts.Add(Temp)){
   PostMessage(
    mtError,
    File.GetFilename(),
    Line-1,
    "Duplicate Reference."
   );
   Error = true;
   delete Temp;
   return;
  }

  RemoveSpace();
 }
}
//------------------------------------------------------------------------------

void NETLIST::DoNets(){
 bool NewLine;

 if(Nets){
  PostMessage(
   mtError,
   File.GetFilename(),
   Line,
   "Nets section already handled."
  );
  Error = true;
  return;
 }

 RemoveSpace();

 Nets = new NET;
 Nets->Body.Set("");

 NewLine = true;
 while(Index < BufferSize){
  if(Buffer[Index] == ' '  ||
     Buffer[Index] == '\r' ||
     Buffer[Index] == '\t' ){
  }else if(Buffer[Index] == '\n'){
   Line++;
   NewLine = true;
  }else{
   if(NewLine                &&
      Buffer[Index  ] == '*' &&
      Index+1 < BufferSize   &&
      Buffer[Index+1] != 'S' ){
    break;
   }
   NewLine = false;
  }
  Nets->Body.Append(Buffer[Index]);
  Index++;
 }

 // Make sure there is only one linefeed at the end
 while(Nets->Body.String[Nets->Body.GetLength()-1] == '\r' ||
       Nets->Body.String[Nets->Body.GetLength()-1] == '\n' ){
  Nets->Body.Remove(Nets->Body.GetLength()-1);
 }
 Nets->Body.Append("\r\n");
}
//------------------------------------------------------------------------------

void NETLIST::ParseFile(){
 JString Section;

 while(GetSection(&Section)){
  if      (!Section.Compare("PADS-PCB")){
  }else if(!Section.Compare("PART")){
   DoParts();
  }else if(!Section.Compare("NET")){
   DoNets();
  }else if(!Section.Compare("END")){
   return;
  }else if(!Error){
   Section.Prefix("Unknown section: ");
   PostMessage(
    mtError,
    File.GetFilename(),
    Line,
    Section.String
   );
   Error = true;
  }
 }

 if(!Error){
  PostMessage(
   mtError,
   File.GetFilename(),
   Line,
   "Section expected"
  );
  Error = true;
 }
}
//------------------------------------------------------------------------------

void NETLIST::Clear(){
 Parts.Clear();
 if(Nets) delete Nets;
}
//------------------------------------------------------------------------------

bool NETLIST::LoadFile(const char* Filename){
 bool  b;

 Clear();

 File.SetFilename(Filename);
 if(File.Open(JFile::Read)){
  BufferSize = File.GetSize();
  Buffer = new char[BufferSize];
  File.ReadBuffer(Buffer, BufferSize, &b);
  File.Close();
  if(b){
   Error = false;
   Line  = 1;
   Index = 0;
   ParseFile();
  }else{
   PostMessage(
    mtError,
    Filename,
    0,
    "Error reading file"
   );
   Error = true;
  }
  delete[] Buffer;

 }else{
  PostMessage(
   mtError,
   Filename,
   0,
   "File could not be opened"
  );
  return false;
 }

 if(!Error){
  PostMessage(
   mtInfo,
   Filename,
   Line,
   "File read successfully."
  );
 }

 return !Error;
}
//------------------------------------------------------------------------------

void NETLIST::SaveParts(JString* Buffer){
 int     Length;
 PART*   Temp;
 JString s;

 // Get the maximum references length
 Length = 0;
 Temp = (PART*)Parts.First();
 while(Temp){
  if(Length < Temp->Reference.GetLength()){
   Length = Temp->Reference.GetLength();
  }
  Temp = (PART*)Parts.Next();
 }

 Length++;

 // Save the parts
 Temp = (PART*)Parts.First();
 while(Temp){
  s.Set(Temp->Reference.String);
  while(s.GetLength() < Length) s.Append(' ');
  Buffer->Append(s.String);
  if(Temp->Value.GetLength()){
   Buffer->Append(Temp->Value.String);
   Buffer->Append('@');
  }
  Buffer->Append(Temp->Package.String);
  Buffer->Append("\r\n");
  Temp = (PART*)Parts.Next();
 }
}
//------------------------------------------------------------------------------

void NETLIST::SaveNets(JString* Buffer){
 if(Nets) Buffer->Append(Nets->Body.String);
}
//------------------------------------------------------------------------------

bool NETLIST::SaveFile(const char* Filename){
 JString SaveBuffer;
 bool    b;

 File.SetFilename(Filename);
 if(File.Open(JFile::Create)){
  SaveBuffer.Append("*PADS-PCB*\r\n");
  SaveBuffer.Append("*PART*\r\n");
  SaveParts(&SaveBuffer);
  SaveBuffer.Append("\r\n*NET*\r\n");
  SaveNets (&SaveBuffer);
  SaveBuffer.Append("*END*\r\n");

  File.WriteBuffer(SaveBuffer.String, SaveBuffer.GetLength(), &b);
  File.Close();
  if(!b){
   PostMessage(
    mtError,
    Filename,
    0,
    "Error writing file"
   );
   return false;
  }

 }else{
  PostMessage(
   mtError,
   Filename,
   0,
   "File could not be opened"
  );
  return false;
 }

 if(!Error){
  PostMessage(
   mtInfo,
   Filename,
   0,
   "File written successfully."
  );
 }

 return true;
}
//------------------------------------------------------------------------------
