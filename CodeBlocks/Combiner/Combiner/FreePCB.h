//==============================================================================
// FreePCB File Abstraction Header
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

#ifndef FREEPCB_H
#define FREEPCB_H
//------------------------------------------------------------------------------

#include "Types.h"
#include "JFile.h"
#include "StrokeFont.h"
//------------------------------------------------------------------------------

class FREEPCB{
 public:
  struct OPTIONS{
   JString Body;
  };
//------------------------------------------------------------------------------

  struct FOOTPRINT{
   JString    Name;
   JString    Body;
   FOOTPRINT* Next;
  };
//------------------------------------------------------------------------------

  struct CORNER{ // 1-based index
   int X;     // nm
   int Y;     // nm
   int Style; // Style of the next side
  };

  struct BOARD{ // 0-based index
   int     CornerCount;
   CORNER* Corner; // Array
   BOARD * Next;
  };
//------------------------------------------------------------------------------

  struct SOLDERMASK{
   int         CornerCount;
   int         FillStyle;
   int         Layer;
   CORNER    * Corner; // Array
   SOLDERMASK* Next;
  };
//------------------------------------------------------------------------------

  struct REF_TEXT{
   int  Height;
   int  LineWidth;
   int  Angle; // Degrees
   int  X;
   int  Y;
   bool Visibility;
  };

  struct POS{
   int  X;     // nm
   int  Y;     // nm
   int  Side;
   int  Angle; // Degrees
   bool Glued;
  };

  struct VALUE{
   JString Value;
   int     Height;
   int     LineWidth;
   int     Angle; // Degrees
   int     X;
   int     Y;
   bool    Visibility;
  };

  struct PART{
   JString  Reference;
   REF_TEXT RefText;
   JString  Package;
   JString  Shape;
   POS      Pos;
   VALUE    Value;
   PART*    Next;
  };
//------------------------------------------------------------------------------

  struct VERTEX{ // 1-based index
   int  X; // nm
   int  Y; // nm
   int  PadLayer;
   bool ForceVia;
   int  ViaPadWidth;
   int  ViaHoleDiameter;
   int  TeeID;
  };

  struct SEGMENT{ // 1-based index
   int Layer;
   int Width;
   int ViaPadWidth;
   int ViaHoleDiameter;
  };

  struct PIN{ // 1-based index
   int     Index;
   JString Name;
  };

  struct CONNECT{ // 1-based index
   int      StartPinIndex;
   int      EndPinIndex;
   int      SegmentCount;
   bool     Locked;
   VERTEX * Vertex;  // Array
   SEGMENT* Segment; // Array
  };

  struct AREA_CORNER{ // 1-based index
   int  X;
   int  Y;
   int  Style; // Style of next side
   bool LastCorner;
  };

  struct AREA{ // 1-based index
   int          CornerCount;
   int          Layer;
   int          HatchStyle;
   AREA_CORNER* Corners; // Array
  };

  struct NET{
   JString Name;
   int     PinCount;
   int     ConnectionsCount;
   int     AreasCount;
   int     DefaultTraceWidth;
   int     DefaultViaPadWidth;
   int     DefaultViaHoleDiameter;
   bool    Visibility;
   PIN    * Pins;        // Array
   CONNECT* Connections; // Array
   AREA   * Areas;       // Array
   NET    * Next;
  };
//------------------------------------------------------------------------------

  struct TEXT{
   JString Text;
   int     X;
   int     Y;
   int     Layer;
   int     Angle;
   bool    Mirror;
   int     Height;
   int     LineWidth;
   bool    Negative;
   TEXT*   Next;
  };
//------------------------------------------------------------------------------

  OPTIONS   * Options;
  FOOTPRINT * Footprints;       // Linked List
  BOARD     * BoardOutline;     // Linked List
  SOLDERMASK* SolderMaskCutout; // Linked List
  PART      * Parts;            // Linked List
  NET       * Nets;             // Linked List
  TEXT      * Texts;            // Linked List
//------------------------------------------------------------------------------

  struct RECT{
   int Left;   // nm
   int Bottom; // nm
   int Width;  // nm
   int Height; // nm
  };
//------------------------------------------------------------------------------

 private:
  JFile File;

  STROKEFONT StrokeFont;

  bool Error;

  char* Buffer;
  int   BufferSize;
  int   Line;
  int   Index;

  void GetFootprintBody(JString    * Body   );
  bool GetCorner       (CORNER     * Corner );
  void GetRefText      (REF_TEXT   * RefText);
  void GetPackage      (JString    * Package);
  void GetShape        (JString    * Shape  );
  void GetPos          (POS        * Pos    );
  void GetValue        (VALUE      * Value  );
  bool GetPin          (PIN        * Pin    );
  bool GetConnect      (CONNECT    * Connect);
  bool GetArea         (AREA       * Area   );
  bool GetVertex       (VERTEX     * Vertex );
  bool GetSegment      (SEGMENT    * Segment);
  bool GetAreaCorner   (AREA_CORNER* Corner );

  void DoOptions();
  void DoFootprints();
  void DoBoard();
  void DoSolderMask();
  void DoParts();
  void DoNets();
  void DoTexts();

  bool GetNewLine();
  void RemoveSpace();
  bool GetInteger(int    * Integer);
  bool GetString (JString* String );
  bool GetKeyword(JString* Keyword);
  bool GetSection(JString* Section);
  void ParseFile();
//------------------------------------------------------------------------------

 public:
  FREEPCB();
 ~FREEPCB();

  void Clear();
  bool LoadFile(const char* Filename);
  bool SaveFile(const char* Filename);

  POST_MESSAGE_FUNC PostMessage;

  void SaveOptions   (JString* Buffer);
  void SaveFootprints(JString* Buffer, const char* Prefix = "");
  void SaveBoard     (JString* Buffer, int StartIndex = 0,
                      int XOffset = 0, int YOffset = 0);
  void SaveSolderMask(JString* Buffer, int XOffset = 0, int YOffset = 0);
  void SaveParts     (JString* Buffer,
                      const char* FootprintPrefix = "",
                      const char* PartPrefix      = "",
                      int XOffset = 0, int YOffset = 0);
  void SaveNets      (JString* Buffer, const char* Prefix = "",
                      int XOffset = 0, int YOffset = 0);
  void SaveTexts     (JString* Buffer, int XOffset = 0, int YOffset = 0);

  void GetRect(RECT* Rect);
  void HideReferences();
  void ShowReferences();
  void ReferencesToValues();
  void ReferencesToText();
};
//------------------------------------------------------------------------------

#endif
//------------------------------------------------------------------------------
