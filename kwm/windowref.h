#ifndef WINDOWREF_H
#define WINDOWREF_H

#include "types.h"

/* AXUIElementRef documentation:
	https://developer.apple.com/library/mac/documentation/ApplicationServices/Reference/AXUIElement_header_reference/#//apple_ref/c/tdef/AXUIElementRef
	
	A structure used to refer to an accessibility object.

	Declaration:
		typedef const struct __AXUIElement *AXUIElementRef;

	Discussion:
		An accessibility object provides information about the user interface 
		object it represents. This information includes the object's position in
		the accessibility hierarchy, its position on the display, details about
		what it is, and what actions it can perform. Accessibility objects 
		respond to messages sent by assistive applications and send 
		notifications that describe state changes.
*/

/* Get the Window Title from a WindowRef

    Map:
        WindowRef ~> WindowInfo

	Parameters:
        WindowRef - the reference to the Window

	Return:
		std::string WindowTitle - the title of the window

	Called Functions:
		AXUIElementCopyAttributeValue(..., kAXTitleAttribute, ...)	
		CFStringGetCStringPtr(..., kCSFStringEncodingMacRoman)
        helpers::GetUTF8String(...)

	Calling Functions:
		interpreter::FocusedAXObserverCallback(.., AXUIElementRef, .., ..)

    Notes:
      - Only called by interpreter
*/
std::string GetWindowTitle(AXUIElementRef WindowRef);

/* Get the Window Size from a WindowRef

	Map:
		WindowRef ~> WindowInfo

	Parameters:
		WindowRef - the reference to the window

	Return:
		CGSize WindowSize - the size of the window

	Called Functions:
		AXUIElementCopyAttributeValue(.., kAXSizeAttribute, ..)
		AXValueGetValue(.., kAXValueCGSizeType, ..)
	
	Calling Functions:
        windowref::CenterWindowInsideNodeContainer(AXUIElementRef, ...)
        windowref::MoveCursorToCenterOfWindow(...)
*/
CGSize GetWindowSize(AXUIElementRef WindowRef);

/* Get the Window Position from a WindowRef

	Map:
		WindowRef ~> WindowInfo

	Parameters:
		WindowRef - the reference to the window

	Return:
		CGPoint WindowPos - the position of the window

	Called Functions:
		AXUIElementCopyAttributeValue(.., kAXValueCGSizeType, ..)
		AXValueGetValue(.., kAXValueCGPointType, ..)

	Calling Functions:
        windowref::CenterWindowInsideNodeContainer(AXUIElementRef, ...)
		windowref::MoveFloatingWindow(...)
		windowref::MoveCursorToCenterOfWindow(...)
*/
CGPoint GetWindowPos(AXUIElementRef WindowRef);

/* Get the WindowRef from cache, if exists, else set KWMCache to empty for PID
 
    Map:
        Window ~> WindowRef
        
    Parameters:
        Window - the Window
    [M] WindowRef - the reference to the Window

    Mutations:
        WindowRef - stores the result of the lookup, if true, else invalid
        KWMCache.WindowRefs[Window->PID] - empty vector (if PID not cached)
    
    Return:
        bool - true  : WindowRef was found in cache
               false : WindowRef not found in cache

    Called Functions:
        application::IsApplicationInCache(Window->PID, ...)
        _AXUIElementGetWindow(...)
        
    Calling Functions:
        windowref::GetWindowRef(Window, WindowRef)
*/
bool GetWindowRefFromCache(window_info *Window, AXUIElementRef *WindowRef);

