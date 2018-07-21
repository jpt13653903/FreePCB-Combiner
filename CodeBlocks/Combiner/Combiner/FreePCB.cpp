//==============================================================================
// FreePCB File Abstraction
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

#include "FreePCB.h"
//------------------------------------------------------------------------------

FREEPCB::FREEPCB(){
 PostMessage      = 0;
 Options          = 0;
 Footprints       = 0;
 BoardOutline     = 0;
 SolderMaskCutout = 0;
 Parts            = 0;
 Nets             = 0;
 Texts            = 0;
}
//------------------------------------------------------------------------------

FREEPCB::~FREEPCB(){
 Clear();
}
//------------------------------------------------------------------------------

void FREEPCB::Clear(){
 int j;

 if(Options){
  delete Options;
 }
 Options = 0;

 FOOTPRINT* TempFP;
 while(Footprints){
  TempFP     = Footprints;
  Footprints = Footprints->Next;
  delete TempFP;
 }

 BOARD* TempB;
 while(BoardOutline){
  TempB        = BoardOutline;
  BoardOutline = BoardOutline->Next;
  if(TempB->Corner) delete[] TempB->Corner;
  delete TempB;
 }

 SOLDERMASK* TempSM;
 while(SolderMaskCutout){
  TempSM           = SolderMaskCutout;
  SolderMaskCutout = SolderMaskCutout->Next;
  if(TempSM->Corner) delete[] TempSM->Corner;
  delete TempSM;
 }

 PART* TempP;
 while(Parts){
  TempP = Parts;
  Parts = Parts->Next;
  delete TempP;
 }

 NET* TempN;
 while(Nets){
  TempN = Nets;
  Nets  = Nets->Next;
  if(TempN->Pins){
   delete[] TempN->Pins;
  }
  if(TempN->Connections){
   for(j = 0; j < TempN->ConnectionsCount; j++){
    if(TempN->Connections[j].Vertex ) delete[] TempN->Connections[j].Vertex ;
    if(TempN->Connections[j].Segment) delete[] TempN->Connections[j].Segment;
   }
   delete[] TempN->Connections;
  }
  if(TempN->Areas){
   for(j = 0; j < TempN->AreasCount; j++){
    if(TempN->Areas[j].Corners) delete[] TempN->Areas[j].Corners;
   }
   delete[] TempN->Areas;
  }
  delete TempN;
 }

 TEXT* TempT;
 while(Texts){
  TempT = Texts;
  Texts = Texts->Next;
  delete TempT;
 }
}
//------------------------------------------------------------------------------

void FREEPCB::DoOptions(){
 bool NewLine;

 if(Options){
  PostMessage(
   mtError,
   File.GetFilename(),
   Line,
   "Options section already handled."
  );
  Error = true;
  return;
 }

 RemoveSpace();

 Options = new OPTIONS;
 Options->Body.Set("");

 NewLine = true;
 while(Index < BufferSize){
  if(Buffer[Index] == ' '  ||
     Buffer[Index] == '\r' ||
     Buffer[Index] == '\t' ){
  }else if(Buffer[Index] == '\n'){
   Line++;
   NewLine = true;
  }else{
   if(NewLine && Buffer[Index] == '['){
    break;
   }
   NewLine = false;
  }
  Options->Body.Append(Buffer[Index]);
  Index++;
 }

 // Make sure there is only one linefeed at the end
 while(Options->Body.String[Options->Body.GetLength()-1] == '\r' ||
       Options->Body.String[Options->Body.GetLength()-1] == '\n' ){
  Options->Body.Remove(Options->Body.GetLength()-1);
 }
 Options->Body.Append("\r\n");
}
//------------------------------------------------------------------------------

void FREEPCB::GetFootprintBody(JString* Body){
 bool NewLine;

 Body->Set("");

 NewLine = true;
 while(Index < BufferSize){
  if(Buffer[Index] == ' '  ||
     Buffer[Index] == '\r' ||
     Buffer[Index] == '\t' ){
  }else if(Buffer[Index] == '\n'){
   Line++;
   NewLine = true;
  }else{
   if(NewLine){
    if(Buffer[Index] == '['){
     break;
    }else if(Index+4 < BufferSize   &&
             Buffer[Index  ] == 'n' &&
             Buffer[Index+1] == 'a' &&
             Buffer[Index+2] == 'm' &&
             Buffer[Index+3] == 'e' &&
             Buffer[Index+4] == ':' ){
     break;
    }
   }
   NewLine = false;
  }
  Body->Append(Buffer[Index]);
  Index++;
 }

 // Make sure there is only one linefeed at the end
 while(Body->String[Body->GetLength()-1] == '\r' ||
       Body->String[Body->GetLength()-1] == '\n' ){
  Body->Remove(Body->GetLength()-1);
 }
 Body->Append("\r\n");
}
//------------------------------------------------------------------------------

void FREEPCB::DoFootprints(){
 JString Keyword;

 FOOTPRINT* Last;

 Last = Footprints;
 while(Last && Last->Next) Last = Last->Next;

 RemoveSpace();
 while(Index < BufferSize){
  if(Buffer[Index] == '[') return;

  if(GetKeyword(&Keyword)){
   if(!Keyword.Compare("name")){
    if(Last){
     Last->Next = new FOOTPRINT;
     Last = Last->Next;
    }else{
     Last = new FOOTPRINT;
     Footprints = Last;
    }
    Last->Next = 0;
    if(!GetString(&Last->Name)){
     PostMessage(
      mtError,
      File.GetFilename(),
      Line,
      "String expected."
     );
     Error = true;
     return;
    }
    if(!GetNewLine()){
     PostMessage(
      mtError,
      File.GetFilename(),
      Line,
      "Newline expected."
     );
     Error = true;
     return;
    }
    GetFootprintBody(&Last->Body);

   }else{
    PostMessage(
     mtError,
     File.GetFilename(),
     Line,
     "\"name\" expected."
    );
    Error = true;
    return;
   }

  }else{
   PostMessage(
    mtError,
    File.GetFilename(),
    Line,
    "Keyword expected."
   );
   Error = true;
   return;
  }

  RemoveSpace();
 }
}
//------------------------------------------------------------------------------

bool FREEPCB::GetCorner(CORNER* Corner){
 int     i;
 JString Keyword;

 Corner->X     = 0;
 Corner->Y     = 0;
 Corner->Style = 0;

 if(Error) return false;

 if(GetKeyword(&Keyword)){
  if(!Keyword.Compare("corner")){
   if(!GetInteger(&i)){
    PostMessage(
     mtError,
     File.GetFilename(),
     Line,
     "Integer (corner index) expected."
    );
    Error = true;
    return false;
   }
   if(!GetInteger(&Corner->X)){
    PostMessage(
     mtError,
     File.GetFilename(),
     Line,
     "Integer (X) expected."
    );
    Error = true;
    return false;
   }
   if(!GetInteger(&Corner->Y)){
    PostMessage(
     mtError,
     File.GetFilename(),
     Line,
     "Integer (Y) expected."
    );
    Error = true;
    return false;
   }
   if(!GetInteger(&Corner->Style)){
    PostMessage(
     mtError,
     File.GetFilename(),
     Line,
     "Integer (style) expected."
    );
    Error = true;
    return false;
   }
   if(!GetNewLine()){
    PostMessage(
     mtError,
     File.GetFilename(),
     Line,
     "Newline expected."
    );
    Error = true;
    return false;
   }

  }else{
   PostMessage(
    mtError,
    File.GetFilename(),
    Line,
    "\"corner\" expected."
   );
   Error = true;
   return false;
  }

 }else{
  PostMessage(
   mtError,
   File.GetFilename(),
   Line,
   "Keyword expected."
  );
  Error = true;
  return false;
 }

 return true;
}
//------------------------------------------------------------------------------

