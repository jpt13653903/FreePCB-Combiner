//==============================================================================
// Netlist File Abstraction Header
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

#ifndef NETLIST_H
#define NETLIST_H
//------------------------------------------------------------------------------

#include "Types.h"
#include "JFile.h"
#include "JData.h"
//------------------------------------------------------------------------------

class NETLIST{
 public:
  struct PART{
   JString Reference;
   JString Value;
   JString Package;
  };

  JData Parts;

  struct NET{
   JString Body;
  };
  NET* Nets;

 private:
  JFile File;

  bool Error;

  char* Buffer;
  int   BufferSize;
  int   Line;
  int   Index;

  void DoParts();
  void DoNets();

  bool GetNewLine();
  void RemoveSpace();
  bool GetString (JString* String );
  bool GetSection(JString* Section);
  void ParseFile();

  void SaveParts(JString* Buffer);
  void SaveNets (JString* Buffer);

 public:
  NETLIST();
 ~NETLIST();

 void Clear();
 bool LoadFile(const char* Filename);
 bool SaveFile(const char* Filename);

 POST_MESSAGE_FUNC PostMessage;
};
//------------------------------------------------------------------------------

#endif
//------------------------------------------------------------------------------
