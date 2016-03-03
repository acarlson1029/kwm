/* Map Window arguments onto the Tree structure. */

#ifndef WINDOWTREE_H
#define WINDOWTREE_H

#include "types.h"
/* Focus */


/* Call SetWindowFocus on the Node's Window

    Map:
        Node->WindowID ~> SetWindowFocus

	Parameters:
       Node - the node whose Window will be focused.

    Global References:
    [M]  KWMScreen.ForceRefreshFocus

    Mutations:
        KWMScreen.ForceRefreshFocus set to true, then false.

	Return:
		(void)

	Called Functions:
        GetWindowByID(Node->WindowID)
        windowref::SetWindowFocus(...)

	Calling Functions:
        display::GiveFocusToScreen(.., FocusNode, ..)
        space::UpdateActiveSpace()
        FocusFirstLeafNode()
        FocusLastLeafNode()
        ShiftWindowFocus(..)
        RemoveWindowFromBSPTree(...)
        RemoveWindowFromMonocleTree(...)
*/
void SetWindowFocusByNode(tree_node *Node); // whereshould this go? it just calls SetWindowFocus(Window)

/* Set Focus to Window in first Leaf

    Map:
        KWMScreen.Current ~> SetWindowFocusByNode

	Parameters:
       (none)

    Global References:
        KWMScreen.Current

	Return:
		(void)

	Called Functions:
        DoesSpaceExistInMapOfScreen(KWMScreen.Current))
        GetActiveSpaceOfScreen(KWMScreen.Current)
        GetFirstLeafNode(...)
        SetWindowFocusByNode(...)
        MoveCursorToCenterOfFocusedWindow();

	Calling Functions:
        interpreter::KwmWindowCommand

    Notes:
		Only used by the interpreter.
		Calls MoveCursor afterward.
		Related to ShiftWindowFocus()
*/
void FocusFirstLeafNode();

/* Set Focus to Window in last Leaf

    Map:
        KWMScreen.Current ~> SetWindowFocusByNode

	Parameters:
       (none)

    Global References:
        KWMScreen.Current

	Return:
		(void)

	Called Functions:
        DoesSpaceExistInMapOfScreen(KWMScreen.Current))
        GetActiveSpaceOfScreen(KWMScreen.Current)
        GetLastLeafNode(...)
        SetWindowFocusByNode(...)
        MoveCursorToCenterOfFocusedWindow();

	Calling Functions:
        interpreter::KwmWindowCommand

    Notes:
		Only used by the interpreter.
		Calls MoveCursor afterward.
		Related to ShiftWindowFocus()
*/
void FocusLastLeafNode();

/* Set Focus to WindowID; if on a different Screen, change focus of Screen

    Map:
        WindowID -> SetWindowFocus; GiveFocusToScreen

	Parameters:
        WindowID - The WID of the window to focus

    Global References:
        KWMScreen.Current

	Return:
		(void)

	Called Functions:
        GetWindowByID(WindowID)
        GetDisplayOfWindow(..)
        SetWindowFocus(..)
        MoveCursorToCenterOfFocusedWindow()
        IsSpaceInitializedForScreen(...)
        GiveFocusToScreen(...)

	Calling Functions:
        interpreter::KwmSpaceCommand(..)
    
    Notes:
        Only called from interpreter.
*/
void FocusWindowByID(int WindowID);

