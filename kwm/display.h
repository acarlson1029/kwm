/* NOTES
 * OSX uses a DisplayID to keep track of the different displays in the system.
 * CGGetActiveDisplayList gets an array with the DisplayIDs
 *
 * This works for the user, too, since they'll often want to specify things like "move to display 3"
 * This requires having an easy way to map between the DisplayID and the Display objects.
 *   Current System: KWM stores a D$, and each Display has an ID
 *                   Walk through D$ to see if any Display->ID matches the given ID
*/
#ifndef DISPLAY_H
#define DISPLAY_H

#include "types.h"

extern int GetActiveSpaceOfDisplay(screen_info *Screen);

/* Callback to detect new Displays, detect sleeping Displays, or erase removed Displays by OS X Display ID

    Map:
        OS X ~> Display

    Parameters:
        Display - the unique ID of the Display in OS X
            (see https://developer.apple.com/library/mac/documentation/GraphicsImaging/Reference/Quartz_Services_Ref/#//apple_ref/c/tdef/CGDirectDisplayID)
        Flags - flags describing the state change of the display
            (see https://developer.apple.com/library/prerelease/mac/documentation/GraphicsImaging/Reference/Quartz_Services_Ref/index.html#//apple_ref/c/tdef/CGDisplayChangeSummaryFlags)
        UserInfo - (unused) // TODO Remove if unused
        
    Global References:
        KWMThread.Lock
    [M] KWMTiling.DisplayMap[Display]
    [M] KWMTiling.DisplayMap[Display].Identifier

    Mutations:
        KWMTiling.DisplayMap[Display].Identifier released if display asleep
        KWMTiling.DisplayMap[Display] erased if asleep
    
    Return:
        (void)

    Called Functions:
        sys     :: pthread_mutex_lock(&KWMThread.Lock)
        sys     :: pthread_mutex_unlock(&KWMThread.Lock);
        osx     :: CGDisplayIsAsleep(Display)
        display :: RefreshActiveDisplays() // TODO is mutating call?
        tree    :: DestroyNodeTree()

    Calling Functions:
        display::GetActiveDisplays()
    
    Notes:

    TODO: Encapsulate the display / space / tree logic
*/
void DisplayReconfigurationCallBack(CGDirectDisplayID Display, CGDisplayChangeSummaryFlags Flags, void *UserInfo);

/* Initialize KWM's Display information

    Map:
        OSX ~> Display

    Parameters:
        (none)
        
    Global References:
    [M] KWMScreen.Displays
        KWMScreen.MaxCount
    [M] KWMScreen.ActiveCount
        KWMScreen.Current
    [M] KWMScreen.Current->ActiveSpace
        KWMTiling.DisplayMap

    Mutations:
        KWMScreen.Displays is reinitialized with malloc
        KWMScreen.Displays is set to a list of DisplayIDs by CGGetActiveDisplayList
        KWMScreen.ActiveCount is set to the current active Display count.
        KWMTiling.DisplayMap is set for each DisplayID
        KWMScreen.Current->ActiveSpace is set to the active Space of the current Display

    Return:
        (void)

    Called Functions:
    [M] osx       :: CGGetActiveDisplayList
        osx       :: CGDisplayRegisterReconfigurationCallback(DisplayReconfigurationCallBack, NULL);
        workspace :: GetActiveSpaceOfDisplay(KWMScreen.Current)
        display   :: CreateDefaultScreenInfo()
        display   :: GetDisplayOfMousePointer()
        space     :: ShouldActiveSpaceBeManaged()

    Calling Functions:
        kwm :: KwmInit()
    
    Notes:
      - Only called during INIT

    TODO: Shouldn't have mutations in a "GET" function
    TODO: Should have a separate display initialization which gets called by the KwmInit function
*/
void GetActiveDisplays();

