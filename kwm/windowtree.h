/* Map Window arguments onto the Tree structure. */

#ifndef WINDOWTREE_H
#define WINDOWTREE_H

#include "types.h"

/* Call SetWindowFocus on the Node's Window

    Map:
        Node ~> WindowRef

    Parameters:
        Node - the node whose Window will be focused.

    Global References:
    [M] KWMScreen.ForceRefreshFocus

    Mutations:
        KWMScreen.ForceRefreshFocus set to true, then false.
        (see SetWindowFocus)

    Return:
        (void)

    Called Functions:
        window    :: GetWindowByID(Node->WindowID)
    [M] windowref :: SetWindowFocus(...)

    Calling Functions:
        dispatcher :: FocusFirstLeafNode()
        dispatcher :: FocusLastLeafNode()
        display    :: GiveFocusToScreen(.., FocusNode, ..)
        space      :: UpdateActiveSpace()
        dispatcher :: ShiftWindowFocus(..)
        windowtree :: RemoveWindowFromBSPTree(...)
        windowtree :: RemoveWindowFromMonocleTree(...)

    TODO: Add Window abstraction between Node~>WindowRef
*/
void SetWindowFocusByNode(tree_node *Node);

/* Global Update function from KwmWindowMonitor

    Map:
        Display ~> Tree

    Parameters:
       (none)

    Global References:
        KWMScreen.Current
        KWMToggles.EnableTilingMode

    Mutations:
        (see DestroyNodeTree)
        (see CreateWindowNodeTree)
        (see ShouldWindowNodeTreeUpdate)
        (see GetActiveSpaceOfScreen)
        (see UpdateActiveWindowList)
        (see FilterWindowList)
        (see ClearFocusedWindow)

    Return:
        (void)

    Called Functions:
        display    :: GetAllWindowsOnDisplay(KWMScreen.Current->ID)
        space      :: IsSpaceTransitionInProgress()
        space      :: IsActiveSpaceManaged()
    [M] space      :: GetActiveSpaceOfScreen(KWMScreen.Current)
        space      :: IsActiveSpaceFloating()
    [M] tree       :: DestroyNodeTree(...)
    [M] windowtree :: CreateWindowNodeTree(KWMScreen.Current, ..)
    [M] windowtree :: ShouldWindowNodeTreeUpdate(KWMScreen.Current)
    [M] window     :: UpdateActiveWindowList(KWMScreen.Current)
    [M] window     :: FilterWindowList(KWMScreen.Current)
    [M] window     :: ClearFocusedWindow()

    Calling Functions:
        kwm :: KwmWindowMonitor

    Notes:
        Part of constant polling
*/
void UpdateWindowTree();

/* Create a Tree from a list of WindowIDs, resize the containers, resize the windows, and focus the window below the cursor.

    Map:
        Display ~> Tree

    Parameters:
       Screen - the current Display
       Windows - list of Windows use when creating the Tree

    Global References:
        KWMMode.Space

    Mutations:
        ActiveSpace(Screen)->Mode
        ActiveSpace(Screen)->Initialized = true
        ActiveSpace(Screen)->RootNode
        (see GetActiveSpaceOfScreen)
        (see CreateTreeFromWindowIDList)
        (see ResizeTreeNodes)
        (see ApplyNodeContainer)
        (see FocusWindowBelowCursor)

    Return:
        (void)

    Called Functions:
        display :: GetDisplayOfWindow(..)
    [M] space   :: GetActiveSpaceOfScreen(Screen)
        space   :: IsSpaceFloating()
    [M] tree    :: CreateTreeFromWindowIDList(Screen, .., *Windows, ..)
    [M] tree    :: ResizeTreeNodes(Screen, ...)
    [M] tree    :: ApplyNodeContainer(...)
    [M] window  :: FocusWindowBelowCursor()

    Calling Functions:
        space      :: TileFocusedSpace(..)
        windowtree :: UpdateWindowTree()
*/
void CreateWindowNodeTree(screen_info *Screen, std::vector<window_info*> *Windows);

