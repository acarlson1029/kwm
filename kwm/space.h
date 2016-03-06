/* Notes
*  - Need to track the ID of the Space for use with OS X API calls
*/

#ifndef SPACE_H
#define SPACE_H

#include "types.h"

extern CFStringRef GetDisplayIdentifier(screen_info *Screen);

/* Get information on the number of Windows, and the Focused Window, if exists.

    Map:
        Space ~> Tree

    Parameters:
        Space - the Space to process
    [M] Tag -   stores the string representation of the Window status:
                Takes the form: [<NumberOfWindows>] or [<FocusWindowIndex>/<NumberOfWindows>]
                  ex. [4] or [1/4]
        
    Global References:
        KWMFocus.Window
        KWMFocus.Window->WID

    Mutations:
        Tag - store the resulting string in the input variable

    Return:
        (void)

    Called Functions:
        (none)

    Calling Functions:
        space :: GetTagForCurrentSpace(Space, Tag)
    
    Notes:
        Does tree processing on Space->RootNode

    TODO: Break out into calling Tree functions
*/
void GetTagForMonocleSpace(space_info *Space, std::string &Tag);

/* Get a Tag describing the Tree on the currently active Space

    Map:
        Space ~> Tree

    Parameters:
    [M] Tag - store the resulting tag describing the Space
        
    Global References:
        KWMScreen.Current

    Mutations:
        Tag - holds the resulting tag for the Space
                BSP Tree: "[bsp]"
                Floating: "[float]"
                Monocle:  "[<# of windows>]" or "[<focus index>/<# of windows>]" if focused Window on Space

    Return:
        (void)

    Called Functions:
        space :: IsSpaceInitializedForScreen(KWMScreen.Current)
        space :: GetActiveSpaceOfScreen(KWMScreen.Current)

    Calling Functions:
        interpreter :: KwmReadCommand(...)
    
    Notes:

    TODO: Genericize to get tag of ANY Space
*/
void GetTagForCurrentSpace(std::string &Tag);

/* Determine whether the Active Space exists in the Display's Space Cache, and is Initialized

    Map:
        Screen ~> Space

    Parameters:
        Screen - the Display whose cache is checked for the Active Space
        
    Global References:
        (none)

    Mutations:
        (none)

    Return:
        bool - true  : Space was found in cache and is Initialized
               false : Space was not found in cache
                        - OR - 
                       Space was found but is not Initialized

    Called Functions:
        (none)

    Calling Functions:
        serialize  :: SaveBSPTreeToFile(Screen, ..)
        serialize  :: LoadBSPTreeFromFile(Screen, ..)
        dispatcher :: FocusWindowByID(..)
        space      :: GetTagForCurrentSpace(KWMScreen.Current)
        space      :: IsSpaceFloating(KWMScreen.Current)
        windowtree :: AddWindowToTreeOfUnfocusedMonitor(Screen, ..)
    
    Notes:
      - Can probably split this into two functions:
        IsSpaceInDisplayCache(Display, Space)
        IsSpaceInitialized(Space)

    TODO: Move to Display file
*/
bool IsSpaceInitializedForScreen(screen_info *Screen);

/* Determine whether the Active Space exists in the Display's Space Cache, is Initialized, and contains a RootNode.

    Map:
        Display ~> Space

    Parameters:
        Screen - the Display whose Active Space and Space Cache is checked.

    Global References:
        (none)

    Mutations:
        (none)

    Return:
        bool - true  : Space was found in cache and is Initialized and has RootNode
               false : Space was not found in cache
                        - OR - 
                       Space was found but is not Initialized

    Called Functions:
        (none)

    Calling Functions:
        dispatcher  :: FocusFirstLeafNode()
        dispatcher  :: FocusLastLeafNode()
        dispatcher  :: ShiftWindowFocus()
        dispatcher  :: ShiftWindowFocusDirected()
        dispatcher  :: ToggleFocusedWindowParent()
        dispatcher  :: ToggleFocusedWindowFullscreen()
        dispatcher  :: SwapFocusedWindowWithMarked()
        dispatcher  :: SwapFocusedWindowDirected()
        dispatcher  :: SwapFocusedWindowWithNearest()
        dispatcher  :: ModifySubtreeSplitRatioFromWindow()
        interpreter :: KwmReadCommand()
        windowtree  :: AddWindowToBSPTree()
        windowtree  :: AddWindowToMonocleTree()
        windowtree  :: RemoveWindowFromBSPTree()
        windowtree  :: RemoveWindowFromMonocleTree()
        windowtree  :: ResizeElementInTree()
        windowref   :: IsWindowNonResizable()
    
    Notes:
        This is literally IsSpaceInitializedForScreen(Screen) && Space.RootNode != NULL
    TODO: Remove this function, replace with IsSpaceInitialized, genericize
*/
bool DoesSpaceExistInMapOfScreen(screen_info *Screen);