/* Reset KWM's Display information

    Map:
        OSX ~> Display

    Parameters:
        (none)
        
    Global References:
    [M] KWMScreen.Displays
        KWMScreen.MaxCount
    [M] KWMScreen.ActiveCount
    [M] KWMTiling.DisplayMap

    Mutations:
        KWMScreen.Displays is deleted, then malloc'ed
        KWMScreen.Displays is set to a list of DisplayIDs by CGGetActiveDisplayList
        KWMScreen.ActiveCount is reset to the OS X value
        KWMTiling.DisplayMap[ID] is set to new Display

    Return:
        (void)

    Called Functions:
    [M] osx     :: CGGetActiveDisplayList(KWMScreen.MaxCount, KWMScreen.Displays, &KWMScreen.ActiveCount)
        display :: UpdateExistingScreenInfo(&KWMTiling.DisplayMap[<ID>], <ID>, ...) // TODO is mutating?
        display :: CreateDefaultScreenInfo() // TODO is mutating?
        display :: GetDisplayOfMousePointer() // TODO is mutating?
    [M] display :: GiveFocusToScreen()

    Calling Functions:
        display :: DisplayReconfigurationCallBack()
    
    Notes:
        // TODO

    TODO: This function duplicates some logic in the GetActiveDisplays() function
*/
void RefreshActiveDisplays();

/* Get the Display ID of the next Display

    Map:
        Display ~> Display

    Parameters:
        (none)
        
    Global References:
        KWMScreen.Current->ID
        KWMScreen.ActiveCount

    Mutations:
        (none)

    Return:
        int - the Display ID of the next Display

    Called Functions:
        (none)

    Calling Functions:
        interpreter :: KwmScreenCommand()
        dispatcher  :: ShiftWindowFocus()
        dispatcher  :: ShiftWindowFocusDirected()
        display     :: MoveWindowToDisplay()
    
    Notes:
      - uses OS X display ID convention to determine next/prev
*/
int GetIndexOfNextScreen();

/* Get the Display ID of the previous Display

    Map:
        Display ~> Display

    Parameters:
        (none)
        
    Global References:
        KWMScreen.Current->ID
        KWMScreen.ActiveCount

    Mutations:
        (none)

    Return:
        int - the Display ID of the previous Display

    Called Functions:
        (none)

    Calling Functions:
        interpreter :: KwmScreenCommand()
        dispatcher  :: ShiftWindowFocus()
        dispatcher  :: ShiftWindowFocusDirected()
        display     :: MoveWindowToDisplay()
    
    Notes:
      - uses OS X display ID convention to determine next/prev

    TODO: could probably implement a "GetIndexOfScreen(<relative pos>)"
          ex. GetIndexOfScreen(+1), GetIndexOfScreen(-1)
*/
int GetIndexOfPrevScreen();

/* Get the Display pointer from a given DisplayID

    Map:
        Display ~> Display

    Parameters:
        ID - the DisplayID to get the Display for
        
    Global References:
        KWMTiling.DisplayMap

    Mutations:
        (none)

    Return:
        screen_info - the Display, if found in D$, else NULL

    Called Functions:
        (none)

    Calling Functions:
        application :: CaptureApplication()
        dispatcher  :: ShiftWindowFocus()
        dispatcher  :: ShiftWindowFocusDirected()
        display     :: GetAllWindowsOnDisplay()
        display     :: GetAllWindowIDsOnDisplay()
        display     :: GiveFocusToScreen()
        display     :: CaptureApplicationToScreen()
        display     :: MoveWindowToDisplay()
    
    Notes:
      - Walks the D$ looking for the Display
*/
screen_info *GetDisplayFromScreenID(unsigned int ID);

/* Get the Display pointer from the current Mouse position

    Map:
        Display ~> Display

    Parameters:
        (none)

    Global References:
        KWMTiling.DisplayMap

    Mutations:
        (none)

    Return:
        screen_info - the Display the mouse pointer is currently on

    Called Functions:
        window::GetCursorPos()

    Calling Functions:
        display :: RefreshActiveDisplays()
        display :: GetActiveDisplays()
        display :: GetDisplayOfWindow()
        display :: ChangePaddingOfDisplay()
        display :: ChangeGapOfDisplay()
        display :: UpdateActiveScreen()
    
    Notes:
      - Walks through the D$
      - Checks if the cursor X/Y positions are within the display's boundaries.
        - on OS X, the second displays have X coordinates corresponding to (Sum(PrevDisplays.Resolution.X) + CurDisplay.Offset)

    TODO: Can add a generic "is within bounds" function which would be useful for containers too.
    TODO: Can convert the Cursor/Screen elements into a dimensions type struct
*/
screen_info *GetDisplayOfMousePointer();