/* Check if the number of Windows has changed, and if so, modify the BSP Tree accordingly.

    Map:
        Display, Space ~> Tree

    Parameters:
        Screen - the current Display
    [M] Space - the current Space

    Global References:
        KWMTiling.WindowLst - check the number of Windows versus the Screen->OldWindowListCount
        KWMFocus.Window - not set ? clear focused window and set a new focus
        KWMMode.Focus - is FocusModeDisabled ?

    Mutations:
        Space->RootNode tree is mutated.
        (see ApplyNodeContainer)
        (see AddWindowToBSPTree)
        (see RemoveWindowFromBSPTree)
        (see ClearFocusedWindow)
        (see FocusWindowBelowCursor)
        (see FocusWindowOfOSX)
        (see SetWindowFocus)
        (see MoveCursorToCenterOfFocusedWindow)

    Return:
        (void)

    Called Functions:
        tree        :: GetNodeFromWindowID(Space->RootNode, KWMTilingWindowLst[<idx>].WID, Space->Mode)
        tree        :: GetFirstPseudoLeafNode(Space->RootNode)
    [M] tree        :: ApplyNodeContainer(.., SpaceModeBSP)
        tree        :: GetFirstLeafNode(Space->RootNode)
        tree        :: GetNearestNodeToTheRight(.., SpaceModeBSP)
    [M] windowtree  :: AddWindowToBSPTree(Screen, KWMTiling.WindowLst[<idx>].WID)
    [M] windowtree  :: RemoveWindowFromBSPTree(Screen, .., true)
        window      :: IsWindowFloating(KWMTiling.WindowLst[<idx].WID, NULL)
    [M] window      :: ClearFocusedWindow()
        window      :: IsAnyWindowBelowCursor()
    [M] window      :: FocusWindowBelowCursor()
    [M] window      :: FocusWindowOfOSX()
    [M] windowref   :: SetWindowFocus(KWMTiling.WindowLst[<idx>]
    [M] windowref   :: MoveCursorToCenterOfFocusedWindow()
        application :: IsApplicationFloating(KWMTiling.WindowLst[<idx>)

    Calling Functions:
        windowtree :: ShouldWindowNodeTreeUpdate(Screen)

    Notes:
      - Only called by ShouldWindowNodeTreeUpdate()
      - Should only pass Space->RootNode into this function
        `-> Remove Space->Mode -- we're in a BSP Tree, so hardcode it.
      - Should just check if the current list of windows is the same as the old list
        `-> maybe one was added AND one was removed
*/
void ShouldBSPTreeUpdate(screen_info *Screen, space_info *Space);

/* Check if the number of Windows has changed, and if so, modify the Monocle tree accordingly.

    Map:
        Display, Space ~> Tree

    Parameters:
        Screen - the current Display
    [M] Space - the current Space

    Global References:
        KWMTiling.WindowLst - check the number of Windows versus the Screen->OldWindowListCount

    Mutations:
        Space->RootNode is set/unset to Tree

    Return:
        (void)

    Called Functions:
        tree       :: GetNodeFromWindowID(Space->RootNode, KWMTilingWindowLst[<idx>].WID, Space->Mode) // TODO replace Space->Mode with SpaceModeMonocle
        tree       :: GetNearestNodeToTheRight(.., SpaceModeMonocle)
        windowtree :: IsApplicationFloating(KWMTiling.WindowLst[<idx>)
        windowtree :: AddWindowToMonocleTree(Screen, KWMTiling.WindowLst[<idx>].WID)
        windowtree :: RemoveWindowFromMonocleTree(Screen, ..);
        windowref  :: SetWindowFocus(KWMTiling.WindowLst[<idx>]
        windowref  :: MoveCursorToCenterOfFocusedWindow()

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

/* Wrapper around Should[BSP|Monocle]TreeUpdate()

    Map:
        Display ~> Should[BSP|Monocle]TreeUpdate(Screen, ..)

    Parameters:
       Screen

    Return:
        (void)

    Called Functions:
        windowtree::ShouldBSPTreeUpdate(Screen, ..)
        windowtree::ShouldMonocleTreeUpdate(Screen, ..)

    Calling Functions:
        windowtree::UpdateWindowTree()

    Notes:
        Functions should call this instead of the wrapped functions directly.
*/
void ShouldWindowNodeTreeUpdate(screen_info *Screen);

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

#endif