/* Get the WindowRef of the given Window

    Map:
        Window ~> WindowRef

    Parameters:
        Window - the Window to get the reference for
    [M] WindowRef - the resulting WindowRef

    Global References:
    [M] KWMCache.WindowRefs

    Mutations:
        WindowRef - store the result in this variable
        KWMCache.WindowRefs[Window->PID] - push back the AppWindowRef

    Return:
        bool - whether the WindowRef was found

    Called Functions:
        windowref::GetWindowRefFromCache(Window, WindowRef)
        windowref::FreeWindowRefCache(Window->PID)
        AXUIElementCreateApplication(Window->PID)
        AXUIElementCopyAttributeValue(.., kAXWindowsAttribute, ..)
        CFArrayGetValueAtIndex(...)
        _AXUIElementGetWindow(...)

    Calling Functions:
        window::GetWindowRole(Window, ...)
        windowref::SetWindowFocus(Window, ...)
        dispatcher::ResizeWindowToContainerSize(Window, ...)
        windowref::CenterWindow(.., Window)
        windowref::MoveFloatingWindow(...)
        windowref::MoveCursorToCenterOfWindow(Window)

    Notes:
      - Should GetWindowRole move down into the windowref layer?
*/
bool GetWindowRef(window_info *Window, AXUIElementRef *WindowRef);

/* Get the WID of the currently focused Window (from OS X)

    Map:
        WindowRef ~> WindowID

    Parameters:
    [M] WindowWID - the WID of the focused window in OS X

    Mutations:
        WindowWID - set the resulting WID in this parameter (invalid if returned false)

    Return:
        bool - true : the WindowID was found
               false : the WindowID was not found (mutated data invalid)

    Called Functions:
        AXUIElementCreateSystemWide()
        AXUIElementCopyAttributeValue(.., kAXFocusedApplicationAttribute, ..)
        AXUIElementCopyAttributeValue(.., kAXFocusedWindowAttribute, ..)
        _AXUIElementGetWindow(.., WindowWID)

    Calling Functions:
        window::FocusWindowOfOSX()
*/
bool GetWindowFocusedByOSX(int *WindowWID);

/* Set the Window as Focused in OS X, the Tree, the KWM globals; register Notifications.

    Map:
        WindowRef ~> KWM, Tree, WindowRef

    Parameters:
    [M] WindowRef - the reference to the Window
        Window - the Window
        Notification - whether notifications exist for Window->PID

    Global References:
    [M] KWMFocus.Window
    [M] KWMFocus.PSN
    [M] KWMFocus.Cache
        KWMFocus.PID
        KWMFocus.Observer
    [M] KWMMode.Focus
    [M] KWMScreen.Current->(ActiveSpace)
        KWMToggles.EnableTilingMode


    Mutations:
        WindowRef          set as focused in OS X
        KWMFocus.Window:   set to KWMFocus.Cache
        KWMFocus.PSN:      set to PSN of Window->PID (input)
        KWMFocus.Cache:    set to Window (input)
        KWMScreen.Focus:   set to IsFocusedWindowFloating() ? FocusModeStandby : FocusModeAutoraise
        KWMScreen.Current->(ActiveSpace)->FocusedNode: set to NodeOf(Window->WID)
    [C] notifications::CreateApplicationNotifications()

    Return:
        (void)

    Called Functions:
        notifications::CreateApplicationNotifications()
        space::IsActiveSpaceFloating()
        space::GetActiveSpaceOfScreen(...)
        tree::GetNodeFromWindowID
        window::IsFocusedWindowFloating()
        border::UpdateBorder(...)
        GetProcessForPID(Window->PID, ..)
        AXUIElementSetAttributeValue(WindowRef, kAXMainAttribute, kCFBooleanTrue);
        AXUIElementSetAttributeValue(WindowRef, kAXFocusedAttribute, kCFBooleanTrue);
        AXUIElementPerformAction(WindowRef, kAXRaiseAction);
        SetFrontProcessWithOptions(.., kSetFrontProcessFrontWindowOnly);

    Calling Functions:
        notifications::FocusedAXObserverCallback(.., Element, true)
        windowref::SetWindowFocus(Window)

    Notes:
      - This function has tendrils everywhere.. keep an eye out for repeated
        use of the above functions.

    TODO: refactor SetWindowRefFocus
*/
void SetWindowRefFocus(AXUIElementRef WindowRef, window_info *Window, bool Notification); // does tree lookup to set focused node