/* Get the Display pointer from the given Window position

    Map:
        Display ~> Display

    Parameters:
        Window - the Window whose position is used to determine its Display
        
    Global References:
        KWMTiling.DisplayMap

    Mutations:
        (none)

    Return:
        screen_info - the Display found:
                      (1) if Window coordinates found in D$, return Display from D$
                      (2) if Window not found in D$, return Display from MousePointer
                      (3) if not Window, return NULL

    Called Functions:
        display :: GetDisplayOfMousePointer()

    Calling Functions:
        application :: CaptureApplication()
        dispatcher  :: FocusWindowByID()
        display     :: GetAllWindowsOnDisplay()
        windowtree  :: CreateWindowNodeTree()
        windowtree  :: AddWindowToTreeOfUnfocusedMonitor()
        window      :: FilterWindowList()
        windowref   :: IsWindowNonResizable()
    
    Notes:
      - Walks through the D$
      - Checks if the window X/Y positions are within the display's boundaries.
        - on OS X, the second displays have X coordinates corresponding to (Sum(PrevDisplays.Resolution.X) + CurDisplay.Offset)
      - Only checks the X/Y, not the center, so the top-left corner is the determiner

    TODO: Can add a generic "is within bounds" function which would be useful for containers too.
    TODO: Can make the window use a "Dimensions" struct
*/
screen_info *GetDisplayOfWindow(window_info *Window);

/* Get all of the managed non-floating Windows on a given Display

    Map:
        Display ~> Display

    Parameters:
        ScreenIndex - the DisplayID for which the Windows are collected
        
    Global References:
        KWMTiling.WindowLst

    Mutations:
        (none)

    Return:
        std::vector<window_info*> - List of all Windows found on the Display

    Called Functions:
        display     :: GetDisplayFromScreenID()
        display     :: GetDisplayOfWindow()
        window      :: IsWindowFloating()
        application :: IsApplicationFloating()

    Calling Functions:
        serialize  :: FillDeserializedTree()
        space      :: TileFocusedSpace()
        windowtree :: UpdateWindowTree()
    
    Notes:
      - Doesn't collect floating windows, or windows from appications who are floating.
      - Doesn't walk through, just checks the position of each window in the global KWM Window$

    TODO: Remove the KWM Window$
    TODO: Refactor this to collect all Windows from the Display's Spaces
          -> Displays don't have windows, they have spaces which have windows. Abstraction!
*/
std::vector<window_info*> GetAllWindowsOnDisplay(int ScreenIndex);

/* Get all of the IDs for the managed Windows on a given Display

    Map:
        Display ~> Display

    Parameters:
        ScreenIndex - the ID of the Display for which the WindowIDs are collected
        
    Global References:
        KWMTiling.WindowLst

    Mutations:
        (none)

    Return:
        std::vector<int> - list of WindowIDs found on the given Display

    Called Functions:
        display::GetDisplayFromScreenID()
        application::IsApplicationFloating()

    Calling Functions:
        (none)
    
    Notes:
      - There's different behavior between this and the GetAllWindowsOnDisplay function
        -> this function doesn't filter out floating windows, whereas the other does

    TODO: Remove this function -- should just GetAllWindowIDsOnDisplay and then get their IDs if desired
          -> or leave this as a wrapper that does the above
*/
std::vector<int> GetAllWindowIDsOnDisplay(int ScreenIndex);

/* Add an Application and register it to a given Display

    Map:
        Display ~> Display

    Parameters:
        ScreenID - the Display to which the Application is captured
        Application - the Application to capture

    Global References:
    [M] KWMTiling.CapturedAppLst

    Mutations:
        KWMTiling.CapturedAppLst appended to Application, if doesn't already exist

    Return:
        (void)

    Called Functions:
        display :: GetDisplayFromScreenID()

    Calling Functions:
        interpreter :: KwmConfigCommand()
    
    Notes:
      - Only called by interpreter

    TODO: find out what this is used for
*/
void CaptureApplicationToScreen(int ScreenID, std::string Application);

/* Move the given Window to a Display

    Map:
        Display ~> Display

    Parameters:
        Window   - the Window to be moved
        Shift    - the index of the destination Display (see Relative)
        Relative - whether the Shift argument is relative to the current Display,
                   or an absolute DisplayID
        
    Global References:
        (none)

    Mutations:
        (see AddWindowToTreeOfUnfocusedMonitor)

    Return:
        (void)

    Called Functions:
        display    :: GetIndexOfNextScreen()
        display    :: GetIndexOfPrevScreen()
        display    :: GetDisplayFromScreenID()
    [M] windowtree :: AddWindowToTreeOfUnfocusedMonitor()

    Calling Functions:
        interpreter::KwmScreenCommand()
        application::CaptureApplication()
    
    Notes:
      - This function muxes into the 
      - Assumes the Window is already detached from its Tree

    TODO: Only supports relative movement by 1, can't shift 2 screens over
    TODO: Genericize the GetDisplay
    TODO: Current calling functions don't bother to remove/resize the given tree
*/
void MoveWindowToDisplay(window_info *Window, int Shift, bool Relative);

