/** @file

  Copyright 2006 - 2010 Unified EFI, Inc.<BR>
  Copyright (c) 2010 - 2019, Intel Corporation. All rights reserved.<BR>

  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at 
  http://opensource.org/licenses/bsd-license.php
 
  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
 
**/
/*++

Module Name:

  Support.c

Abstract:

  Test Driver of Event and Timer and Task Priority Services

--*/

#include "EventTimerTaskPriorityServicesBBTestMain.h"


VOID
NotifyFunction (
  IN EFI_EVENT                  Event,
  IN VOID                       *Context
  )
{
  UINTN     *Buffer;

  if (Context != NULL) {
    Buffer = Context;

    Buffer[1]++;
    if (Buffer[1] <= Buffer[0]) {
      Buffer[1 + Buffer[1]] = (UINTN) Event;
    }
  }

  return;
}

#if (EFI_SPECIFICATION_VERSION >= 0x00020000)
VOID
NotifyFunctionTplEx(
  IN EFI_EVENT                  Event,
  IN VOID                       *Context
  )
{
  UINTN     *Buffer;
  EFI_TPL   OldTpl;
  UINTN     EventIndex;
  UINTN     Index;
  
  if (Context != NULL) {
    Buffer = Context;

    EventIndex = Buffer[0];

    //
    // The special code check for the BBTestCreateEventEx_Func_Sub3
    // Besides AllocatePages(), CreateEventEx() may trigger the memorymap
    // change when it is out of resource in memory pool
    // Use SIGNAL_CONTEXT to block possible enter triggered by CreateEventEx
    //
    if (EventIndex != 2 && Buffer[4] == (UINTN)(SIGNAL_CONTEXT))
      return;

    //
    // It is the code execution path as expect
    // The overall layout buffer as below
    // Buffer[0] [1] [2] store 1st/2nd/3rd event index (start from 0)
    // Buffer[3] [5] [7] store the index of event notified
    // Buffer[4] [6] [8] store the tpl of notification function of 1st/2nd/3rd event notified
    //
    // since 3rd event is created at notify tpl, 1nd/2rd event at callback
    // EventIndex should be 2 here for the first enter
    // Because Context points to Buffer[2] and value(EventIndex) is 2
    // To initial the Buffer to 0xFF
    //

    if (EventIndex == 2 && Buffer[1] == (UINTN)(SIGNAL_CONTEXT)) {
      for (Index=1; Index<MAX_TEST_EVENT_NUM*2+1; Index++) {
        Buffer[Index] = (UINTN)(0xFF);
      }
    }

    Index = 3-EventIndex;

    while (1) {     
      if (Buffer[Index] == (UINTN)(0xFF)) {
        break;
      } else {
        Index += 2;
      }
    }
    
    OldTpl = gtBS->RaiseTPL (TPL_HIGH_LEVEL);
    gtBS->RestoreTPL (OldTpl);

    Buffer[Index] = EventIndex;
    Buffer[Index+1] = OldTpl;
  }

  return;
}
#endif

VOID
NotifyFunctionTpl (
  IN EFI_EVENT                  Event,
  IN VOID                       *Context
  )
{
  UINTN     *Buffer;
  EFI_TPL   OldTpl;

  if (Context != NULL) {
    Buffer = Context;

    OldTpl = gtBS->RaiseTPL (TPL_HIGH_LEVEL);
    gtBS->RestoreTPL (OldTpl);

    Buffer[1]++;
    if (Buffer[1] <= Buffer[0]) {
      Buffer[1 + Buffer[1]] = (UINTN) OldTpl;
    }
  }

  return;
}


VOID
NotifyFunctionSignal (
  IN EFI_EVENT                  Event,
  IN VOID                       *Context
  )
{
  gtBS->SignalEvent (Event);
  return;
}

VOID
NotifyFunctionNoSignal (
  IN EFI_EVENT                  Event,
  IN VOID                       *Context
  )
{
  return;
}