/* Get the Active Space from the Display's Space Cache. If ActiveSpace not in Space Cache, clear the entry.

    Map:
        Display ~> Screen

    Parameters:
    [M] Screen - the Display to get the active space of
        
    Global References:
        (none)

    Mutations:
        Screen->Space[Screen->ActiveSpace] cleared

    Return:
        space_info* - pointer to the Space

    Called Functions:
        (none)

    Calling Functions:
        serialize   :: CreateDeserializedNodeContainer()
        serialize   :: FillDeserializedTree()
        serialize   :: DeserializeChildNode()
        serialize   :: DeserializeNodeTree()
        serialize   :: SaveBSPTreeToFile()
        serialize   :: LoadBSPTreeFromFile()
        dispatcher  :: FocusFirstLeafNode()
        dispatcher  :: FocusLastLeafNode()
        dispatcher  :: FocusWindowByID()
        dispatcher  :: ShiftWindowFocus()
        dispatcher  :: ShiftWindowFocusDirected()
        dispatcher  :: ToggleFocusedWindowParent()
        dispatcher  :: ToggleFocusedWindowFullscreen()
        dispatcher  :: SwapFocusedWindowWithMarked()
        dispatcher  :: SwapFocusedWindowDirected()
        dispatcher  :: SwapFocusedWindowWithNearest()
        dispatcher  :: ModifySubtreeSplitRatioFromWindow()
        interpreter :: KwmReadCommand()
        interpreter :: KwmTreeCommand()
        display     :: ChangePaddingOfDisplay()
        display     :: ChangeGapOfDisplay()
        display     :: GiveFocusToScreen()
        display     :: UpdateActiveScreen()
        space       :: GetTagForCurrentSpace()
        space       :: IsActiveSpaceManaged()
        space       :: ShouldActiveSpaceBeManaged()
        space       :: FloatFocusedSpace()
        space       :: TileFocusedSpace()
        space       :: UpdateActiveSpace()
        windowtree  :: UpdateWindowTree()
        windowtree  :: CreateWindowNodeTree()
        windowtree  :: ShouldWindowNodeTreeUpdate()
        windowtree  :: AddWindowToBSPTree()
        windowtree  :: AddWindowToMonocleTree()
        windowtree  :: AddWindowToTreeOfUnfocusedMonitor()
        windowtree  :: RemoveWindowFromBSPTree()
        windowtree  :: RemoveWindowFromMonocleTree()
        windowtree  :: ResizeElementInTree()
        windowref   :: SetWindowRefFocus()
        windowref   :: IsWindowNonResizable()
          
    Notes:
        This is usually called right after querying the cache to see if Space exists.
    TODO: This is called fucking everywhere...
*/
space_info *GetActiveSpaceOfScreen(screen_info *Screen);

/* Query the Space->Managed element of the current Display's active Space

    Map:
        Display ~> Space

    Parameters:
        (none)
        
    Global References:
        KWMScreen.Current

    Mutations:
        (none)

    Return:
        bool - Space->Managed // TODO define this

    Called Functions:
        space :: GetActiveSpaceOfScreen(KWMScreen.Current)

    Calling Functions:
        kwm        :: KwmWindowMonitor()
        space      :: FloatFocusedSpace()
        space      :: TileFocusedSpace()
        window     :: FocusWindowOfOSX()
        window     :: FocusWindowBelowCursor()
        windowtree :: UpdateWindowTree()
    
    Notes:
        Gets the active space from the current display's cache and returns the .Managed element

    TODO: fix encapsulation
*/
bool IsActiveSpaceManaged();

/* Update the Space->Managed element of the current Display's active Space from OS X APIs

    Map:
        Display ~> Space

    Parameters:
        (none)
        
    Global References:
        KWMScreen.Current

    Mutations:
        KWMScreen.Current->(ActiveSpace)->Managed set

    Return:
        (void)

    Called Functions:
        osx :: CGSSpaceGetType(CGSDefaultConnection, KWMScreen.Current->ActiveSpace)

    Calling Functions:
        display :: GetActiveDisplays()
        display :: GiveFocusToScreen()
        space   :: UpdateActiveSpace()

    Notes:
      - Does OSX API call
      - Does mutation

    TODO: Refactor name to clarify the mutation, and that no bool is returned.
           - something like Set*
    TODO: Find out when/where/why this is needed.
*/
void ShouldActiveSpaceBeManaged();