/* Set the Display's default Mode for Spaces

    Map:
        Display ~> Display

    Parameters:
        ScreenIndex - the ID of the Display for which the default mode is set
        Mode - the Mode to be set
        
    Global References:
    [M] KWMTiling.DisplayMode[ScreenIndex]

    Mutations:
        KWMTiling.DisplayMode[ScreenIndex] set to Mode

    Return:
        (void)

    Called Functions:
        (none)

    Calling Functions:
        interpreter :: KwmConfigCommand()
    
    Notes:
      - Only called from interpreter

    TODO: Can have a helper to convert from STRING to space_tiling_option
*/
void SetSpaceModeOfDisplay(unsigned int ScreenIndex, std::string Mode);

/* Get the Displays' default Mode for Spaces

    Map:
        Display ~> Display

    Parameters:
        ScreenIndex - the ID of the Display for which the Mode is retrieved
        
    Global References:
        KWMTiling.DisplayMode[ScreenIndex]

    Mutations:
        (none)

    Return:
        space_tiling_option - the default Mode for the Display

    Called Functions:
        (none)

    Calling Functions:
        windowtree::CreateWindowNodeTree()
    
    Notes:

    TODO: Encapsulate -- should only be called when creating new Spaces
*/
space_tiling_option GetSpaceModeOfDisplay(unsigned int ScreenIndex);

/* Move focus to a different Display

    Map:
        Display ~> Node

    Parameters:
        ScreenIndex - the DisplayID (kwm) of the Display to get Focus
        Focus - the Node to be focused, if any. If not provided, defaults to the left-most node
        Mouse - use the mouse for determining the Focus on the other Display?
        
    Global References:
    [M] KWMScreen.Current
    [M] KWMScreen.Current.PrevSpace
        KWMScreen.Current->ActiveSpace
        KWMFocus.Window

    Mutations:
        KWMScreen.Current.PrevSpace set to KWMScreen.Current->ActiveSpace
        KWMScreen.Current is set to Screen
        (fucktons)

    Return:
        (void)

    Called Functions:
        dispatcher :: ToggleFocusedWindowFloating()
        workspace  :: GetActiveSpaceOfDisplay()
        display    :: GetDisplayFromScreenID()
        space      :: GetActiveSpaceOfScreen()
        tree       :: GetFirstLeafNode()
        windowtree :: SetWindowFocusByNode()
        window     :: ClearFocusedWindow()
        window     :: FilterWindowList()
        window     :: FocusWindowBelowCursor()
        window     :: GetCursorPos()
        window     :: IsAnyWindowBelowCursor()
        window     :: IsWindowFloating()
        window     :: UpdateActiveWindowList()
        windowref  :: MoveCursorToCenterOfFocusedWindow()
        osx        :: CGEventCreateMouseEvent(NULL, kCGEventLeftMouseDown, .., kCGMouseButtonLeft);
        osx        :: CGEventCreateMouseEvent(NULL, kCGEventLeftMouseUp, .., kCGMouseButtonLeft);
        osx        :: CGEventPost(kCGHIDEventTap, ClickEvent);

    Calling Functions:
        interpreter :: KwmScreenCommand()
        dispatcher  :: FocusWindowByID()
        dispatcher  :: ShiftWindowFocus()
        dispatcher  :: ShiftWindowFocusDirected()
        display     :: RefreshActiveDisplays()
        display     :: UpdateActiveScreen()
    
    Notes:
      - Check if target Display is already the current  -- do nothing if so
      - Store the previous Display and set the given Display to KWMScreen.Current
      - Set the Display's Active Space (osx)
      - Check if space should be managed (??)
      - Get the Display's Active Space (kwm)
      - If Space is initialized and we're focusing a Node:
        -> UpdateActiveList(Screen)
        -> FilterWindowList(Screen)
        -> SetWindowFocusBynode(Node)
        -> MoveCursor to focused window
     - Else if Space is initialized and there's a Tree
        -> UpdateActiveList(Screen)
        -> FilterWindowList(Screen)
        -> If using the Mouse and there isn't a Window below the cursor
           -> ClearFocusedWindow
        -> Else If using the mouse and there IS a window below the cursor:
           -> FocusWindowBelowCursor
        -> Else if not using the mouse
           -> If space doesn't have a focused node, focus the first leaf node
            -> SetWindowFocusBynode(Node)
            -> MoveCursor to focused window

    TODO: Refactor the fuck out of this
*/
void GiveFocusToScreen(int ScreenIndex, tree_node *Focus, bool Mouse);

