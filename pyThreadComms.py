#!/usr/bin/env python
# Copyright Hugh Perkins 2004
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
# or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
#  more details.
#
# You should have received a copy of the GNU General Public License along
# with this program in the file licence.txt; if not, write to the
# Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-
# 1307 USA
# You can find the licence also on the web at:
# http://www.opensource.org/licenses/gpl-license.php
#

import threading

WorkerTheadCommsMutex = threading.RLock()

workerthreadincomingmessages = []

def SendMessageToWorkerThread( messageobject ):
   global workerthreadincomingmessages, WorkerTheadCommsMutex
   
   WorkerTheadCommsMutex.acquire()
   workerthreadincomingmessages.append( messageobject )
   WorkerTheadCommsMutex.release()

def GetNextMessageForWorkerThread():
   global workerthreadincomingmessages, WorkerTheadCommsMutex
   
   objecttoreturn = None
   WorkerTheadCommsMutex.acquire()
   if len( workerthreadincomingmessages ) > 0:
      objecttoreturn = workerthreadincomingmessages[0]
      workerthreadincomingmessages = workerthreadincomingmessages[1:]
   WorkerTheadCommsMutex.release()
   return objecttoreturn

