/* Dispatch commands from the interpreter into the KWM structure. */

#ifndef DISPATCHER_H
#define DISPATCHER_H

/* Set Focus to Window in first Leaf

    Map:
        Display (KWMScreen.Current) ~> Node

    Parameters:
        (none)

    Global References:
        KWMScreen.Current

    Mutations:
        (see GetActiveSpaceOfScreen)
        (see SetWindowFocusByNode)
        (see MoveCursorToCenterOfFocusedWindow)

    Return:
        (void)

    Called Functions:
        space      :: DoesSpaceExistInMapOfScreen(KWMScreen.Current))
    [M] space      :: GetActiveSpaceOfScreen(KWMScreen.Current)
        tree       :: GetFirstLeafNode(...)
    [M] windowtree :: SetWindowFocusByNode(...)
    [M] windowref  :: MoveCursorToCenterOfFocusedWindow();

    Calling Functions:
        interpreter :: KwmWindowCommand

    Notes:
     - Only used by the interpreter.
     - Calls MoveCursor afterward.
     - Related to ShiftWSetWindowFocusByNodeindowFocus()

    TODO: Add abstraction - Interpreter ~> KWM ~> Display ~> Space ~> Tree ~> Node
*/
void FocusFirstLeafNode();

/* Set Focus to Window in last Leaf

    Map:
        Display (KWMScreen.Current) ~> Node

    Parameters:
        (none)

    Global References:
        KWMScreen.Current

    Mutations:
        (see GetActiveSpaceOfScreen)
        (see SetWindowFocusByNode)
        (see MoveCursorToCenterOfFocusedWindow)

    Return:
        (void)

    Called Functions:
        space      :: DoesSpaceExistInMapOfScreen(KWMScreen.Current))
    [M] space      :: GetActiveSpaceOfScreen(KWMScreen.Current)
        tree       :: GetLastLeafNode(...)
    [M] windowtree :: SetWindowFocusByNode(...)
    [M] windowref  :: MoveCursorToCenterOfFocusedWindow();

    Calling Functions:
        interpreter :: KwmWindowCommand

    Notes:
     - Only used by the interpreter.
     - Calls MoveCursor afterward.
     - Related to ShiftWindowFocus()

    TODO: Add abstraction barrier: Interpreter ~> KWM ~> Display ~> Space ~> Tree ~> Node
*/
void FocusLastLeafNode();

/* Set Focus to WindowID; if on a different Screen, change focus of Screen

    Map:
        Display ~> WindowRef

    Parameters:
        WindowID - The WID of the window to focus

    Global References:
        KWMScreen.Current

    Mutations:
        (see GiveFocusToScreen)
        (see SetWindowFocus)
        (see MoveCursorToCenterOfFocusedWindow)

    Return:
        (void)

    Called Functions:
        display   :: GetDisplayOfWindow(..)
    [M] display   :: GiveFocusToScreen(...)
        space     :: IsSpaceInitializedForScreen(...)
        window    :: GetWindowByID(WindowID)
    [M] windowref :: SetWindowFocus(..)
    [M] windowref :: MoveCursorToCenterOfFocusedWindow()

    Calling Functions:
        interpreter :: KwmSpaceCommand(..)

    Notes:
     - Only called from interpreter.

    TODO: Add abstraction
*/
void FocusWindowByID(int WindowID);