/* Focus the next/previous window

    Map:
        Shift +1 / -1 ~> SetWindowFocusByNode(right or left node)

	Parameters:
       Shift - +1 : Shift focus Right
               -1 : Shift focus Left

    Global References:
        KWMFocus.Window
        KWMScreen.Current
        KWMMode.Cycle

	Return:
	    (void)

	Called Functions:
        space::DoesSpaceExistInMapOfScreen(KWMScreen.Current)
        space::GetActiveSpaceOfScreen(KWMScreen.Current)
        tree::GetNodeFromWindowID(.., KWMFocus.Window->WID, ..);
        tree::GetNearestNodeToTheRight(...)
        tree::GetNearestNodeToTheLeft(...)
        tree::GetFirstLeafNode(..)
        display::GetIndexOfNextScreen()
        display::GetDisplayFromScreenID(..)
        tree::GetFirstLeafNode(..)
        display::GiveFocusToScreen(..., false)
        SetWindowFocusByNode(..)
        MoveCursorToCenterOfFocusedWindow()

	Calling Functions:
        interpreter::KwmSpaceCommand(..)
    
    Notes:
	  - Can combine the two Shift blocks -- they just determine which node to get (nearest L/R)
	  - Only called from interpreter.
*/
void ShiftWindowFocus(int Shift);

/* Move Focus to the North, East, South, or West Window.

    Map:
        Degrees ~> Choose Window nearest North/East/South/West

	Parameters:
       Degrees - rotate focus North = 0, East = 90, South = 180, West = 270

    Global References:
        KWMFocus.Window
        KWMScreen.Current
        KWMScreen.Current->ID
        KWMMode.Cycle

	Return:
		(void)

	Called Functions:
        space::DoesSpaceExistInMapOfScreen(KWMScreen.Current)
        space::GetActiveSpaceOfScreen(KWMScreen.Current)
        window::FindClosestWindow(Degrees, ...)
        window::WindowIsInDirection(KWMFocus.Window, ..., false)
        windowref::SetWindowFocus(..)
        windowref::MoveCursorToCenterOfFocusedWindow()
        display::GetIndexOfNextScreen()
        display::GetIndexOfPrevScreen()
        display::GetDisplayFromScreenID(..)
        display::GiveFocusToScreen(..., false)
        tree::GetFirstLeafNode(..)
        tree::GetLastLeafNode(..)
        ShiftWindowFocus(..)

	Calling Functions:
        interpreter::KwmSpaceCommand
    
    Notes:
		Only called from interpreter
	
	TODO:
	    Refactor into existing functions.
*/
void ShiftWindowFocusDirected(int Degrees);

/* Global Update function from KwmWindowMonitor

	Parameters:
       (none)

    Global References:
        KWMScreen.Current
        KWMToggles.EnableTilingMode

    Mutations:


	Return:
		(void)

	Called Functions:
        display::GetAllWindowsOnDisplay(KWMScreen.Current->ID)
        space::IsSpaceTransitionInProgress()
        space::IsActiveSpaceManaged()
        space::GetActiveSpaceOfScreen(KWMScreen.Current)
        space::IsActiveSpaceFloating()
        tree::DestroyNodeTree(...)
        windowtree::CreateWindowNodeTree(KWMScreen.Current, ..)
        windowtree::ShouldWindowNodeTreeUpdate(KWMScreen.Current)
        window::UpdateActiveWindowList(KWMScreen.Current)
        window::FilterWindowList(KWMScreen.Current)
        window::ClearFocusedWindow()

	Calling Functions:
        kwm:: KwmWindowMonitor
    
    Notes:
		Part of constant polling
*/
void UpdateWindowTree();

/* Create a Tree from a list of WindowIDs, resize the containers, resize the windows, and focus the window below the cursor.

    Map:
        Screen, Windows -> Tree

	Parameters:
       Screen - the current Display
       Windows - list of Windows use when creating the Tree

    Global References:
        KWMMode.Space

    Mutations:
        ActiveSpace(Screen)->Mode
        ActiveSpace(Screen)->Initialized = true
        ActiveSpace(Screen)->RootNode
        (creates tree from RootNode)

	Return:
		(void)

	Called Functions:
        display::GetDisplayOfWindow(..)
        space::GetActiveSpaceOfScreen(Screen)
        space::IsSpaceFloating()
        tree::CreateTreeFromWindowIDList(Screen, .., *Windows, ..)
        tree::ResizeTreeNodes(Screen, ...)
        tree::ApplyNodeContainer(...)
        window::FocusWindowBelowCursor()

	Calling Functions:
        space::TileFocusedSpace(..)
        UpdateWindowTree()
*/
void CreateWindowNodeTree(screen_info *Screen, std::vector<window_info*> *Windows);