void FREEPCB::DoBoard(){
 int     i, j;
 JString Keyword;

 BOARD* Last;

 Last = BoardOutline;
 while(Last && Last->Next) Last = Last->Next;

 RemoveSpace();
 while(Index < BufferSize){
  if(Buffer[Index] == '[') return;

  if(GetKeyword(&Keyword)){
   if(!Keyword.Compare("outline")){
    if(Last){
     Last->Next = new BOARD;
     Last = Last->Next;
    }else{
     Last = new BOARD;
     BoardOutline = Last;
    }
    Last->CornerCount = 0;
    Last->Corner      = 0;
    Last->Next        = 0;
    if(!GetInteger(&Last->CornerCount)){
     PostMessage(
      mtError,
      File.GetFilename(),
      Line,
      "Integer (number of corners) expected."
     );
     Error = true;
     return;
    }
    if(!GetInteger(&i)){
     PostMessage(
      mtError,
      File.GetFilename(),
      Line,
      "Integer (outline index) expected."
     );
     Error = true;
     return;
    }
    if(!GetNewLine()){
     PostMessage(
      mtError,
      File.GetFilename(),
      Line,
      "Newline expected."
     );
     Error = true;
     return;
    }
    Last->Corner = new CORNER[Last->CornerCount];
    for(j = 0; j < Last->CornerCount; j++){
     if(!GetCorner(&(Last->Corner[j]))){
      PostMessage(
       mtError,
       File.GetFilename(),
       Line,
       "Corner expected."
      );
      Error = true;
      return;
     }
    }

   }else{
    PostMessage(
     mtError,
     File.GetFilename(),
     Line,
     "\"outline\" expected."
    );
    Error = true;
    return;
   }

  }else{
   PostMessage(
    mtError,
    File.GetFilename(),
    Line,
    "Keyword expected."
   );
   Error = true;
   return;
  }

  RemoveSpace();
 }
}
//------------------------------------------------------------------------------

void FREEPCB::DoSolderMask(){
 int     j;
 JString Keyword;

 SOLDERMASK* Last;

 Last = SolderMaskCutout;
 while(Last && Last->Next) Last = Last->Next;

 RemoveSpace();
 while(Index < BufferSize){
  if(Buffer[Index] == '[') return;

  if(GetKeyword(&Keyword)){
   if(!Keyword.Compare("sm_cutout")){
    if(Last){
     Last->Next = new SOLDERMASK;
     Last = Last->Next;
    }else{
     Last = new SOLDERMASK;
     SolderMaskCutout = Last;
    }
    Last->CornerCount = 0;
    Last->FillStyle   = 0;
    Last->Layer       = 0;
    Last->Corner      = 0;
    Last->Next        = 0;
    if(!GetInteger(&Last->CornerCount)){
     PostMessage(
      mtError,
      File.GetFilename(),
      Line,
      "Integer (number of corners) expected."
     );
     Error = true;
     return;
    }
    if(!GetInteger(&Last->FillStyle)){
     PostMessage(
      mtError,
      File.GetFilename(),
      Line,
      "Integer (fill style) expected."
     );
     Error = true;
     return;
    }
    if(!GetInteger(&Last->Layer)){
     PostMessage(
      mtError,
      File.GetFilename(),
      Line,
      "Integer (layer) expected."
     );
     Error = true;
     return;
    }
    if(!GetNewLine()){
     PostMessage(
      mtError,
      File.GetFilename(),
      Line,
      "Newline expected."
     );
     Error = true;
     return;
    }
    Last->Corner = new CORNER[Last->CornerCount];
    for(j = 0; j < Last->CornerCount; j++){
     if(!GetCorner(&(Last->Corner[j]))){
      PostMessage(
       mtError,
       File.GetFilename(),
       Line,
       "Corner expected."
      );
      Error = true;
      return;
     }
    }

   }else{
    PostMessage(
     mtError,
     File.GetFilename(),
     Line,
     "\"sm_cutout\" expected."
    );
    Error = true;
    return;
   }

  }else{
   PostMessage(
    mtError,
    File.GetFilename(),
    Line,
    "Keyword expected."
   );
   Error = true;
   return;
  }

  RemoveSpace();
 }
}
//------------------------------------------------------------------------------

void FREEPCB::GetRefText(REF_TEXT* RefText){
 int i;

 if(!GetInteger(&RefText->Height)){
  PostMessage(
   mtError,
   File.GetFilename(),
   Line,
   "Integer (height) expected."
  );
  Error = true;
  return;
 }

 if(!GetInteger(&RefText->LineWidth)){
  PostMessage(
   mtError,
   File.GetFilename(),
   Line,
   "Integer (line width) expected."
  );
  Error = true;
  return;
 }

 if(!GetInteger(&RefText->Angle)){
  PostMessage(
   mtError,
   File.GetFilename(),
   Line,
   "Integer (angle) expected."
  );
  Error = true;
  return;
 }

 if(!GetInteger(&RefText->X)){
  PostMessage(
   mtError,
   File.GetFilename(),
   Line,
   "Integer (X) expected."
  );
  Error = true;
  return;
 }

 if(!GetInteger(&RefText->Y)){
  PostMessage(
   mtError,
   File.GetFilename(),
   Line,
   "Integer (Y) expected."
  );
  Error = true;
  return;
 }

 if(!GetInteger(&i)){
  PostMessage(
   mtError,
   File.GetFilename(),
   Line,
   "Integer (visibility) expected."
  );
  Error = true;
  return;
 }
 RefText->Visibility = i;

 if(!GetNewLine()){
  PostMessage(
   mtError,
   File.GetFilename(),
   Line,
   "Newline expected."
  );
  Error = true;
  return;
 }
}
//------------------------------------------------------------------------------

void FREEPCB::GetPackage(JString* Package){
 if(!GetString(Package)){
  PostMessage(
   mtError,
   File.GetFilename(),
   Line,
   "String expected."
  );
  Error = true;
  return;
 }

 if(!GetNewLine()){
  PostMessage(
   mtError,
   File.GetFilename(),
   Line,
   "Newline expected."
  );
  Error = true;
  return;
 }
}
//------------------------------------------------------------------------------

void FREEPCB::GetShape(JString* Shape){
 if(!GetString(Shape)){
  PostMessage(
   mtError,
   File.GetFilename(),
   Line,
   "String expected."
  );
  Error = true;
  return;
 }

 if(!GetNewLine()){
  PostMessage(
   mtError,
   File.GetFilename(),
   Line,
   "Newline expected."
  );
  Error = true;
  return;
 }
}
//------------------------------------------------------------------------------

void FREEPCB::GetPos(POS* Pos){
 int i;

 if(!GetInteger(&Pos->X)){
  PostMessage(
   mtError,
   File.GetFilename(),
   Line,
   "Integer (X) expected."
  );
  Error = true;
  return;
 }

 if(!GetInteger(&Pos->Y)){
  PostMessage(
   mtError,
   File.GetFilename(),
   Line,
   "Integer (Y) expected."
  );
  Error = true;
  return;
 }

 if(!GetInteger(&Pos->Side)){
  PostMessage(
   mtError,
   File.GetFilename(),
   Line,
   "Integer (side) expected."
  );
  Error = true;
  return;
 }

 if(!GetInteger(&Pos->Angle)){
  PostMessage(
   mtError,
   File.GetFilename(),
   Line,
   "Integer (angle) expected."
  );
  Error = true;
  return;
 }

 if(!GetInteger(&i)){
  PostMessage(
   mtError,
   File.GetFilename(),
   Line,
   "Integer (glued) expected."
  );
  Error = true;
  return;
 }
 Pos->Glued = i;

 if(!GetNewLine()){
  PostMessage(
   mtError,
   File.GetFilename(),
   Line,
   "Newline expected."
  );
  Error = true;
  return;
 }
}
//------------------------------------------------------------------------------