/* Focus the next/previous window

    Map:
        Display (KWMScreen.Current) ~> Node

    Parameters:
        Shift - +1 : Shift focus Right
                -1 : Shift focus Left

    Global References:
        KWMFocus.Window
        KWMScreen.Current
        KWMMode.Cycle

    Mutations:
        (see GiveFocusToScreen)
        (see GetActiveSpaceOfScreen)
        (see SetWindowFocusByNode)
        (see MoveCursorToCenterOfFocusedWindow)

    Return:
        (void)

    Called Functions:
        display    :: GetDisplayFromScreenID(..)
        display    :: GetIndexOfNextScreen()
    [M] display    :: GiveFocusToScreen(..., false)
        space      :: DoesSpaceExistInMapOfScreen(KWMScreen.Current)
    [M] space      :: GetActiveSpaceOfScreen(KWMScreen.Current)
        tree       :: GetNodeFromWindowID(.., KWMFocus.Window->WID, ..);
        tree       :: GetNearestNodeToTheRight(...)
        tree       :: GetNearestNodeToTheLeft(...)
        tree       :: GetFirstLeafNode(..)
        tree       :: GetLastLeafNode(..)
    [M] windowtree :: SetWindowFocusByNode(..)
    [M] windowref  :: MoveCursorToCenterOfFocusedWindow()

    Calling Functions:
        interpreter :: KwmSpaceCommand(..)

    Notes:
     - Can combine the two Shift blocks -- they just determine which node to get (nearest L/R)
     - Only called from interpreter.
*/
void ShiftWindowFocus(int Shift);

/* Move Focus to the North, East, South, or West Window.

    Map:
        Display (KWMScreen.Current) ~> WindowRef

    Parameters:
       Degrees - rotate focus North = 0, East = 90, South = 180, West = 270

    Global References:
        KWMFocus.Window
        KWMScreen.Current
        KWMScreen.Current->ID
        KWMMode.Cycle

    Mutations:
        (see GiveFocusToScreen)
        (see GetActiveSpaceOfScreen)
        (see ShiftWindowFocus)
        (see SetWindowFocus)
        (see MoveCursorToCenterOfFocusedWindow)

    Return:
        (void)

    Called Functions:
        display    :: GetIndexOfNextScreen()
        display    :: GetIndexOfPrevScreen()
        display    :: GetDisplayFromScreenID(..)
    [M] display    :: GiveFocusToScreen(..., false)
        space      :: DoesSpaceExistInMapOfScreen(KWMScreen.Current)
    [M] space      :: GetActiveSpaceOfScreen(KWMScreen.Current)
        tree       :: GetFirstLeafNode(..)
        tree       :: GetLastLeafNode(..)
    [M] dispatcher :: ShiftWindowFocus(..)
    [M] windowref  :: SetWindowFocus(..)
    [M] windowref  :: MoveCursorToCenterOfFocusedWindow()

    Calling Functions:
        interpreter :: KwmSpaceCommand

    Notes:
     - Only called from interpreter

    TODO: Refactor into existing functions.
*/
void ShiftWindowFocusDirected(int Degrees);

/* Remove a Window from the Tree and Add it back to a position given by Degrees.

    Map:
        WindowID ~> Tree

    Parameters:
       WindowID - the WindowID of the Window to operate on
       Degrees - insert WindowID at: North = 0, East = 90, South = 180, West = 270

    Global References:
    [M] KWMScreen.MarkedWindow
        KWMFocus.Window->WID

    Mutations:
        KWMScreen.MarkedWindow set to WindowID

    Return:
        (void)

    Called Functions:
        ToggleWindowFloating(WindowID)
        ClearMarkedWindow()
        ToggleWindowFloating(WindowID)
        MoveCursorToCenterOfFocusedWindow()
        FindClosestWindow(Degrees, .., false)

    Calling Functions:
        interpreter::KWMWindowCommand(..)
    
    Notes:
      - Only called from the interpreter
      - Can combine both blocks as they do almost the same thing.
      - "Degrees" isn't the best argument here.
*/
void DetachAndReinsertWindow(int WindowID, int Degrees);

/* Focused Window fills the Parent Container

    Map:
        Focus.Window->WID ~> Tree

    Parameters:
       (none)

    Global References:
        KWMFocus.Window->WID
        KWMScreen.Current

    Mutations:
        (see ToggleElementInTree)

    Return:
        (void)

    Called Functions:
        space::DoesSpaceExistInMapOfScreen(KWMScreen.Current)
        space::GetActiveSpaceOfScreen(KWMScreen.Current)
        tree::ToggleElementInTree(KWMScreen.Current, .., KWMFocus.Window->WID, ...)

    Calling Functions:
        interpreter::KwmWindowCommand(..)
        border::UpdateBorder()
    
    Notes:
      - Only called from interpreter

    TODO: Rename function to make clear that it fills parent container
*/
void ToggleFocusedWindowParent();