/* Wrapper around Should[BSP|Monocle]TreeUpdate()

    Map:
        Screen ~> Should[BSP|Monocle]TreeUpdate(Screen, ..)

	Parameters:
       Screen

	Return:
		(void)

	Called Functions:
        ShouldBSPTreeUpdate(Screen, ..)
        ShouldMonocleTreeUpdate(Screen, ..)

	Calling Functions:
        UpdateWindowTree()

    Notes:
        Functions should call this instead of the wrapped functions directly.
*/
void ShouldWindowNodeTreeUpdate(screen_info *Screen);

/* Check if the number of Windows has changed, and if so, modify the BSP Tree accordingly.

    Map:
        Screen, Space ~> Add/Remove Window(s) in BSP Tree

	Parameters:
       Screen - the current Display
       Space - the current Space

    Global References:
        KWMTiling.WindowLst - check the number of Windows versus the Screen->OldWindowListCount
        KWMFocus.Window - not set ? clear focused window and set a new focus
        KWMMode.Focus - is FocusModeDisabled ?

    Mutations:
        Space->RootNode

	Return:
		(void)

	Called Functions:
        tree::GetNodeFromWindowID(Space->RootNode, KWMTilingWindowLst[<idx>].WID, Space->Mode)
        tree::GetFirstPseudoLeafNode(Space->RootNode)
        tree::ApplyNodeContainer(.., SpaceModeBSP)
        tree::GetFirstLeafNode(Space->RootNode)
        tree::GetNearestNodeToTheRight(.., SpaceModeBSP)
        windowtree::AddWindowToBSPTree(Screen, KWMTiling.WindowLst[<idx>].WID)
        windowtree::RemoveWindowFromBSPTree(Screen, .., true)
        windowtree::IsApplicationFloating(KWMTiling.WindowLst[<idx>)
        window::IsWindowFloating(KWMTiling.WindowLst[<idx].WID, NULL)
        window::ClearFocusedWindow()
        window::IsAnyWindowBelowCursor()
        window::FocusWindowBelowCursor()
        window::FocusWindowOfOSX()
        windowref::SetWindowFocus(KWMTiling.WindowLst[<idx>]
        windowref::MoveCursorToCenterOfFocusedWindow()

	Calling Functions:
        ShouldWindowNodeTreeUpdate(Screen)
    
    Notes:
	  - Only called by ShouldWindowNodeTreeUpdate()
      - Should only pass Space->RootNode into this function
        `-> Remove Space->Mode -- we're in a BSP Tree, so hardcode it.
      - Should just check if the current list of windows is the same as the old list
        `-> maybe one was added and one was removed
*/
void ShouldBSPTreeUpdate(screen_info *Screen, space_info *Space);

/* Check if the number of Windows has changed, and if so, modify the Monocle tree accordingly.

    Map:
        Screen, Space ~> Add/Remove Window(s) in Monocle Tree

	Parameters:
       Screen - the current Display
       Space - the current Space

    Global References:
        KWMTiling.WindowLst - check the number of Windows versus the Screen->OldWindowListCount

    Mutations:
        Space->RootNode

	Return:
		(void)

	Called Functions:
        tree::GetNodeFromWindowID(Space->RootNode, KWMTilingWindowLst[<idx>].WID, Space->Mode) // TODO replace Space->Mode with SpaceModeMonocle
        tree::GetNearestNodeToTheRight(.., SpaceModeMonocle)
        windowtree::IsApplicationFloating(KWMTiling.WindowLst[<idx>)
        windowtree:AddWindowToMonocleTree(Screen, KWMTiling.WindowLst[<idx>].WID)
        windowtree::RemoveWindowFromMonocleTree(Screen, ..);
        windowref::SetWindowFocus(KWMTiling.WindowLst[<idx>]
        windowref::MoveCursorToCenterOfFocusedWindow()

	Calling Functions:
        ShouldWindowNodeTreeUpdate
    
    Notes:
	  - Only called by ShouldWindowNodeTreeUpdate
	  - Should only pass Space->RootNode to this function
      - Should just check if the current list of windows is the same as the old list
        `-> maybe one was added and one was removed
    TODO: Extract code that is common between ShouldMonocleTreeUpdate and ShouldBSPTreeUpdate
*/
void ShouldMonocleTreeUpdate(screen_info *Screen, space_info *Space);