/* If mouse moved to different Display, update the Display

    Map:
        Display ~> Space

    Parameters:
        (none)

    Global References:
        KWMScreen.Current

    Mutations:
        (ActiveSpace)->Tree containers are applied
        (ActiveSpace->ForceContainerUpdate disabled after containers applied
        (see GiveFocusToScreen)
        (see ApplyNodeContainer)
        (see ClearMarkedWindow)

    Return:
        (void)

    Called Functions:
        display::GetDisplayOfMousePointer()
        display::GiveFocusToScreen()
        space::GetActiveSpaceOfScreen()
        tree::ApplyNodeContainer()
        window::ClearMarkedWindow()

    Calling Functions:
        kwm::CGEventCallback()
    
    Notes:
      - Only called as part of callback (mouse moved)

    TODO: Clean up what happens here and when it happens (does all of it need to happen?)
    TODO: Can we call some kind of "UpdateDisplay(X)" function that this wraps around?
*/
void UpdateActiveScreen();

/* Create default Offset for Display

    Map:
        (none)

    Parameters:
        (none)
        
    Global References:
        (none)

    Mutations:
        (none)

    Return:
        container_offset - the Offset to be used for the Display (default values)

    Called Functions:
        (none)

    Calling Functions:
        kwm::KwmInit()
    
    Notes:
      - Initialized during KwmInit to store in KWM.DefaultOffset

    TODO: Don't need this, just make a struct in types.h
*/
container_offset CreateDefaultScreenOffset();
padding_offset CreateDefaultPaddingOffset(); // TODO remove and replace with a struct

/* Create default Display object

    Map:
        Display ~> Display

    Parameters:
        DisplayIndex - the ID for the Display to be looked up in OSX
        ScreenIndex - the ID for the Display to be created in KWM

        
    Global References:
        KWMScreen.DefaultOffset

    Mutations:
        (none)

    Return:
        screen_info - the created Display

    Called Functions:
        osx :: CGDisplayBounds(DisplayIndex)

    Calling Functions:
        display::GetActiveDisplays()
        display::RefreshActiveDisplays()
    
    Notes:
      - CGDisplayBounds already gives us the dimensions rect struct we were looking to create
      - Keeping track of OS X DisplayID and KWM DisplayID separately.
        -> Why? Do they diverge?

    TODO: Do we need to return the object itself, or just a pointer?
*/ 
screen_info CreateDefaultScreenInfo(int DisplayIndex, int ScreenIndex);

/* Update the given Display's ID and dimensions

    Map:
        

    Parameters:
    [M] Screen - the Diplay to be updated
        DisplayIndex - the DisplayID (osx) of the Display to be updated
        Screenindex - the DisplayID (kwm) to set in the Display
        
    Global References:
        KWMScreen.DefaultOffset

    Mutations:
        Screen has its elements set in this function
          -> Screen->ID set to ScreenIndex
          -> Screen->X, Y, Width, Height set to dimensions from the DisplayID (osx)
          -> Screen->Offset reset to default offset
          -> Screen->ForceContainerUpdate set to true

    Return:
        (void)

    Called Functions:
        osx :: CGDisplayBounds(Displayindex)

    Calling Functions:
        display :: RefreshActiveDisplays()
    
    Notes:
        Can use the "dimensions" struct in this function

    TODO: Consolidate the Screen, DisplayID, and ScreenIndex
*/
void UpdateExistingScreenInfo(screen_info *Screen, int DisplayIndex, int ScreenIndex);

