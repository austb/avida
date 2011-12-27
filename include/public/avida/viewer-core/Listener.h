/*
 *  viewer-core/Listener.h
 *  avida-core
 *
 *  Created by David on 11/11/10.
 *  Copyright 2010-2011 Michigan State University. All rights reserved.
 *  http://avida.devosoft.org/
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
 *  Authors: David M. Bryson <david@programerror.com>
 *
 */

#ifndef AvidaCoreViewListener_h
#define AvidaCoreViewListener_h


namespace Avida {
  namespace CoreView {
    class Map;
    
    class Listener
    {
    public:
      virtual ~Listener() = 0;
      
      virtual bool WantsMap() = 0;
      virtual bool WantsUpdate() = 0;
      
      virtual void NotifyMap(Map* map) { (void)map; }
      virtual void NotifyUpdate(int update) { (void)update; }      
    };
    
  };
};

#endif