/* Check if currently Display is transitioning between Spaces

    Map:
        Display ~> Display

    Parameters:
        (none)
        
    Global References:
    [M] KWMScreen.Transitioning
        KWMScreen.Current
    [M] KWMScreen.Current->Identifier

    Mutations:
        KWMScreen.Transitioning set if space transition in progress
        KWMScreen.Current->Identifier set if not currently set
        (see ClearFocusedWindow)
        (see ClearMarkedWindow)

    Return:
        bool - true  : transition in progress
               false : no transition in progress

    Called Functions:
        workspace :: GetDisplayIdentifier(KWMScreen.Current)
        osx       :: CGSManagedDisplayIsAnimating(CGSDefaultConnection, KWMScreen.Current->Identifier)
    [M] window    :: ClearFocusedWindow()
    [M] window    :: ClearMarkedWindow()

    Calling Functions:
        kwm        :: KwmWindowMonitor()
        space      :: FloatFocusedSpace()
        space      :: TileFocusedSpace()
        window     :: FocusWindowOfOSX()
        window     :: FocusWindowBelowCursor()
        windowtree :: UpdateWindowTree()
    
    Notes:
        Does lots of mutations under the hood

    TODO: Move to DISPLAY file
    TODO: Remove calls from window functions
    TODO: Refactor name into something descriptive like "UpdateTransitionStatus"
          - OR - have some callback on transition which will call functions to clear
                 the marked/focused windows through proper abstraction path and set
                 these values
                 have a separate function just for the query
*/
bool IsSpaceTransitionInProgress();

/* Check the SpaceMode of the given Space

    Map:
        Space ~> Space

    Parameters:
        SpaceID - the identifier of the Space in question
        
    Global References:
        KWMScreen.Current

    Mutations:
        (none)

    Return:
        bool - true  : Space of SpaceID is Floating

    Called Functions:
        space :: IsSpaceInitializedForScreen(KWMScreen.Current)

    Calling Functions:
        space      :: IsActiveSpaceFloating()
        space      :: ToggleFocusedSpaceFloating()
        windowtree :: CreateWindowNodeTree() // TODO shouldn't need to query, we should tell it the correct type of tree to create.

    Notes:
      - Walks through the KWMScreen.Current Space Cache

    TODO: Break into Display/Space for encapsulation:
          display :: IsSpaceOnDisplayFloating(D, S)
          space   :: IsSpaceFloating(S)
          space   :: SetSpaceFloating(S)
*/
bool IsSpaceFloating(int SpaceID);

/* Check the SpaceMode of the active Space

    Map:
        Space ~> Space

    Parameters:
        (none)
        
    Global References:
        KWMScreen.Current->ActiveSpace

    Mutations:

    Return:
        bool - true if Space is floating
        
    Called Functions:
        space :: IsSpaceFloating(KWMScreen.Current->ActiveSpace)

    Calling Functions:
        kwm        :: CGEventCallback()
        windowtree :: UpdateWindowTree()
        windowref  :: SetWindowRefFocus()
    
    Notes:
        Just calls IsSpaceFloating on the active space

    TODO: Encapsulate away from calling functions.
*/
bool IsActiveSpaceFloating();

/* Create a Tiling Window Tree for the Windows on the current Display's active Screen

    Map:
        Display ~> Screen

    Parameters:
        Mode - the tiling mode to use for the Space
        
    Global References:
        KWMScreen.Current
        KWMToggles.EnableTilingMode

    Mutations:
        Space->RootNode reset
        Space->Mode set
        (see IsSpaceTransitionInProgress)
        (see GetActiveSpaceOfScreen)
        (see FilterWindowList)
        (see DestroyNodeTree)
        (see CreateWindowNodeTree)

    Return:
        (void)

    Called Functions:
        display :: GetAllWindowsOnDisplay(KWMScreen.Current->ID)
    [M] space   :: IsSpaceTransitionInProgress()
        space   :: IsActiveSpaceManaged()
    [M] space   :: GetActiveSpaceOfScreen(KWMScreen.Current)
    [M] tree    :: DestroyNodeTree(...)
    [M] tree    :: CreateWindowNodeTree(KWMScreen.Current)
    [M] window  :: FilterWindowList(KWMScreen.Current)

    Calling Functions:
        interpreter :: KwmSpaceCommand()
        space       :: ToggleFocusedSpaceFloating()

    Notes:
      - If we aren't in a tiling space, collect windows, filter by Space, destroy the existing 
        tree, and create a tiling tree.

    TODO: Genercize to Tile any given Space
    TODO: Break into Display/Space encapsulations
    TODO: Add a function in the dispatcher to the interpreter doesn't call explicitly.
    TODO: GetAllWindows takes a Screen and gets the Space again, even though we have it in the calling function
*/
void TileFocusedSpace(space_tiling_option Mode);

