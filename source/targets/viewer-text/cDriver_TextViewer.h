/*
 *  cDriver_TextViewer.h
 *  Avida
 *
 *  Created by Charles on 7-1-07
 *  Copyright 1999-2009 Michigan State University. All rights reserved.
 *
 *
 *  This file is part of Avida.
 *
 *  Avida is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License
 *  as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 *
 *  Avida is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License along with Avida.
 *  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef cDriver_TextViewer_h
#define cDriver_TextViewer_h

#ifndef cAvidaDriver_h
#include "cAvidaDriver.h"
#endif

#ifndef cCoreView_Info_h
#include "cCoreView_Info.h"
#endif

#ifndef cScreen_Map_h
#include "cScreen_Map.h"
#endif
 
#ifndef cTextWindow_h
#include "cTextWindow.h"
#endif

#ifndef cWorldDriver_h
#include "avida/core/cWorldDriver.h"
#endif

#ifndef ncurses_defs_h
#include "ncurses-defs.h"
#endif

#include <sstream>
#include <iostream>
#include <fstream>

class cWorld;

using namespace std;

class cDriver_TextViewer : public cAvidaDriver, public cWorldDriver {
private:
  cWorld* m_world;
  cCoreView_Info m_info;
  cTextWindow m_main_window;
  cTextWindow m_bar_window;

  cScreen_Map m_screen_map;

  cTextScreen * m_cur_screen;

  bool m_done;           // This is set to true when run should finish.

  void ChangeCurScreen(cTextScreen * new_screen);
public:
  cDriver_TextViewer(cWorld* world);
  ~cDriver_TextViewer();
  
  void Run();
  
  // Driver Actions
  void SignalBreakpoint();
  void SetDone() { m_done = true; }
  void SetPause() { ; }

  // IO
  void Flush();
  int GetKeypress() { return getch(); }
  bool ProcessKeypress(int keypress);
  void NoDelay(bool _nd=true) { nodelay(stdscr, _nd); }  // Don't wait for input if no key is pressed.

  void RaiseException(const cString& in_string);
  void RaiseFatalException(int exit_code, const cString& in_string);

  // Drawing and interaction.
  void Draw();
  void DoUpdate();

  // Notifications
  void NotifyComment(const cString& in_string);
  void NotifyWarning(const cString& in_string);
  void NotifyError(const cString& in_string);
  void NotifyOutput(const cString& in_string);
  void Notify(const cString& in_string);

  int Confirm(const cString& in_string);

  // Tests
  bool IsInteractive() { return true; }
};


void ExitTextViewer(int exit_code);

#endif