/* Set the Window as Focused (with SetWindowRefFocus)

    Map:
        Window ~> WindowRef

    Parameters:
        Window - the Window to focus

    Mutations:
        (see SetWindowRefFocus)

    Return:
        (void)

    Called Functions:
        windowref::SetWindowRefFocus(Window, .., false)

    Calling Functions:
        application::CaptureApplication(Window)
        window::FocusWindowOfOSX()
        window::FocusWindowBelowCursor()
        windowtree::SetWindowFocusByNode(...)
        dispatcher::FocusWindowByID(...)
        dispatcher::ShiftWindowFocusDirected(...)
        windowtree::ShouldBSPTreeUpdate(...)
        windowtree::ShouldMonocleTreeUpdate(...)

    Notes:
        Just a wrapper around SetWindowRefFocus which gets the WindowRef.
*/
void SetWindowFocus(window_info *Window); // gets WindowRef from Window, calls SetWindowRefFocus

/* Set the WindowRef position centered in OS X constrained by the parameters, and update parameters

    Map:
        WindowRef ~> WindowRef
        
    Parameters:
    [M] WindowRef - output position/size set for the window reference
    [M] Xptr - inout X pos of window
    [M] Yptr - inout Y pos of window
    [M] Wptr - inout Width of window
    [M] Hptr - inout Height of window

    Return:
        (void)

    Called Functions:
        windowref::GetWindowPos(WindowRef)
        windowref::GetWindowSize(WindowRef)
        CGPointMake(...)
        CGSizeMake(...)
        AXValueCreate(kAXValueCGPointType, ...)
        AXValueCreate(kAXValueCGSizeType, ...)
        AXUIElementSetAttributeValue(WindowRef, kAXSizeAttribute, ...)
        AXUIElementSetAttributeValue(WindowRef, kAXPositionAttribute, ...)

    Calling Functions:
        windowreff::SetWindowDimensions(WindowRef, *Xptr, *Yptr, *Wptr, *Hptr)

    Notes:
      - Doesn't have anything to do with node containers, should rename for clarity.
    TODO: Decode the calculation logic
*/
void CenterWindowInsideNodeContainer(AXUIElementRef WindowRef, int *Xptr, int *Yptr, int *Wptr, int *Hptr); // changes WindowSize of windowref

/* Set the Window dimensions of the given WindowRef and Window based on parameters

    Map:
        WindowRef ~> WindowRef

    Parameters:
    [M] WindowRef - WindowRef to reposition
    [M] Window - Window to determine if resizeable and update attributes for 
    [M] X
    [M] Y
    [M] Width
    [M] Height

    Mutations:
        (see CenterWindowInsideNodeContainer)
        Window->X
        Window->Y
        Window->Width
        Window->Height

    Return:
        (void)

    Global References:
        KWMTiling
         - FloatNonResizeable

    Called Functions:
        CGPointMake(X, Y)
        AXValueCreate(kAXValueCGPointType, ...)
        AXUIElementSetAttributeValue(WindowRef, kAXPositionAttribute, ...)
        CGSizeMake(Width, Height)
        AXValueCreate(kAXValueCGSizeType, ...)
        AXUIElementSetAttributeValue(WindowRef, kAXSizeAttribute, ...)
        IsWindowNonResizeable(WindowRef, Window, ...)
        CenterWindowInsideNodeContainer(WindowRef, X, Y, W, H)
        UpdateBorder()

    Calling Functions:
        ResizeWindowToContainerSize(Window, ...)
        CenterWindow(.., Window)
        
    Notes:
      - Do we really need to pass X, Y, W, H explicitly, or can we pass a struct
        of some kind?
      - The ints are modified within the function, but it's getting copies.
*/
void SetWindowDimensions(AXUIElementRef WindowRef, window_info *Window, int X, int Y, int Width, int Height);