/* Add/Remove windows */

/* Add a WindowID to the BSP Tree
   Finds the most relevant Node and inserts the WindowID next to it.

    Map:
        Screen, WindowID -> BSP Tree (add)

	Parameters:
        Screen - the current display
        WindowID - the WID of the Window to add

    Global References:
        KWMFocus.Window->WID - check whether to use focused container
        KWMScreen.MarkedWindow - TODO 
        KWMScreen.SplitMode - SplitMode for adding the Element to the Tree
 
    Mutations:
        (see AddElementToTree())

	Return:
		(void)

	Called Functions:
        space::DoesSpaceExistInMapOfScreen(Screen)
        space::GetActiveSpaceOfScreen(Screen)
        tree::GetNodeFromWindowID(.., KWMFocus.Window->WID, Space->Mode)
        tree::GetNodeFromWindowID(.., KWMScreen.MarkedWindow, Space->Mode)
        tree::LevelOrderSearch(...)
        tree::AddElementToTree(Screen, .., .., WindowID, KWMScreen.SplitMode, SpaceModeBSP);
        window::IsWindowOnActiveSpace(KWMFocus.Window->WID)
        window::IsWindowFloating(KWMScreen.MarkedWindow)
        window::ClearMarkedWindow()

	Calling Functions:
        ShouldBSPTreeUpdate(Screen, ..)  -- note: has Space argumnent which this function looks up
        AddWindowToBSPTree()
        ToggleWindowFloating(WindowID)
    
    Notes:
		Should Node picking logic for AddElementToTree happen in the Tree functions?
		Now it requires a bit of work on calling functions to search the tree for
		the right place to put it.
		Currently uses FocusedWindow, or MarkedWindow, or first Node (LOSearch)
		`-> could probably make this a parameter?
*/
void AddWindowToBSPTree(screen_info *Screen, int WindowID);

/* UNUSED FUNCTION */
void AddWindowToBSPTree();

/* Add a WindowID to the Monocle Tree

    Map:
        Screen, WindowID -> Monocle Tree (add)

	Parameters:
        Screen - the Display to add the WindowID to
        WindwoID - the WindowID to add to the tree

    Mutations:
        (see AddElementToTree)

	Return:
		(void)

	Called Functions:
        space::DoesSpaceExistInMapOfScreen(Screen)
        space::GetActiveSpaceOfScreen(Screen)
        tree::GetLastLeafNode(Space->RootNode)
        tree::AddElementToTree(Screen, Space->Offset, .., WindowID, SplitModeUnset, SpaceModeMonocle)

	Calling Functions:
        ShouldMonocleTreeUpdate(Screen, ..)
    
    Notes:
		Calling function has Space info, can we just pass the Space directly?
*/
void AddWindowToMonocleTree(screen_info *Screen, int WindowID);