/* Set the Current Display's padding

    Map:
        Display ~> Display

    Parameters:
        Side - the padding to set ("left", "right", "top", "bottom")
        Offset - the new value of the padding
        
    Global References:
    [M] KWMScreen.DefaultOffset

    Mutations:
        One of the following is mutated (see Side)
          KWMScreen.DefaultPadding.PaddingLeft
          KWMScreen.DefaultPadding.PaddingRight
          KWMScreen.DefaultPadding.PaddingTop
          KWMScreen.DefaultPadding.PaddingBottom

    Return:
        (void)

    Called Functions:
        (none)

    Calling Functions:
        interpreter::KwmConfigCommand()
    
    Notes:
      - Only called by interpreter

    TODO: Rename the parameter to be more descriptive, like 'NewPadding'
    TODO: Genericize to operate on any given Display
    TODO: Move the "CurrentDisplay" function to the dispatcher
*/ 
void SetDefaultPaddingOfDisplay(const std::string &Side, int Offset);

/* Set the Current Display's Gap

    Map:
        Display ~> Display

    Parameters:
        Side - which gap to set ("vertical", "horizontal")
        Offset - the value of the padding
        
    Global References:
    [M] KWMScreen.DefaultOffset

    Mutations:
        One of the following is mutated (see Side)
          KWMScreen.DefaultOffset.VerticalGap
          KWMScreen.DefaultOffset.HorizontalGap

    Return:
        (void)

    Called Functions:
        (none)

    Calling Functions:
        interpreter :: KwmConfigCommand()
    
    Notes:

    TODO: Rename the parameter to be more descriptive, like 'NewPadding'
    TODO: Genericize to operate on any given Display
    TODO: Move the "CurrentDisplay" function to the dispatcher
*/
void SetDefaultGapOfDisplay(const std::string &Side, int Offset);

/* Adjust the Current Display's padding, and resize the Containers in the active Space's Tree

    Map:
        Display ~> Tree

    Parameters:
        Side - the side to which the padding should be adjusted ("left", "right", "top", "bottom")
        Offset - the amount to change the padding (pos or neg)
        
    Global References:
        (none)

    Mutations:
        (MouseScreen)->(ActiveSpace)->Offset.Padding<Side> is adjusted by Offset
        (MouseScreen)->(ActiveSpace)->Tree is resized and updated
        (see GetActiveSpaceOfScreen)
        (see ResizeTreeNodes)
        (see ApplyNodeContainer)
        (see SetRootNodeContainer)

    Return:
        (void)

    Called Functions:
        display   :: GetDisplayOfMousePointer()
    [M] space     :: GetActiveSpaceOfScreen()
        tree      :: ResizeTreeNodes()
        tree      :: ApplyNodeContainer()
        container :: SetRootNodeContainer()

    Calling Functions:
        interpreter::KwmSpaceCommand()

    
    Notes:
      - Doesn't set padding, just modifies it
      - Resizes the tree after doing the adjustment

    TODO: Add function which adjusts the padding (just calls SetPadding with the adjusted value)
    TODO: Add abstraction between Display / Space / Tree
    TODO: Add abstraction which operates on any given Display
    TODO: Add function wrapper to operate on focused display && updates the tree sizes
          -> UpdatePaddingOfCurrentDisplay(side, adjustment)
             -> ChangePaddingOfDisplay(current, side, adjustment)
                -> SetPaddingOfDisplay(D, <padding>)
             -> for S in S$:
                -> SetPaddingOfScreen(S, <padding>)
                -> ResizeTree(S->RootNode, S-><padding>)
*/
void ChangePaddingOfDisplay(const std::string &Side, int Offset);

/* Adjust the Current Display's container gaps, and resize the Containers in the active Space's Tree

    Map:
        Display ~> Tree

    Parameters:
        Side - the gap to be adjusted ("vertical", "horizontal")
        Offset - the amount to change the gap (pos or neg)
        
    Global References:
        (None)


    Mutations:
        (MouseScreen)->(ActiveSpace)->Offset.<Side>Gap is adjusted by Offset
        (MouseScreen)->(ActiveSpace)->Tree is resized and updated
        (see GetActiveSpaceOfScreen)
        (see ResizeTreeNodes)
        (see ApplyNodeContainer)

    Return:
        (void)

    Called Functions:
        display :: GetDisplayOfMousePointer()
        space   :: GetActiveSpaceOfScreen()
        tree    :: ResizeTreeNodes()
        tree    :: ApplyNodeContainer()

    Calling Functions:
        interpreter :: KwmSpaceCommand()
    
    Notes:
      - Resizes the tree after adjustment
    TODO: Consolidate logic with the ChangePadding function
    TODO: See ChangePaddingOfDisplay
*/
void ChangeGapOfDisplay(const std::string &Side, int Offset);

#endif