/* Resize the Window of the given Window based on the Container; if Focused, update KWMFocus.Cache

    Map:
        Container ~> WindowRef

    Parameters:
        Window - the Window to change dimensions of
        Container - Container with dimensions

    Global References:
        KWMFocus.Window
    [M] KWMFocus.Cache

    Mutations:
        (see SetWindowDimensions)
        KWMFocus.Cache = *Window if Window is Focused

    Return:
        (void)

    Called Functions:
        windowref::GetWindowRef(Window, ..)
        windowref::SetWindowDimensions(.., Window, Container->X, Container->Y, Container->Width. Container->Height)
        windowref::WindowsAreEqual(Window, KWMFocus.Window)

    Calling Functions:
        container::ResizeElementInContainer(.., Container)

    Overloaded Functions:
        windowtree::ResizeWindowToContainerSize()

    TODO: Do we want the overloaded function windowtree::ResizeWindowToContainerSize() ?
*/
void ResizeWindowToContainerSize(window_info *Window, node_container *Container);

/* Set the Window to the center of the display, based on dimensions in Screen.

    Map:
        Screen ~> WindowRef // TODO give this a dimensions struct

    Parameters:
        Screen - the Screen whose position info is used.
    [M] Window - the Window to reposition and resize

    Mutations:
        (see SetWindowDimensions)

    Return:
        (void)

    Called Functions:
        windowref::GetWindowRef(Window, ..)
        windowref::SetWindowDimensions(.., Window, ...)

    Calling Functions:
        windowtree::AddWindowToTreeOfUnfocusedMonitor(Screen, Window)

    Notes:
      - Only uses Screen for X, Y, Width, Height
      - Could change to a struct which is passed to SetWindowDimensions and its
        called functions.
*/
void CenterWindow(screen_info *Screen, window_info *Window);

/* Set the currently focused Window's WindowRef to a new position by X and Y pixels.

    Map:
        WindowRef ~> WindowRef

    Parameters:
        X - amount to move the window in the X direction
        Y - amount to move the window int he Y direction

    Global References:
        KWMFocus.Window
        KWMFocus.Window->WID

    Mutations:
        WindowRef of KWMFocus.Window

    Return:
        (void)

    Called Functions:
        application::IsApplicationFloating(KWMFocus.Window)
        window::IsWindowFloating(KWMFocus.Window->WID, ..)
        windowref::GetWindowRef(KWMFocus.Window, ..)
        windowref::GetWindowPos(...)
        AXValueCreate(kAXValueCGPointType, ..)
        AXUIElementSetAttributeValue(.., kAXPositionAttribute, ..)

    Calling Functions:
        interpreter::KwmWindowCommand(...)
            
    Notes:
      - Only called by interpreter
      - Could build an abstraction barrier between Windows and WindowRefs
        -> in this function we use the Window to get the WindowRef, then
           operate on just the WindowRef
*/
void MoveFloatingWindow(int X, int Y);

/* Set the OS X Mouse Cursor to center of Window

    Map:
        WindowRef ~> OS X

    Parameters:
        WindowRef - the Window to which the cursor is moved

    Mutations:
        OS X Mouse Cursor position

    Return:
        (void)

    Called Functions:
        windowref::GetWindowPos(...)
        windowref::GetWindowSize(...)
        CGPointMake(...)
        CGWarpMouseCursorPosition(...)

    Calling Functions:
        windowref::MoveCursorToCenterOfFocusedWindow()
*/
void MoveCursorToCenterOfWindow(window_info *Window); // immediately gets the WindowRef