/* Toggle Focused Window filling Root Container (fullscreen)

    Map:
        KWMFocus.Window->WID ~> Tree

    Parameters:
       (none)

    Global References:
        KWMFocus.Window->WID
        KWMScreen.Current

    Mutations:
        (see ToggleElementInRoot)

    Return:
        (void)

    Called Functions:
        space::DoesSpaceExistInMapOfScreen(KWMScreen.Current)
        space::GetActiveSpaceOfScreen(KWMScreen.Current)
        tree::ToggleElementInRoot(KWMScreen.Current, .., KWMFocus.Window->WID, ...)
        border::UpdateBorder()

    Calling Functions:
        interpreter::KWMWindowCommand(..)
    
    Notes:
      - Only called from interpreter
      - See if this can be combined with the ToggleFocusedWindow* functions.
*/
void ToggleFocusedWindowFullscreen();

/* Remove a Window from the Tree to let it float, or Add it back in if it's already floating

    Map:
        WindowID ~> Tree

    Parameters:
        WindowID - the WID of the Window to make Floating

    Global References:
        KWMScreen.Current
    [M] KWMTiling.FloatingWindowLst
    [M] KWMMode.Focus
        KWMToggles.StandbyOnFloat

    Mutations:
        (see AddWindowToBSPTree)
        (see RemoveWindowFromBSPTree)
        KWMMode.Focus changed
        KWMTiling.FloatingWindowLst add/remove WindowID

    Return:
        (void)

    Called Functions:
        tree::AddWindowToBSPTree(KWMScreen.Current, WindowID)
        tree::RemoveWindowFromBSPTree(KWMScreen.Current, WindowID, true)
        window::IsWindowOnActiveSpace(WindowID)
        window::IsWindowFloating(WindowID, ..)

    Calling Functions:
        ToggleFocusedWindowFloating()
        DetatchAndReinsertWindow(WindowID, ..)
*/
void ToggleWindowFloating(int WindowID);

/* Toggle Floating on the Focused Window

    Map:
        <interpreter> ~> Window floating

    Parameters:
       (none)

    Global References:
        KWMFocus.Window

    Mutations:
        (see ToggleFocusedWindowFloating(int))

    Return:
        (void)

    Called Functions:
        ToggleFocusedWindowFloating(int)

    Calling Functions:
        display::GiveFocusToScreen
        interpreter::KwmWindowCommand
    
    Notes:
        Just a wrapper defaulting to the KWMFocus
*/
void ToggleFocusedWindowFloating(); // calls window->tree function

/* Swap focused Window with Marked Window

    Map:
        KWMFocus.Window, KWMScreen.MarkedWindow ~> Tree

    Parameters:
       (none)

    Global References:
        KWMFocus.Window->WID
        KWMScreen.MarkedWindow
        KWMScreen.Current

    Mutations:
        (see SwapNodeWindowIDs)

    Return:
        (void)

    Called Functions:
        space::DoesSpaceExistInMapOfScreen(KWMScreen.Current)
        space::GetActiveSpaceOfScreen(KWMScreen.Current)
        tree::GetNodeFromWindowID(.., KWM**, ..)
        tree::SwapNodeWindowIDs(...)
        window::ClearMarkedWindow()
        windowref::MoveCursorToCenterOfFocusedWindow()

    Calling Functions:
        interpreter::KwmSpaceCommand(..)
    
    Notes:
      - Only called from interpreter
*/
void SwapFocusedWindowWithMarked();