void FREEPCB::GetValue(VALUE* Value){
 int i;

 if(!GetString(&Value->Value)){
  PostMessage(
   mtError,
   File.GetFilename(),
   Line,
   "String expected."
  );
  Error = true;
  return;
 }

 if(!GetInteger(&Value->Height)){
  PostMessage(
   mtError,
   File.GetFilename(),
   Line,
   "Integer (height) expected."
  );
  Error = true;
  return;
 }

 if(!GetInteger(&Value->LineWidth)){
  PostMessage(
   mtError,
   File.GetFilename(),
   Line,
   "Integer (line width) expected."
  );
  Error = true;
  return;
 }

 if(!GetInteger(&Value->Angle)){
  PostMessage(
   mtError,
   File.GetFilename(),
   Line,
   "Integer (angle) expected."
  );
  Error = true;
  return;
 }

 if(!GetInteger(&Value->X)){
  PostMessage(
   mtError,
   File.GetFilename(),
   Line,
   "Integer (X) expected."
  );
  Error = true;
  return;
 }

 if(!GetInteger(&Value->Y)){
  PostMessage(
   mtError,
   File.GetFilename(),
   Line,
   "Integer (Y) expected."
  );
  Error = true;
  return;
 }

 if(!GetInteger(&i)){
  PostMessage(
   mtError,
   File.GetFilename(),
   Line,
   "Integer (visibility) expected."
  );
  Error = true;
  return;
 }
 Value->Visibility = i;

 if(!GetNewLine()){
  PostMessage(
   mtError,
   File.GetFilename(),
   Line,
   "Newline expected."
  );
  Error = true;
  return;
 }
}
//------------------------------------------------------------------------------

void FREEPCB::DoParts(){
 int     index;
 int     line;
 JString Keyword;

 PART* Last;

 Last = Parts;
 while(Last && Last->Next) Last = Last->Next;

 RemoveSpace();
 while(!Error && Index < BufferSize){
  if(Buffer[Index] == '[') return;

  if(GetKeyword(&Keyword)){
   if(!Keyword.Compare("part")){
    if(Last){
     Last->Next = new PART;
     Last = Last->Next;
    }else{
     Last = new PART;
     Parts = Last;
    }
    Last->RefText.Height     = 0;
    Last->RefText.LineWidth  = 0;
    Last->RefText.Angle      = 0;
    Last->RefText.X          = 0;
    Last->RefText.Y          = 0;
    Last->RefText.Visibility = 0;
    Last->Pos    .X          = 0;
    Last->Pos    .Y          = 0;
    Last->Pos    .Side       = 0;
    Last->Pos    .Angle      = 0;
    Last->Pos    .Glued      = 0;
    Last->Value  .Height     = 0;
    Last->Value  .LineWidth  = 0;
    Last->Value  .Angle      = 0;
    Last->Value  .X          = 0;
    Last->Value  .Y          = 0;
    Last->Value  .Visibility = 0;
    Last->Next               = 0;
    if(!GetString(&Last->Reference)){
     PostMessage(
      mtError,
      File.GetFilename(),
      Line,
      "String (reference) expected."
     );
     Error = true;
     return;
    }
    if(!GetNewLine()){
     PostMessage(
      mtError,
      File.GetFilename(),
      Line,
      "Newline expected."
     );
     Error = true;
     return;
    }
    while(!Error && Index < BufferSize){
     index = Index;
     line  = Line;
     if(Buffer[Index] == '[') return;
     if(GetKeyword(&Keyword)){
      if      (!Keyword.Compare("ref_text")){
       GetRefText(&Last->RefText);
      }else if(!Keyword.Compare("package")){
       GetPackage(&Last->Package);
      }else if(!Keyword.Compare("shape")){
       GetShape  (&Last->Shape);
      }else if(!Keyword.Compare("pos")){
       GetPos    (&Last->Pos);
      }else if(!Keyword.Compare("value")){
       GetValue  (&Last->Value);
      }else if(!Keyword.Compare("part")){
       Index = index;
       Line  = line;
       break;
      }else{
       Keyword.Prefix("Unknown part attribute: ");
       PostMessage(
        mtError,
        File.GetFilename(),
        Line,
        Keyword.String
       );
       Error = true;
       return;
      }
     }else{
      PostMessage(
       mtError,
       File.GetFilename(),
       Line,
       "Keyword expected."
      );
      Error = true;
      return;
     }
     RemoveSpace();
    }

   }else{
    PostMessage(
     mtError,
     File.GetFilename(),
     Line,
     "\"part\" expected."
    );
    Error = true;
    return;
   }

  }else{
   PostMessage(
    mtError,
    File.GetFilename(),
    Line,
    "Keyword expected."
   );
   Error = true;
   return;
  }

  RemoveSpace();
 }
}
//------------------------------------------------------------------------------

bool FREEPCB::GetPin(PIN* Pin){
 JString Keyword;

 Pin->Index = 0;

 if(Error) return false;

 if(GetKeyword(&Keyword)){
  if(!Keyword.Compare("pin")){
   if(!GetInteger(&Pin->Index)){
    PostMessage(
     mtError,
     File.GetFilename(),
     Line,
     "Integer (pin index) expected."
    );
    Error = true;
    return false;
   }
   if(!GetString(&Pin->Name)){
    PostMessage(
     mtError,
     File.GetFilename(),
     Line,
     "String (pin name) expected."
    );
    Error = true;
    return false;
   }
   if(!GetNewLine()){
    PostMessage(
     mtError,
     File.GetFilename(),
     Line,
     "Newline expected."
    );
    Error = true;
    return false;
   }

  }else{
   PostMessage(
    mtError,
    File.GetFilename(),
    Line,
    "\"pin\" expected."
   );
   Error = true;
   return false;
  }

 }else{
  PostMessage(
   mtError,
   File.GetFilename(),
   Line,
   "Keyword expected."
  );
  Error = true;
  return false;
 }

 return true;
}
//------------------------------------------------------------------------------

bool FREEPCB::GetVertex(VERTEX* Vertex){
 int     i;
 JString Keyword;

 Vertex->X               = 0;
 Vertex->Y               = 0;
 Vertex->PadLayer        = 0;
 Vertex->ForceVia        = 0;
 Vertex->ViaPadWidth     = 0;
 Vertex->ViaHoleDiameter = 0;
 Vertex->TeeID           = 0;

 if(Error) return false;

 if(GetKeyword(&Keyword)){
  if(!Keyword.Compare("vtx")){
   if(!GetInteger(&i)){
    PostMessage(
     mtError,
     File.GetFilename(),
     Line,
     "Integer (vertex index) expected."
    );
    Error = true;
    return false;
   }
   if(!GetInteger(&Vertex->X)){
    PostMessage(
     mtError,
     File.GetFilename(),
     Line,
     "Integer (X) expected."
    );
    Error = true;
    return false;
   }
   if(!GetInteger(&Vertex->Y)){
    PostMessage(
     mtError,
     File.GetFilename(),
     Line,
     "Integer (Y) expected."
    );
    Error = true;
    return false;
   }
   if(!GetInteger(&Vertex->PadLayer)){
    PostMessage(
     mtError,
     File.GetFilename(),
     Line,
     "Integer (pad layer) expected."
    );
    Error = true;
    return false;
   }
   if(!GetInteger(&i)){
    PostMessage(
     mtError,
     File.GetFilename(),
     Line,
     "Integer (force via) expected."
    );
    Error = true;
    return false;
   }
   Vertex->ForceVia = i;
   if(!GetInteger(&Vertex->ViaPadWidth)){
    PostMessage(
     mtError,
     File.GetFilename(),
     Line,
     "Integer (via pad width) expected."
    );
    Error = true;
    return false;
   }
   if(!GetInteger(&Vertex->ViaHoleDiameter)){
    PostMessage(
     mtError,
     File.GetFilename(),
     Line,
     "Integer (via hole diameter) expected."
    );
    Error = true;
    return false;
   }
   if(!GetInteger(&Vertex->TeeID)){
    PostMessage(
     mtError,
     File.GetFilename(),
     Line,
     "Integer (tee id) expected."
    );
    Error = true;
    return false;
   }
   if(!GetNewLine()){
    PostMessage(
     mtError,
     File.GetFilename(),
     Line,
     "Newline expected."
    );
    Error = true;
    return false;
   }

  }else{
   PostMessage(
    mtError,
    File.GetFilename(),
    Line,
    "\"vtx\" expected."
   );
   Error = true;
   return false;
  }

 }else{
  PostMessage(
   mtError,
   File.GetFilename(),
   Line,
   "Keyword expected."
  );
  Error = true;
  return false;
 }

 return true;
}
//------------------------------------------------------------------------------