/* Set the current Display's active Space to floating, destroying the tree.

    Map:
        Display ~> Space

    Parameters:
        (none)
        
    Global References:
        KWMToggles.EnableTilingMode
        KWMScreen.Current

    Mutations:
        (ActiveSpace)->RootNode = NULL
        (ActiveSpace)->Mode = SpaceModeFloating
        (ActiveSpace)->Initialized = true
        (see IsSpaceTransitionInProgress)
        (see GetActiveSpaceOfScreen)
        (see ClearFocusedWindow)

    Return:
        (void)

    Called Functions:
    [M] space  :: IsSpaceTransitionInProgress()
        space  :: IsActiveSpaceManaged()
    [M] space  :: GetActiveSpaceOfScreen(KWMScreen.Current)
    [M] tree   :: DestroyNodeTree()
    [M] window :: ClearFocusedWindow()

    Calling Functions:
        interpreter :: KwmSpaceCommand()
        space       :: ToggleFocusedSpaceFloating()
    
    Notes:
      - Function qualifications have the same signature as TileFocusedSpace()
        -> Might be able to consolidate logic

    TODO: Get a function call in the Dispatcher for this command
*/
void FloatFocusedSpace();

/* If the current Display's active Space is Tiling, switch to Float, else Tile it.

    Map:
        Display ~> Space

    Parameters:
        (none)
        
    Global References:
        KWMScreen.Current->ActiveSpace

    Mutations:
        (see FloatFocusedSpace)
        (see TileFocusedSpace)

    Return:
        (void)

    Called Functions:
        space :: IsSpaceFloating(KWMScreen.Current->ActiveSpace)
    [M] space :: FloatFocusedSpace()
    [M] space :: TileFocusedSpace()

    Calling Functions:
        interpreter::KwmSpaceCommand()
    
    Notes:
      - This is just a wrapper/mux for the two called focus functions

    TODO: Move this to the dispatcher
    TODO: Genericize for any space
    TODO: Genericize for any display
*/
void ToggleFocusedSpaceFloating();

/* Refresh the Focus of the current Display's Active Space

    Map:
        Display ~> Space

    Parameters:
        (none)
        
    Global References:
        KWMThread.Lock
        KWMScreen.Current
    [M] KWMScreen.Current->ActiveSpace
    [M] KWMScreen.Transitioning
    [M] KWMScreen.PrevSpace
    [M] KWMScreen.ForceRefreshFocus
        KWMMode.Focus

    Mutations:
        KWMScreen.Current->ActiveSpace set to GetActiveSpaceOfDisplay
        KWMScreen.Transitioning set to false
        KWMScreen.PrevSpace set to old KWMScreen.Current->ActiveSpace
        KWMScrenForceRefreshFocus is set to true, then false
        (see ShouldActiveSpaceBeManaged)
        (see UpdateActiveWindowList)
        (see GetActiveSpaceOfScreen)
        (see SetWindowFocusByNode)
        (see MoveCursorToCenterOfFocusedWindow)

    Return:
        (void)

    Called Functions:
        sys        :: pthread_mutex_lock
        sys        :: pthread_mutex_unlock
        workspace  :: GetActiveSpaceOfDisplay(KWMScreen.Current)
    [M] space      :: ShouldActiveSpaceBeManaged()
    [M] space      :: UpdateActiveWindowList(KWMScreen.Current)
    [M] space      :: GetActiveSpaceOfScreen(KWMScreen.Current)
    [M] windowtree :: SetWindowFocusByNode()
        window     :: IsAnyWindowBelowCursor()
        window     :: FocusWindowBelowCursor()
        window     :: FocusWindowofOSX()
    [M] windowref  :: MoveCursorToCenterOfFocusedWindow()

    Calling Functions:
        workspace :: activeSpaceDidChange
    
    Notes:

    TODO: Genericize for display / space
    TODO: Encapsulate
    TODO: gut this pig
*/
void UpdateActiveSpace();

#endif