/* Add a WindowID to the Tree on an Unfocused Display

    Map:
        Screen, WindowID -> Tree (add)

	Parameters:
    [M] Screen - the display to add the Window to
    [M] Window - the Window to add to the Tree

    Global References:
        KWMScreen.MarkedWindow -- check if Window is marked -> clear marked window
        KWMScreen.SplitMode -- for calling AddElementToTree

    Mutations:
        (see CenterWindow)
        Screen->ForceContainerUpdate = true

	Return:
		(void)

	Called Functions:
        display::GetDisplayOfWindow(Window)
        space::IsSpaceInitializedForScreen(Screen)
        space::GetActiveSpaceOfScreen(Screen)
        tree::LevelOrderSearch(...)
        tree::GetLastLeafNode(..)
        tree::AddElementToTree(Screen, .., .., .., Window->WID, KWMScreen.SplitMode, ..)
        window::ClearMarkedWindow()
        windowref::CenterWindow(Screen, Window)

	Calling Functions:
        display::MoveWindowToDisplay(Window, ...)
    
    Notes:
		Why can't this function just call one of the other AddWindowTo*Tree() functions?
		`-> they have active space guards, but they could be moved to a higher-level wrapper function instead.
*/
void AddWindowToTreeOfUnfocusedMonitor(screen_info *Screen, window_info *Window);

/* Given a WindowID, remove it from the Screen's active Space's Tree (and refresh)

    Map:
        Screen, WindowID -> Tree

	Parameters:
        Screen - the current Display
        WindowID - the WID of the Window to remove.
        Refresh - whether to reset the focus/cursor

    Mutations:
        (see RemoveElementFromTree)

	Return:
		(void)

	Called Functions:
        space::DoesSpaceExistInMapOfScreen(Screen)
        space::GetActiveSpaceOfScreen(Screen)
        tree::RemoveElementFromTree(Screen, .., .., WindowID, ..)
        tree::GetFirstLeafNode(..)
        windowtree::SetWindowFocusByNode(..)
        windowref::MoveCursorToCenterOfFocusedWindow()

	Calling Functions:
        windowref::IsWindowNonResizable(.., Window, ...)
        windowref::ShouldBSPTreeUpdate(Screen, ...)
        RemoveWindowFromBSPTree()
    
    Notes:
		All calling functions have Space info, so maybe pass the Space directly? Or Root Node?
*/
void RemoveWindowFromBSPTree(screen_info *Screen, int WindowID, bool Refresh);

/* UNUSED FUNCTION */
void RemoveWindowFromBSPTree();

/* Given a WindowID, remove it from the Screen's active Space's Tree and update the focus.

    Map:
        Screen, WindowID ~> Tree

	Parameters:
        Screen - the current Display
        WindowID - the WID of the Window to remove.

    Mutations:
        (see RemoveElementFromTree)

	Return:
		(void)

	Called Functions:
        display::DoesSpaceExistInMapOfScreen(Screen)
        space::GetActiveSpaceOfScreen(Screen)
        tree::RemoveElementFromTree(Screen, .., .., WindowID, ..)
        windowtree::SetWindowFocusByNode(..)
        windowref::MoveCursorToCenterOfFocusedWindow()

	Calling Functions:
        windowref::IsWindowNonResizable
        ShouldMonocleTreeUpdate(Screen, ..)
    
    Notes:
		Can pass Space/RootNode directly to this function.
*/
void RemoveWindowFromMonocleTree(screen_info *Screen, int WindowID);

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


/* Resize the Element in the Tree.

    Map:
        WindowID ~> Node

	Parameters:
       Screen - the current Display
       Window - the Window to resize

    Mutations:
        (see ResizeElementInNode)

	Return:
		(void)

	Called Functions:
        space::DoesSpaceExistInMapOfScreen(Screen)
        space::GetActiveSpaceOfScreen(Screen)
        tree::GetNodeFromWindowID(.., Window->WID, ..)
        node::ResizeElementInNode(..)

	Calling Functions:
	    ResizeWindowToContainerSize()
    
    Notes:
		Should this be moved to tree.h?
*/
void ResizeElementInTree(screen_info *Screen, window_info *Window);

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