bool FREEPCB::GetSegment(SEGMENT* Segment){
 int     i;
 JString Keyword;

 Segment->Layer           = 0;
 Segment->Width           = 0;
 Segment->ViaPadWidth     = 0;
 Segment->ViaHoleDiameter = 0;

 if(Error) return false;

 if(GetKeyword(&Keyword)){
  if(!Keyword.Compare("seg")){
   if(!GetInteger(&i)){
    PostMessage(
     mtError,
     File.GetFilename(),
     Line,
     "Integer (vertex index) expected."
    );
    Error = true;
    return false;
   }
   if(!GetInteger(&Segment->Layer)){
    PostMessage(
     mtError,
     File.GetFilename(),
     Line,
     "Integer (layer) expected."
    );
    Error = true;
    return false;
   }
   if(!GetInteger(&Segment->Width)){
    PostMessage(
     mtError,
     File.GetFilename(),
     Line,
     "Integer (width) expected."
    );
    Error = true;
    return false;
   }
   if(!GetInteger(&Segment->ViaPadWidth)){
    PostMessage(
     mtError,
     File.GetFilename(),
     Line,
     "Integer (via pad width) expected."
    );
    Error = true;
    return false;
   }
   if(!GetInteger(&Segment->ViaHoleDiameter)){
    PostMessage(
     mtError,
     File.GetFilename(),
     Line,
     "Integer (via hole diameter) expected."
    );
    Error = true;
    return false;
   }
   if(!GetNewLine()){
    PostMessage(
     mtError,
     File.GetFilename(),
     Line,
     "Newline expected."
    );
    Error = true;
    return false;
   }

  }else{
   PostMessage(
    mtError,
    File.GetFilename(),
    Line,
    "\"seg\" expected."
   );
   Error = true;
   return false;
  }

 }else{
  PostMessage(
   mtError,
   File.GetFilename(),
   Line,
   "Keyword expected."
  );
  Error = true;
  return false;
 }

 return true;
}
//------------------------------------------------------------------------------

bool FREEPCB::GetConnect(CONNECT* Connect){
 int     i, j;
 JString Keyword;

 Connect->StartPinIndex = 0;
 Connect->EndPinIndex   = 0;
 Connect->SegmentCount  = 0;
 Connect->Locked        = 0;
 Connect->Vertex        = 0;
 Connect->Segment       = 0;

 if(Error) return false;

 if(GetKeyword(&Keyword)){
  if(!Keyword.Compare("connect")){
   if(!GetInteger(&i)){
    PostMessage(
     mtError,
     File.GetFilename(),
     Line,
     "Integer (connection index) expected."
    );
    Error = true;
    return false;
   }
   if(!GetInteger(&Connect->StartPinIndex)){
    PostMessage(
     mtError,
     File.GetFilename(),
     Line,
     "Integer (start pin index) expected."
    );
    Error = true;
    return false;
   }
   if(!GetInteger(&Connect->EndPinIndex)){
    PostMessage(
     mtError,
     File.GetFilename(),
     Line,
     "Integer (end pin index) expected."
    );
    Error = true;
    return false;
   }
   if(!GetInteger(&Connect->SegmentCount)){
    PostMessage(
     mtError,
     File.GetFilename(),
     Line,
     "Integer (number of segments) expected."
    );
    Error = true;
    return false;
   }
   if(!GetInteger(&i)){
    PostMessage(
     mtError,
     File.GetFilename(),
     Line,
     "Integer (number of segments) expected."
    );
    Error = true;
    return false;
   }
   Connect->Locked = i;
   if(!GetNewLine()){
    PostMessage(
     mtError,
     File.GetFilename(),
     Line,
     "Newline expected."
    );
    Error = true;
    return false;
   }
   Connect->Vertex  = new VERTEX [Connect->SegmentCount+1];
   Connect->Segment = new SEGMENT[Connect->SegmentCount  ];
   for(j = 0; j < Connect->SegmentCount; j++){
    if(!GetVertex(&Connect->Vertex[j])){
     PostMessage(
      mtError,
      File.GetFilename(),
      Line,
      "Vertex expected."
     );
     Error = true;
     return false;
    }
    if(!GetSegment(&Connect->Segment[j])){
     PostMessage(
      mtError,
      File.GetFilename(),
      Line,
      "Segment expected."
     );
     Error = true;
     return false;
    }
   }
   if(!GetVertex(&Connect->Vertex[j])){
    PostMessage(
     mtError,
     File.GetFilename(),
     Line,
     "Vertex expected."
    );
    Error = true;
    return false;
   }

  }else{
   PostMessage(
    mtError,
    File.GetFilename(),
    Line,
    "\"connect\" expected."
   );
   Error = true;
   return false;
  }

 }else{
  PostMessage(
   mtError,
   File.GetFilename(),
   Line,
   "Keyword expected."
  );
  Error = true;
  return false;
 }

 return true;
}
//------------------------------------------------------------------------------

bool FREEPCB::GetAreaCorner(AREA_CORNER* Corner){
 int     i;
 JString Keyword;

 Corner->X          = 0;
 Corner->Y          = 0;
 Corner->Style      = 0;
 Corner->LastCorner = 0;

 if(Error) return false;

 if(GetKeyword(&Keyword)){
  if(!Keyword.Compare("corner")){
   if(!GetInteger(&i)){
    PostMessage(
     mtError,
     File.GetFilename(),
     Line,
     "Integer (corner index) expected."
    );
    Error = true;
    return false;
   }
   if(!GetInteger(&Corner->X)){
    PostMessage(
     mtError,
     File.GetFilename(),
     Line,
     "Integer (X) expected."
    );
    Error = true;
    return false;
   }
   if(!GetInteger(&Corner->Y)){
    PostMessage(
     mtError,
     File.GetFilename(),
     Line,
     "Integer (Y) expected."
    );
    Error = true;
    return false;
   }
   if(!GetInteger(&Corner->Style)){
    PostMessage(
     mtError,
     File.GetFilename(),
     Line,
     "Integer (style) expected."
    );
    Error = true;
    return false;
   }
   if(!GetInteger(&i)){
    PostMessage(
     mtError,
     File.GetFilename(),
     Line,
     "Integer (last corner flag) expected."
    );
    Error = true;
    return false;
   }
   Corner->LastCorner = i;
   if(!GetNewLine()){
    PostMessage(
     mtError,
     File.GetFilename(),
     Line,
     "Newline expected."
    );
    Error = true;
    return false;
   }

  }else{
   PostMessage(
    mtError,
    File.GetFilename(),
    Line,
    "\"corner\" expected."
   );
   Error = true;
   return false;
  }

 }else{
  PostMessage(
   mtError,
   File.GetFilename(),
   Line,
   "Keyword expected."
  );
  Error = true;
  return false;
 }

 return true;
}
//------------------------------------------------------------------------------