/* Swap Focused Window with Window in direction (North, East, South, West)

    Map:
        Focused Window, Degrees ~> Tree

    Parameters:
       Degrees - swap with Window at: North = 0, East = 90, South = 180, West = 270

    Global References:
        KWMFocus.Window->WID
        KWMScreen.Current
        KWMScreen.MarkedWindow
        KWMMode.Cycle

    Mutations:
        (see SwapNodeWindowIDs)

    Return:
        (void)

    Called Functions:
        space::DoesSpaceExistInMapOfScreen(KWMScreen.Current)
        space::GetActiveSpaceOfScreen(KWMScreen.Current)
        tree::GetNodeFromWindowID(.., KWMFocus.Window->WID, ..)
        tree::GetNearestNodeToTheRight(...)
        tree::GetNearestNodeToTheLeft(...)
        tree::SwapNodeWindowIDs(...)
        window::FindClosestWindow(Degrees, .., KWMMode.Cycle == CycleModeScreen)
        windowref::MoveCursorToCenterOfFocusedWindow()
        border::UpdateBorder("marked")

    Calling Functions:
        interpreter::KwmWindowCommand
    
    Notes:
      - Only called from interpreter
*/
void SwapFocusedWindowDirected(int Degrees);

/* Swap Focused Window with Window to Right (+1) or Left (-1)

    Map:
        KWMFocus.Window->WID, Shift ~> Tree

    Parameters:
       Shift - +1 : Swap with Right
               -1 : Swap with Left

    Global References:
        KWMFocus.Window->WID
        KWMScreen.Current
        KWMScreen.MarkedWindow

    Mutations:
        (see SwapNodeWindowIDs)

    Return:
        (void)

    Called Functions:
        space::DoesSpaceExistInMapOfScreen(KWMScreen.Current)
        space::GetActiveSpaceOfScreen(KWMScreen.Current)
        tree::GetNodeFromWindowID(.., KWMFocus.Window->WID, ..)
        tree::GetNearestNodeToTheRight(...)
        tree::GetNearestNodeToTheLeft(...)
        tree::SwapNodeWindowIDs(...)
        windowref::MoveCursorToCenterOfFocusedWindow()
        border::UpdateBorder("marked")

    Calling Functions:
        interpreter::KwmWindowCommand(..)
    
    Notes:
      - Only called from interpreter
      - See if the SwapFocused ** functions can be combined.
      - See if the ShiftFOcused ** functions can be combined.
*/
void SwapFocusedWindowWithNearest(int Shift);

/* Change Focused Window's subtree's Split Ratio

    Map:
        KWMFocus.Window, Offset ~> Tree

    Parameters:
       Offset - 

    Global References:
        KWMScreen.Current
        KWMFocus.Window->WID

    Mutations:
        (see ModifySubtreeSplitRatio)

    Return:
        (void)

    Called Functions:
        space::DoesSpaceExistInMapOfScreen(KWMScreen.Current)
        space::GetActiveSpaceOfScreen(KWMScreen.Current)

    Calling Functions:
        interpreter::KwmWindowCommand(..)
        tree::IsLeafNode(..)
        tree::GetNodeFromWindowID(.., KWMFocus.Window->WID, ..)
        tree::ModifySubtreeSplitRatio(KWMScreen.Current, .., Offset, ...)
    
    Notes:
      - Only called from interpreter
      - Should maybe update the title to say "Focused" something or other
*/
void ModifySubtreeSplitRatioFromWindow(const double &Offset);

/* Resize Focused Window to its Container size

    Map:
        KWMScreen.Current, KWMFocus.Window ~> Tree

    Parameters:
       (none)

    Global References:
        KWMScreen.Current
        KWMFocus.Window

    Mutations:
        (see ResizeElementInTree)

    Return:
        (void)

    Called Functions:
        ResizeElementInTre(KWMScreen.Current, KWMFocus.Window)

    Calling Functions:
        interpreter::KwmWindowCommand(..)
    
    Notes:
      - Only called by interpreter

    TODO: Should put the "Focused" part in the function name
*/
void ResizeWindowToContainerSize();

#endif