/* Set the OS X Mouse Cursor to center of KWMFocus.Window's WindowRef
    Map:
        KWMFocus.Window -> MoveCursorToCenterOfWindow

    Parameters:
        (none)

    Global References:
        KWMFocus.Window
        KWMToggles.UseMouseFollowsFocus

    Mutations:
        (see MoveCursorToCenterOfWindow)

    Return:
        (void)

    Called Functions:
        windowref::GetWindowRef(KWMFocus.Window, ..)
        windowref::MoveCursorToCenterOfWindow(KWMFocus.Window)

    Calling Functions:
        workspace     :: didActivateApplication(...)
        dispatcher    :: FocusFirstLeafNode()
        dispatcher    :: FocusLastLeafNode()
        display       :: GiveFocusToScreen(...)
        space         :: UpdateActiveSpace()
        dispatcher    :: FocusWindowByID(...)
        dispatcher    :: ShiftWindowFocus(...)
        dispatcher    :: ShiftWindowFocusDirected(...)
        windowtree    :: ShouldBSPTreeUpdate(...)
        windowtree    :: ShouldMonocleTreeUpdate(...)
        windowtree    :: RemoveWindowFromBSPTree(...)
        windowtree    :: RemoveWindowFromMonocleTree(...)
        dispatcher    :: DetachAndReinsertWindow(...)
        dispatcher    :: SwapFocusedWindowWithMarked()
        dispatcher    :: SwapFocusedWindowDirected()
        dispatcher    :: SwapFocusedWindowWithNearest()
        application   :: CaptureApplication(...)
        notifications :: FocusedAXObserverCallback(...)

    Notes:
      - It looks like this gets called whenever stuff is updated on the window.
      - Maybe look into the above to see if there's common code that it and 
        this function can be wrapped into.
*/
void MoveCursorToCenterOfFocusedWindow();

/* Remove the WindowRef's PID from KWMCache

    Map:
        WindowRef ~> WindowRef

    Parameters:
        PID - the PID of the process to remove from the cache

    Global References:
    [M] KWMCache.WindowRefs

    Mutations:
        KWMCache.WindowRefs[PID] is cleared
        (see GetWindowRef)

    Return:
        (void)

    Called Functions:
        (none)

    Calling Functions:
        windowtree::GetWindowRef(...)
*/
void FreeWindowRefCache(int PID);

/* If we can resize the WindowRef, resize it and find+remove the Window in its Tree

    Map:
        WindowRef ~> WindowRef, Tree

    Parameters:
    [M] WindowRef - WindowRef to set the position/size of.
        Window - Window being referenced
        NewWindowPos - Position to set the WindowRef to
        NewWindowSize - Size to set the WindowRef to

    Global References:
    [M] KWMTiling.FloatingWindowLst

    Mutations:
        KWMTiling.FloatingWindowLst append Window->WID
        Tree on (Screen of (Display of Window))) -> REMOVE(Window)

    Return:
        bool - true  : WindowRef was successfully mutated 
               false : WindowRef was not successfully mutated

    Called Functions:
        display::GetDisplayOfWindow(Window)
        space::DoesSpaceExistInMapOfScreen(...)
        space::GetActiveSpaceOfScreen(...)
        windowtree::RemoveWindowFromBSPTree(..., Window->WID, false)
        windowtree::RemoveWindowFromMonocleTree(..., Window->WID)
        AXUIElementSetAttributeValue(WindowREf, kAXPositionAttribute, NewWindowPos)
        AXUIElementSetAttributeValue(WindowRef, kAXSizeAttribute, NewWindowSize)

    Calling Functions:
        windowref::SetWindowDimensions(WindowRef, Window, ...)

    Notes:
        The tree removal stuff seems like a separate routine to be called
        whenever a window is resized -- it pops it out of the tree.

    TODO: Should NOT be mutating in a query function
*/
bool IsWindowNonResizable(AXUIElementRef WindowRef, window_info *Window, CFTypeRef NewWindowPos, CFTypeRef NewWindowSize);

#endif