bool FREEPCB::GetArea(AREA* Area){
 int     i, j;
 JString Keyword;

 Area->CornerCount = 0;
 Area->Layer       = 0;
 Area->HatchStyle  = 0;
 Area->Corners     = 0;

 if(Error) return false;

 if(GetKeyword(&Keyword)){
  if(!Keyword.Compare("area")){
   if(!GetInteger(&i)){
    PostMessage(
     mtError,
     File.GetFilename(),
     Line,
     "Integer (area index) expected."
    );
    Error = true;
    return false;
   }
   if(!GetInteger(&Area->CornerCount)){
    PostMessage(
     mtError,
     File.GetFilename(),
     Line,
     "Integer (number of corners) expected."
    );
    Error = true;
    return false;
   }
   if(!GetInteger(&Area->Layer)){
    PostMessage(
     mtError,
     File.GetFilename(),
     Line,
     "Integer (layer) expected."
    );
    Error = true;
    return false;
   }
   if(!GetInteger(&Area->HatchStyle)){
    PostMessage(
     mtError,
     File.GetFilename(),
     Line,
     "Integer (hatch style) expected."
    );
    Error = true;
    return false;
   }
   if(!GetNewLine()){
    PostMessage(
     mtError,
     File.GetFilename(),
     Line,
     "Newline expected."
    );
    Error = true;
    return false;
   }
   Area->Corners  = new AREA_CORNER[Area->CornerCount];
   for(j = 0; j < Area->CornerCount; j++){
    if(!GetAreaCorner(&Area->Corners[j])){
     PostMessage(
      mtError,
      File.GetFilename(),
      Line,
      "Corner expected."
     );
     Error = true;
     return false;
    }
   }

  }else{
   PostMessage(
    mtError,
    File.GetFilename(),
    Line,
    "\"area\" expected."
   );
   Error = true;
   return false;
  }

 }else{
  PostMessage(
   mtError,
   File.GetFilename(),
   Line,
   "Keyword expected."
  );
  Error = true;
  return false;
 }

 return true;
}
//------------------------------------------------------------------------------

void FREEPCB::DoNets(){
 int     i, j;
 JString Keyword;

 NET* Last;

 Last = Nets;
 while(Last && Last->Next) Last = Last->Next;

 RemoveSpace();
 while(!Error && Index < BufferSize){
  if(Buffer[Index] == '[') return;

  if(GetKeyword(&Keyword)){
   if(!Keyword.Compare("net")){
    if(Last){
     Last->Next = new NET;
     Last = Last->Next;
    }else{
     Last = new NET;
     Nets = Last;
    }
    Last->PinCount               = 0;
    Last->ConnectionsCount       = 0;
    Last->AreasCount             = 0;
    Last->DefaultTraceWidth      = 0;
    Last->DefaultViaPadWidth     = 0;
    Last->DefaultViaHoleDiameter = 0;
    Last->Visibility             = 0;
    Last->Pins                   = 0;
    Last->Connections            = 0;
    Last->Areas                  = 0;
    Last->Next                   = 0;
    if(!GetString(&Last->Name)){
     PostMessage(
      mtError,
      File.GetFilename(),
      Line,
      "String (net name) expected."
     );
     Error = true;
     return;
    }
    if(!GetInteger(&Last->PinCount)){
     PostMessage(
      mtError,
      File.GetFilename(),
      Line,
      "Integer (number of pins) expected."
     );
     Error = true;
     return;
    }
    if(!GetInteger(&Last->ConnectionsCount)){
     PostMessage(
      mtError,
      File.GetFilename(),
      Line,
      "Integer (number of connections) expected."
     );
     Error = true;
     return;
    }
    if(!GetInteger(&Last->AreasCount)){
     PostMessage(
      mtError,
      File.GetFilename(),
      Line,
      "Integer (number of areas) expected."
     );
     Error = true;
     return;
    }
    if(!GetInteger(&Last->DefaultTraceWidth)){
     PostMessage(
      mtError,
      File.GetFilename(),
      Line,
      "Integer (default trace width) expected."
     );
     Error = true;
     return;
    }
    if(!GetInteger(&Last->DefaultViaPadWidth)){
     PostMessage(
      mtError,
      File.GetFilename(),
      Line,
      "Integer (default via pad width) expected."
     );
     Error = true;
     return;
    }
    if(!GetInteger(&Last->DefaultViaHoleDiameter)){
     PostMessage(
      mtError,
      File.GetFilename(),
      Line,
      "Integer (default via hole diameter) expected."
     );
     Error = true;
     return;
    }
    if(!GetInteger(&i)){
     PostMessage(
      mtError,
      File.GetFilename(),
      Line,
      "Integer (visibility) expected."
     );
     Error = true;
     return;
    }
    Last->Visibility = i;
    if(!GetNewLine()){
     PostMessage(
      mtError,
      File.GetFilename(),
      Line,
      "Newline expected."
     );
     Error = true;
     return;
    }
    Last->Pins = new PIN[Last->PinCount];
    for(j = 0; j < Last->PinCount; j++){
     if(!GetPin(&Last->Pins[j])){
      PostMessage(
       mtError,
       File.GetFilename(),
       Line,
       "Pin expected."
      );
      Error = true;
      return;
     }
    }
    Last->Connections = new CONNECT[Last->ConnectionsCount];
    for(j = 0; j < Last->ConnectionsCount; j++){
     if(!GetConnect(&Last->Connections[j])){
      PostMessage(
       mtError,
       File.GetFilename(),
       Line,
       "Connection expected."
      );
      Error = true;
      return;
     }
    }
    Last->Areas = new AREA[Last->AreasCount];
    for(j = 0; j < Last->AreasCount; j++){
     if(!GetArea(&Last->Areas[j])){
      PostMessage(
       mtError,
       File.GetFilename(),
       Line,
       "Area expected."
      );
      Error = true;
      return;
     }
    }

   }else{
    PostMessage(
     mtError,
     File.GetFilename(),
     Line,
     "\"net\" expected."
    );
    Error = true;
    return;
   }

  }else{
   PostMessage(
    mtError,
    File.GetFilename(),
    Line,
    "Keyword expected."
   );
   Error = true;
   return;
  }

  RemoveSpace();
 }
}
//------------------------------------------------------------------------------

void FREEPCB::DoTexts(){
 int     i;
 JString Keyword;

 TEXT* Last;

 Last = Texts;
 while(Last && Last->Next) Last = Last->Next;

 RemoveSpace();
 while(!Error && Index < BufferSize){
  if(Buffer[Index] == '[') return;

  if(GetKeyword(&Keyword)){
   if(!Keyword.Compare("text")){
    if(Last){
     Last->Next = new TEXT;
     Last = Last->Next;
    }else{
     Last = new TEXT;
     Texts = Last;
    }
    Last->X         = 0;
    Last->Y         = 0;
    Last->Layer     = 0;
    Last->Angle     = 0;
    Last->Mirror    = 0;
    Last->Height    = 0;
    Last->LineWidth = 0;
    Last->Negative  = 0;
    Last->Next      = 0;
    if(!GetString(&Last->Text)){
     PostMessage(
      mtError,
      File.GetFilename(),
      Line,
      "String (text) expected."
     );
     Error = true;
     return;
    }
    if(!GetInteger(&Last->X)){
     PostMessage(
      mtError,
      File.GetFilename(),
      Line,
      "String (X) expected."
     );
     Error = true;
     return;
    }
    if(!GetInteger(&Last->Y)){
     PostMessage(
      mtError,
      File.GetFilename(),
      Line,
      "String (Y) expected."
     );
     Error = true;
     return;
    }
    if(!GetInteger(&Last->Layer)){
     PostMessage(
      mtError,
      File.GetFilename(),
      Line,
      "String (layer) expected."
     );
     Error = true;
     return;
    }
    if(!GetInteger(&Last->Angle)){
     PostMessage(
      mtError,
      File.GetFilename(),
      Line,
      "String (angle) expected."
     );
     Error = true;
     return;
    }
    if(!GetInteger(&i)){
     PostMessage(
      mtError,
      File.GetFilename(),
      Line,
      "String (mirror flag) expected."
     );
     Error = true;
     return;
    }
    Last->Mirror = i;
    if(!GetInteger(&Last->Height)){
     PostMessage(
      mtError,
      File.GetFilename(),
      Line,
      "String (height) expected."
     );
     Error = true;
     return;
    }
    if(!GetInteger(&Last->LineWidth)){
     PostMessage(
      mtError,
      File.GetFilename(),
      Line,
      "String (line width) expected."
     );
     Error = true;
     return;
    }
    if(!GetInteger(&i)){
     PostMessage(
      mtError,
      File.GetFilename(),
      Line,
      "String (negative flag) expected."
     );
     Error = true;
     return;
    }
    Last->Negative = i;
    if(!GetNewLine()){
     PostMessage(
      mtError,
      File.GetFilename(),
      Line,
      "Newline expected."
     );
     Error = true;
     return;
    }

   }else{
    PostMessage(
     mtError,
     File.GetFilename(),
     Line,
     "\"text\" expected."
    );
    Error = true;
    return;
   }

  }else{
   PostMessage(
    mtError,
    File.GetFilename(),
    Line,
    "Keyword expected."
   );
   Error = true;
   return;
  }

  RemoveSpace();
 }
}
//------------------------------------------------------------------------------

bool FREEPCB::GetNewLine(){
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

void FREEPCB::RemoveSpace(){
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

bool FREEPCB::GetInteger(int* Integer){
 bool Negative = false;

 if(Error) return false;

 *Integer = 0;

 while(Index < BufferSize){
  if(Buffer[Index] != ' '  &&
     Buffer[Index] != '\r' &&
     Buffer[Index] != '\t' ){
   break;
  }
  Index++;
 }

 if(Index == BufferSize) return false;

 if(Buffer[Index] == '-'){
  Negative = true;
  Index++;
 }

 while(Index < BufferSize){
  if(Buffer[Index] == ' '  ||
     Buffer[Index] == '\r' ||
     Buffer[Index] == '\t' ||
     Buffer[Index] == '\n' ){
   break;
  }else if(Buffer[Index] >= '0' && Buffer[Index] <= '9'){
   *Integer = 10*(*Integer) + (Buffer[Index] - '0');
  }else{
   return false;
  }
  Index++;
 }

 if(Negative) *Integer *= -1;
 return true;
}
//------------------------------------------------------------------------------

bool FREEPCB::GetString(JString* String){
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

 if(Buffer[Index] == '"'){
  Index++;
  while(Index < BufferSize){
   if(Buffer[Index] == '\n'){
    return false;
   }else if(Buffer[Index] == '"'){
    Index++;
    return true;
   }
   String->Append(Buffer[Index]);
   Index++;
  }

 }else{
  while(Index < BufferSize){
   if(Buffer[Index] == ' '  ||
      Buffer[Index] == '\r' ||
      Buffer[Index] == '\t' ||
      Buffer[Index] == '\n' ){
    return true;
   }
   String->Append(Buffer[Index]);
   Index++;
  }
  return true;
 }

 return false;
}
//------------------------------------------------------------------------------

bool FREEPCB::GetKeyword(JString* Keyword){
 if(Error) return false;

 Keyword->Set("");

 RemoveSpace();

 while(Index < BufferSize){
  if(Buffer[Index] == ' '  ||
     Buffer[Index] == '\r' ||
     Buffer[Index] == '\t' ){
   return false;
  }else if(Buffer[Index] == ':'){
   Index++;
   return true;
  }else{
   Keyword->Append(Buffer[Index]);
  }
  Index++;
 }

 return false;
}
//------------------------------------------------------------------------------

bool FREEPCB::GetSection(JString* Section){
 if(Error) return false;

 Section->Set("");

 RemoveSpace();

 if(Buffer[Index] == '['){
  Index++;
  while(Index < BufferSize){
   if(Buffer[Index] == ']'){
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

void FREEPCB::ParseFile(){
 JString Section;

 while(GetSection(&Section)){
  if      (!Section.Compare("options")){
   DoOptions();
  }else if(!Section.Compare("footprints")){
   DoFootprints();
  }else if(!Section.Compare("board")){
   DoBoard();
  }else if(!Section.Compare("solder_mask_cutouts")){
   DoSolderMask();
  }else if(!Section.Compare("parts")){
   DoParts();
  }else if(!Section.Compare("nets")){
   DoNets();
  }else if(!Section.Compare("texts")){
   DoTexts();
  }else if(!Section.Compare("end")){
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

bool FREEPCB::LoadFile(const char* Filename){
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

void FREEPCB::SaveOptions(JString* Buffer){
 if(Options){
  Buffer->Append("\r\n");
  Buffer->Append(Options->Body.String);
 }
}
//------------------------------------------------------------------------------

void FREEPCB::SaveFootprints(JString* Buffer, const char* Prefix){
 FOOTPRINT* Temp;

 Temp = Footprints;
 while(Temp){
  Buffer->Append("\r\nname: \"");
  Buffer->Append(Prefix);
  Buffer->Append(Temp->Name.String);
  Buffer->Append("\"\r\n");
  Buffer->Append(Temp->Body.String);
  Temp = Temp->Next;
 }
}
//------------------------------------------------------------------------------

void FREEPCB::SaveBoard(
 JString* Buffer,
 int      StartIndex,
 int      XOffset,
 int      YOffset
){
 int    j;
 BOARD* Temp;

 Temp = BoardOutline;
 while(Temp){
  Buffer->Append("\r\noutline: ");
  Buffer->Append(Temp->CornerCount);
  Buffer->Append(' ');
  Buffer->Append(StartIndex);
  Buffer->Append("\r\n");

  for(j = 0; j < Temp->CornerCount; j++){
   Buffer->Append("  corner: ");
   Buffer->Append(j+1);
   Buffer->Append(' ');
   Buffer->Append(Temp->Corner[j].X + XOffset);
   Buffer->Append(' ');
   Buffer->Append(Temp->Corner[j].Y + YOffset);
   Buffer->Append(' ');
   Buffer->Append(Temp->Corner[j].Style);
   Buffer->Append("\r\n");
  }

  StartIndex++;
  Temp = Temp->Next;
 }
}
//------------------------------------------------------------------------------

void FREEPCB::SaveSolderMask(JString* Buffer, int XOffset, int YOffset){
 int    j;
 SOLDERMASK* Temp;

 Temp = SolderMaskCutout;
 while(Temp){
  Buffer->Append("\r\nsm_cutout: ");
  Buffer->Append(Temp->CornerCount);
  Buffer->Append(' ');
  Buffer->Append(Temp->FillStyle);
  Buffer->Append(' ');
  Buffer->Append(Temp->Layer);
  Buffer->Append("\r\n");

  for(j = 0; j < Temp->CornerCount; j++){
   Buffer->Append("  corner: ");
   Buffer->Append(j+1);
   Buffer->Append(' ');
   Buffer->Append(Temp->Corner[j].X + XOffset);
   Buffer->Append(' ');
   Buffer->Append(Temp->Corner[j].Y + YOffset);
   Buffer->Append(' ');
   Buffer->Append(Temp->Corner[j].Style);
   Buffer->Append("\r\n");
  }

  Temp = Temp->Next;
 }
}
//------------------------------------------------------------------------------

void FREEPCB::SaveParts(
 JString   * Buffer,
 const char* FootprintPrefix,
 const char* PartPrefix,
 int         XOffset,
 int         YOffset
){
 PART* Temp;

 Temp = Parts;
 while(Temp){
  Buffer->Append("\r\npart: ");
  Buffer->Append(PartPrefix);
  Buffer->Append(Temp->Reference.String);
  Buffer->Append("\r\n");

  Buffer->Append("  ref_text: ");
  Buffer->Append(Temp->RefText.Height);
  Buffer->Append(' ');
  Buffer->Append(Temp->RefText.LineWidth);
  Buffer->Append(' ');
  Buffer->Append(Temp->RefText.Angle);
  Buffer->Append(' ');
  Buffer->Append(Temp->RefText.X);
  Buffer->Append(' ');
  Buffer->Append(Temp->RefText.Y);
  Buffer->Append(' ');
  Buffer->Append(Temp->RefText.Visibility ? 1 : 0);
  Buffer->Append("\r\n");

  Buffer->Append("  package: \"");
  Buffer->Append(FootprintPrefix);
  Buffer->Append(Temp->Package.String);
  Buffer->Append("\"\r\n");

  if(Temp->Value.Value.GetLength()){
   Buffer->Append("  value: \"");
   Buffer->Append(Temp->Value.Value.String);
   Buffer->Append("\" ");
   Buffer->Append(Temp->Value.Height);
   Buffer->Append(' ');
   Buffer->Append(Temp->Value.LineWidth);
   Buffer->Append(' ');
   Buffer->Append(Temp->Value.Angle);
   Buffer->Append(' ');
   Buffer->Append(Temp->Value.X);
   Buffer->Append(' ');
   Buffer->Append(Temp->Value.Y);
   Buffer->Append(' ');
   Buffer->Append(Temp->Value.Visibility ? 1 : 0);
   Buffer->Append("\r\n");
  }

  Buffer->Append("  shape: \"");
  Buffer->Append(FootprintPrefix);
  Buffer->Append(Temp->Shape.String);
  Buffer->Append("\"\r\n");

  Buffer->Append("  pos: ");
  Buffer->Append(Temp->Pos.X + XOffset);
  Buffer->Append(' ');
  Buffer->Append(Temp->Pos.Y + YOffset);
  Buffer->Append(' ');
  Buffer->Append(Temp->Pos.Side);
  Buffer->Append(' ');
  Buffer->Append(Temp->Pos.Angle);
  Buffer->Append(' ');
  Buffer->Append(Temp->Pos.Glued ? 1 : 0);
  Buffer->Append("\r\n");

  Temp = Temp->Next;
 }
}
//------------------------------------------------------------------------------

void FREEPCB::SaveNets(
 JString   * Buffer,
 const char* Prefix,
 int         XOffset,
 int         YOffset
){
 int  j, q;
 NET* Temp;

 Temp = Nets;
 while(Temp){
  Buffer->Append("\r\nnet: \"");
  Buffer->Append(Prefix);
  Buffer->Append(Temp->Name.String);
  Buffer->Append("\" ");
  Buffer->Append(Temp->PinCount);
  Buffer->Append(' ');
  Buffer->Append(Temp->ConnectionsCount);
  Buffer->Append(' ');
  Buffer->Append(Temp->AreasCount);
  Buffer->Append(' ');
  Buffer->Append(Temp->DefaultTraceWidth);
  Buffer->Append(' ');
  Buffer->Append(Temp->DefaultViaPadWidth);
  Buffer->Append(' ');
  Buffer->Append(Temp->DefaultViaHoleDiameter);
  Buffer->Append(' ');
  Buffer->Append(Temp->Visibility ? 1 : 0);
  Buffer->Append("\r\n");

  for(j = 0; j < Temp->PinCount; j++){
   Buffer->Append("  pin: ");
   Buffer->Append(Temp->Pins[j].Index);
   Buffer->Append(' ');
   Buffer->Append(Prefix);
   Buffer->Append(Temp->Pins[j].Name.String);
   Buffer->Append("\r\n");
  }

  for(j = 0; j < Temp->ConnectionsCount; j++){
   Buffer->Append("  connect: ");
   Buffer->Append(j+1);
   Buffer->Append(' ');
   Buffer->Append(Temp->Connections[j].StartPinIndex);
   Buffer->Append(' ');
   Buffer->Append(Temp->Connections[j].EndPinIndex);
   Buffer->Append(' ');
   Buffer->Append(Temp->Connections[j].SegmentCount);
   Buffer->Append(' ');
   Buffer->Append(Temp->Connections[j].Locked ? 1 : 0);
   Buffer->Append("\r\n");
   for(q = 0; q < Temp->Connections[j].SegmentCount; q++){
    Buffer->Append("    vtx: ");
    Buffer->Append(q+1);
    Buffer->Append(' ');
    Buffer->Append(Temp->Connections[j].Vertex[q].X + XOffset);
    Buffer->Append(' ');
    Buffer->Append(Temp->Connections[j].Vertex[q].Y + YOffset);
    Buffer->Append(' ');
    Buffer->Append(Temp->Connections[j].Vertex[q].PadLayer);
    Buffer->Append(' ');
    Buffer->Append(Temp->Connections[j].Vertex[q].ForceVia ? 1 : 0);
    Buffer->Append(' ');
    Buffer->Append(Temp->Connections[j].Vertex[q].ViaPadWidth);
    Buffer->Append(' ');
    Buffer->Append(Temp->Connections[j].Vertex[q].ViaHoleDiameter);
    Buffer->Append(' ');
    Buffer->Append(Temp->Connections[j].Vertex[q].TeeID);
    Buffer->Append("\r\n");

    Buffer->Append("    seg: ");
    Buffer->Append(q+1);
    Buffer->Append(' ');
    Buffer->Append(Temp->Connections[j].Segment[q].Layer);
    Buffer->Append(' ');
    Buffer->Append(Temp->Connections[j].Segment[q].Width);
    Buffer->Append(' ');
    Buffer->Append(Temp->Connections[j].Segment[q].ViaPadWidth);
    Buffer->Append(' ');
    Buffer->Append(Temp->Connections[j].Segment[q].ViaHoleDiameter);
    Buffer->Append("\r\n");
   }
   Buffer->Append("    vtx: ");
   Buffer->Append(q+1);
   Buffer->Append(' ');
   Buffer->Append(Temp->Connections[j].Vertex[q].X + XOffset);
   Buffer->Append(' ');
   Buffer->Append(Temp->Connections[j].Vertex[q].Y + YOffset);
   Buffer->Append(' ');
   Buffer->Append(Temp->Connections[j].Vertex[q].PadLayer);
   Buffer->Append(' ');
   Buffer->Append(Temp->Connections[j].Vertex[q].ForceVia ? 1 : 0);
   Buffer->Append(' ');
   Buffer->Append(Temp->Connections[j].Vertex[q].ViaPadWidth);
   Buffer->Append(' ');
   Buffer->Append(Temp->Connections[j].Vertex[q].ViaHoleDiameter);
   Buffer->Append(' ');
   Buffer->Append(Temp->Connections[j].Vertex[q].TeeID);
   Buffer->Append("\r\n");
  }
  for(j = 0; j < Temp->AreasCount; j++){
   Buffer->Append("  area: ");
   Buffer->Append(j+1);
   Buffer->Append(' ');
   Buffer->Append(Temp->Areas[j].CornerCount);
   Buffer->Append(' ');
   Buffer->Append(Temp->Areas[j].Layer);
   Buffer->Append(' ');
   Buffer->Append(Temp->Areas[j].HatchStyle);
   Buffer->Append("\r\n");
   for(q = 0; q < Temp->Areas[j].CornerCount; q++){
    Buffer->Append("    corner: ");
    Buffer->Append(q+1);
    Buffer->Append(' ');
    Buffer->Append(Temp->Areas[j].Corners[q].X + XOffset);
    Buffer->Append(' ');
    Buffer->Append(Temp->Areas[j].Corners[q].Y + YOffset);
    Buffer->Append(' ');
    Buffer->Append(Temp->Areas[j].Corners[q].Style);
    Buffer->Append(' ');
    Buffer->Append(Temp->Areas[j].Corners[q].LastCorner ? 1 : 0);
    Buffer->Append("\r\n");
   }
  }

  Temp = Temp->Next;
 }
}
//------------------------------------------------------------------------------

void FREEPCB::SaveTexts(JString* Buffer, int XOffset, int YOffset){
 TEXT* Temp;

 Temp = Texts;
 while(Temp){
  Buffer->Append("\r\ntext: \"");
  Buffer->Append(Temp->Text.String);
  Buffer->Append("\" ");
  Buffer->Append(Temp->X + XOffset);
  Buffer->Append(' ');
  Buffer->Append(Temp->Y + YOffset);
  Buffer->Append(' ');
  Buffer->Append(Temp->Layer);
  Buffer->Append(' ');
  Buffer->Append(Temp->Angle);
  Buffer->Append(' ');
  Buffer->Append(Temp->Mirror ? 1 : 0);
  Buffer->Append(' ');
  Buffer->Append(Temp->Height);
  Buffer->Append(' ');
  Buffer->Append(Temp->LineWidth);
  Buffer->Append(' ');
  Buffer->Append(Temp->Negative ? 1 : 0);
  Buffer->Append("\r\n");
  Temp = Temp->Next;
 }
}
//------------------------------------------------------------------------------

bool FREEPCB::SaveFile(const char* Filename){
 JString SaveBuffer;
 bool    b;

 File.SetFilename(Filename);
 if(File.Open(JFile::Create)){
  SaveBuffer.Append("[options]\r\n");
  SaveOptions   (&SaveBuffer);
  SaveBuffer.Append("\r\n[footprints]\r\n");
  SaveFootprints(&SaveBuffer);
  SaveBuffer.Append("\r\n[board]\r\n");
  SaveBoard     (&SaveBuffer);
  SaveBuffer.Append("\r\n[solder_mask_cutouts]\r\n");
  SaveSolderMask(&SaveBuffer);
  SaveBuffer.Append("\r\n[parts]\r\n");
  SaveParts     (&SaveBuffer);
  SaveBuffer.Append("\r\n[nets]\r\n");
  SaveNets      (&SaveBuffer);
  SaveBuffer.Append("\r\n[texts]\r\n");
  SaveTexts     (&SaveBuffer);
  SaveBuffer.Append("\r\n[end]\r\n");

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

void FREEPCB::GetRect(RECT* Rect){
 int    j;
 BOARD* Temp;

 if(!BoardOutline){
  Rect->Left   = 0;
  Rect->Bottom = 0;
  Rect->Width  = 0;
  Rect->Height = 0;
  return;
 }

 Rect->Left   =  0x7FFFFFFF;
 Rect->Bottom =  0x7FFFFFFF;
 Rect->Width  = -0x7FFFFFFF;
 Rect->Height = -0x7FFFFFFF;

 Temp = BoardOutline;
 while(Temp){
  for(j = 0; j < Temp->CornerCount; j++){
   if(Temp->Corner[j].X < Rect->Left  ) Rect->Left   = Temp->Corner[j].X;
   if(Temp->Corner[j].X > Rect->Width ) Rect->Width  = Temp->Corner[j].X;
   if(Temp->Corner[j].Y < Rect->Bottom) Rect->Bottom = Temp->Corner[j].Y;
   if(Temp->Corner[j].Y > Rect->Height) Rect->Height = Temp->Corner[j].Y;
  }
  Temp = Temp->Next;
 }

 Rect->Width  -= Rect->Left;
 Rect->Height -= Rect->Bottom;
}
//------------------------------------------------------------------------------

void FREEPCB::HideReferences(){
 JString s;
 PART*   Temp;

 Temp = Parts;
 while(Temp){
  if(Temp->RefText.Visibility){
   s.Set   ("Hiding reference of part ");
   s.Append(Temp->Reference.String);
   PostMessage(
    mtInfo,
    File.GetFilename(),
    0,
    s.String
   );
  }
  Temp->RefText.Visibility = false;
  Temp = Temp->Next;
 }
}
//------------------------------------------------------------------------------

void FREEPCB::ShowReferences(){
 JString s;
 PART*   Temp;

 Temp = Parts;
 while(Temp){
  if(!Temp->RefText.Visibility){
   s.Set   ("Showing reference of part ");
   s.Append(Temp->Reference.String);
   PostMessage(
    mtInfo,
    File.GetFilename(),
    0,
    s.String
   );
  }
  Temp->RefText.Visibility = true;
  Temp = Temp->Next;
 }
}
//------------------------------------------------------------------------------

void FREEPCB::ReferencesToValues(){
 JString s;
 PART*   Temp;

 Temp = Parts;
 while(Temp){
  if(Temp->RefText.Visibility){
   s.Set   ("Converting reference of part ");
   s.Append(Temp->Reference.String);
   PostMessage(
    mtInfo,
    File.GetFilename(),
    0,
    s.String
   );
   Temp->Value  .Value.Set   (Temp->Reference.String);
   Temp->Value  .Height     = Temp->RefText  .Height;
   Temp->Value  .LineWidth  = Temp->RefText  .LineWidth;
   Temp->Value  .Angle      = Temp->RefText  .Angle;
   Temp->Value  .X          = Temp->RefText  .X;
   Temp->Value  .Y          = Temp->RefText  .Y;
   Temp->Value  .Visibility = true;
   Temp->RefText.Visibility = false;
  }
  Temp = Temp->Next;
 }
}
//------------------------------------------------------------------------------

#define pi 3.14159265358979323846264338327950288

void FREEPCB::ReferencesToText(){
 JString s;
 PART*   Temp;
 TEXT*   Last;

 long double f, sin, cos, width, x, y;

 Last = Texts;
 while(Last && Last->Next) Last = Last->Next;

 Temp = Parts;
 while(Temp){
  if(Temp->RefText.Visibility){
   s.Set   ("Converting reference of part ");
   s.Append(Temp->Reference.String);
   PostMessage(
    mtInfo,
    File.GetFilename(),
    0,
    s.String
   );

   if(Last){
    Last->Next = new TEXT;
    Last = Last->Next;
   }else{
    Last = new TEXT;
    Texts = Last;
   }
   Last->Text.Set   (Temp->Reference.String);


   if(Temp->Pos.Side){ // Bottom
    f           = Temp->RefText.Angle / 180. * pi;
    sin         = sinl(f);
    cos         = cosl(f);
    width       = StrokeFont.GetWidth(Temp->Reference.String) *
                  (long double)Temp->RefText.Height / 40.;
    x           = Temp->RefText.X + cos*width;
    y           = Temp->RefText.Y - sin*width;

    f           = Temp->Pos.Angle / 180. * pi;
    sin         = sinl(f);
    cos         = cosl(f);

    Last->X     = Temp->Pos.X - roundl(cos*x - sin*y);
    Last->Y     = Temp->Pos.Y + roundl(sin*x + cos*y);
    Last->Angle = (-Temp->RefText.Angle + Temp->Pos.Angle + 360) % 360;
    Last->Layer = 8;

   }else{ // Top
    f           = -Temp->Pos.Angle / 180. * pi;
    sin         = sinl(f);
    cos         = cosl(f);
    Last->X     = Temp->Pos.X +
                  roundl(cos*Temp->RefText.X - sin*Temp->RefText.Y);
    Last->Y     = Temp->Pos.Y +
                  roundl(sin*Temp->RefText.X + cos*Temp->RefText.Y);
    Last->Angle = (Temp->RefText.Angle + Temp->Pos.Angle) % 360;
    Last->Layer = 7;
   }

   Last->Mirror    = Temp->Pos.Side;
   Last->Height    = Temp->RefText.Height;
   Last->LineWidth = Temp->RefText.LineWidth;
   Last->Negative  = 0;
   Last->Next      = 0;
   Temp->RefText.Visibility = false;
  }
  Temp = Temp->Next;
 }
}
//------------------------------------------------------------------------------
