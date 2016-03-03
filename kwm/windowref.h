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

/* GET */
/* Get the Window Title from a WindowRef

    Map:
        WindowRef -> std::string WindowTitle

	Parameters:
        WindowRef - the reference to the window

	Return:
		std::string WindowTItle - the title of the window

	Called Functions:
		AXUIElementCopyAttributeValue(..., kAXTitleAttribute, ...)	
		GetUTF8String(...)
		CFStringGetCStringPtr(..., kCSFStringEncodingMacRoman)

	Calling Functions:
		interpreter::FocusedAXObserverCallback(.., AXUIElementRef, .., ..)
		
*/
std::string GetWindowTitle(AXUIElementRef WindowRef);

/* Get the window Size from a WindowRef

	Map:
		WindowRef -> WindowSize

	Parameters:
		WindowRef - the reference to the window

	Return:
		CGSize WindowSize - the size of the window

	Called Functions:
		AXUIElementCopyAttributeValue(.., kAXSizeAttribute, ..)
		AXValueGetValue(.., kAXValueCGSizeType, ..)
	
	Calling Functions:
		CenterWindowInsideNodeContainer(AXUIElementRef, ...)
		MoveCursorToCenterOfWindow(...)
*/
CGSize GetWindowSize(AXUIElementRef WindowRef);

/* Get the Window Position from a WindowRef

	Map:
		WindowRef -> Window Position

	Parameters:
		WindowRef - the reference to the window

	Return:
		CGPoint WindowPos - the position of the window

	Called Functions:
		AXUIElementCopyAttributeValue(.., kAXValueCGSizeType, ..)
		AXValueGetValue(.., kAXValueCGPointType, ..)

	Calling Functions:
		CenterWindowInsideNodeContainer(AXUIElementRef, ...)
		MoveFloatingWindow(...)
		MoveCursorToCenterOfWindow(...)
*/
CGPoint GetWindowPos(AXUIElementRef WindowRef);

/* Get the WindowRef from cache, if exists, else set KWMCache to empty for PID
 
    Map:
        Window->PID -> WindowRef
        
    Parameters:
        Window - the Window
    [M] WindowRef - the reference to the Window, storing the lookup result

    Mutations:
        KWMCache.WindowRefs[Window->PID] - empty vector
    
    Return:
        bool - true  : window was found in the cache
               false : the window was not found in the cache

    Called Functions:
        IsApplicationInCache(Window->PID, ...)
        _AXUIElementGetWindow(...)
        
    Calling Functions:
        GetWindowRef(Window, WindowRef)

    Notes:
        Function doesn't need Window, just Window->PID.
*/
bool GetWindowRefFromCache(window_info *Window, AXUIElementRef *WindowRef);

/* Get the WindowRef of the given Window
    Map:
        Window -> WindowRef

    Parameters:
        Window - the Window to get the reference for
        `->Owner
        `->Name
        `->PID
        `->WID
    [M] WindowRef - the resulting WindowRef

    Global References:
        KWMCache.WindowRefs

    Mutations:
        KWMCache.WindowRefs[Window->PID] - push back the AppWindowRef

    Return:
        bool - whether the WindowRef was found

    Called Functions:
        GetWindowRefFromCache(Window, WindowRef)
        AXUIElementCreateApplication(Window->PID)
        AXUIElementCopyAttributeValue(.., kAXWindowsAttribute, ..)
        FreeWindowRefCache(Window->PID)
        CFArrayGetValueAtIndex(...)
        _AXUIElementGetWindow(...)

    Calling Functions:
        GetWindowRole(Window, ...)
        SetWindowFocus(Window, ...)
        ResizeWindowToContainerSize(Window, ...)
        CenterWindow(.., Window)
        MoveFloatingWindow(...)
        MoveCursorToCenterOfWindow(Window)
*/
bool GetWindowRef(window_info *Window, AXUIElementRef *WindowRef);

/* Get the WID of the currently focused Window (from OS X)
    Map:
        <osx focused window> -> WindowWID

    Parameters:
    [M] WindowWID - the WID of the focused window in OS X

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

/* SET */
/* Set the window as focused in OS X, the Tree, the KWM globals, and register Notifications.
    Map:
        Window & Ref -> make Focused window

    Parameters:
    [M] WindowRef - the reference to the Window
        Window - the Window
        Notification - whether notifications exist for Window->PID

    Mutations:
        KWMFocus.Window:   KWMFocus.Cache
        KWMFocus.PSN:      PSN of Window->PID (input)
        KWMFocus.Cache:    Window (input)
        KWMScreen.Focus:   IsFocusedWindowFloating() ? FocusModeStandby : FocusModeAutoraise
        ActiveSpace->FocusedNode: NODE(Window->WID)
    [C] notifications::CreateApplicationNotifications()

    Return:
        (void)

    Global References:
        KWMFocus:
        [M] Window
            `->PID
        [M] PSN
        [M] Cache
            Observer

        KWMMode:
        [M] Focus = IsFocusedWindowFloating() ? FocusModeStandby : FocusModeAutoraise

        KWMScreen:
            Current
        [M]   - Active Space ->FocusedNode

        KWMToggles:
            EnableTilingMode

    Called Functions:
        GetProcessForPID(Window->PID, ..)
        AXUIElementSetAttributeValue(WindowRef, kAXMainAttribute, kCFBooleanTrue);
        AXUIElementSetAttributeValue(WindowRef, kAXFocusedAttribute, kCFBooleanTrue);
        AXUIElementPerformAction(WindowRef, kAXRaiseAction);
        SetFrontProcessWithOptions(.., kSetFrontProcessFrontWindowOnly);
        IsActiveSpaceFloating()
        notifications::CreateApplicationNotifications()
        UpdateBorder(...)
        space::GetActiveSpaceOfScreen(...)
        tree::GetNodeFromWindowID
        window::IsFocusedWindowFloating()

    Calling Functions:
        notifications::FocusedAXObserverCallback(.., Element, true)
        SetWindowFocus(Window)

    Notes:
        This function has tendrils everywhere.. keep an eye out for repeated
        use of the above functions.
*/
void SetWindowRefFocus(AXUIElementRef WindowRef, window_info *Window, bool Notification); // does tree lookup to set focused node

/* Set the Window as focused (with SetWindowRefFocus)

    Map:
        Window -> SetWindowRefFocus(...)

    Parameters:
        Window - the Window to focus

    Mutations:
        (see SetWindowRefFocus)

    Return:
        (void)

    Called Functions:
        SetWindowRefFocus(Window, .., false)

    Calling Functions:
        application::CaptureApplication(Window)
        window::FocusWindowOfOSX()
        window::FocusWindowBelowCursor()
        SetWindowFocusByNode(...)
        FocusWindowByID(...)
        ShiftWindowFocusDirected(...)
        ShouldBSPTreeUpdate(...)
        ShouldMonocleTreeUpdate(...)

    Notes:
        Just a wrapper around SetWindowRefFocus which gets the WindowRef.
*/
void SetWindowFocus(window_info *Window); // gets WindowRef from Window, calls SetWindowRefFocus

/* Set the WindowRef position centered in OS X constrained by the parameters, and update parameters

    Map:
        WindowRef + dimensions -> Centered Window in OS X
        
    Parameters:
    [M] WindowRef - output position/size set for the window reference
    [M] Xptr - inout X pos of window
    [M] Yptr - inout Y pos of window
    [M] Wptr - inout Width of window
    [M] Hptr - inout Height of window

    Return:
        (void)

    Called Functions:
        GetWindowPos(WindowRef)
        CGPointMake(...)
        AXValueCreate(kAXValueCGPointType, ...)
        AXUIElementSetAttributeValue(WindowRef, kAXPositionAttribute, ...)
        GetWindowSize(WindowRef)
        CGSizeMake(...)
        AXValueCreate(kAXValueCGSizeType, ...)
        AXUIElementSetAttributeValue(WindowRef, kAXSizeAttribute, ...)

    Calling Functions:
        SetWindowDimensions(WindowRef, *Xptr, *Yptr, *Wptr, *Hptr)

    Notes:
        TODO - Decode the calculation logic
*/
void CenterWindowInsideNodeContainer(AXUIElementRef WindowRef, int *Xptr, int *Yptr, int *Wptr, int *Hptr); // changes WindowSize of windowref

/* Set the Window dimensions of the given WindowRef and Window based on parameters
    Map:
        window position -> new window position

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
        Container dimensions -> Window position        

    Parameters:
    [M] Window - the Window to change dimensions of
        Container - a node container with dimensions

    Global References:
        KWMFocus.Window
    [M] KWMFocus.Cache

    Mutations:
        (see SetWindowDimensions)
        WindowRef of Window is updated
        KWMFocus.Cache = *Window

    Return:
        (void)

    Called Functions:
        GetWindowRef(Window, ..)
        SetWindowDimensions(.., Window, Container->X, Container->Y, Container->Width. Container->Height)
        WindowsAreEqual(Window, KWMFocus.Window)

    Calling Functions:
        ResizeElementInContainer(.., Container)

    Overloaded Functions:
        windowtree::ResizeElementInContainer()
*/
void ResizeWindowToContainerSize(window_info *Window, node_container *Container);

/* Set the Window to the center of the display, based on dimensions in Screen.

    Map:
        Screen dimensions & Window Position -> New Window Position

    Parameters:
        Screen - the Screen whose position info is used.
    [M] Window - the Window to reposition and resize

    Mutations:
        (see SetWindowDimensions)

    Return:
        (void)

    Called Functions:
        GetWindowRef(Window, ..)
        SetWindowDimensions(.., Window, ...)

    Calling Functions:
        AddWindowToTreeOfUnfocusedMonitor(Screen, Window)

    Notes:
      - Only uses Screen for X, Y, Width, Height
      - Could change to a struct which is passed to SetWindowDimensions and its
        called functions.
*/
void CenterWindow(screen_info *Screen, window_info *Window);

/* Set the currently focused Window's WindowRef to a new position by X and Y pixels.
    Map:
        X,Y movements && KWMFocus.Window -> Reposition KWMFocus.Window

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
        window::IsWindowFloating(KWMFocus.Window->WID, ..)
        application::IsApplicationFloating(KWMFocus.Window)
        GetWindowRef(KWMFocus.Window, ..)
        GetWindowPos(...)
        AXValueCreate(kAXValueCGPointType, ..)
        AXUIElementSetAttributeValue(.., kAXPositionAttribute, ..)

    Calling Functions:
        interpreter::KwmWindowCommand(...)
        MoveCursorToCenterOfFocusedWindow()
            
    Notes:
      - Could build an abstraction barrier between Windows and WindowRefs
        -> in this function we use the Window to get the WindowRef, then
           operate on just the WindowRef
*/
void MoveFloatingWindow(int X, int Y);

/* Set the OS X Mouse Cursor to center of Window's WindowRef
    Map:
        Window's WindowRef -> OS X Cursor Position

    Parameters:
        Window - the Window to which the cursor is moved

    Mutations:
        OS X Mouse Cursor position

    Return:
        (void)

    Called Functions:
        GetWindowRef(Window, ..)
        GetWindowPos(...)
        GetWindowSize(...)
        CGPointMake(...)
        CGWarpMouseCursorPosition(...)

    Calling Functions:

    Notes:
        Just immediately gets a WindowRef to operate on
*/
void MoveCursorToCenterOfWindow(window_info *Window); // immediately gets the WindowRef

/* Set the OS X Mouse Cursor to center of KWMFocus.Window's WindowRef
    Map:
        KWMFocus.Window -> MoveCursorToCenterOfWindow

    Parameters:
        (none)

    Mutations:
        (see MoveCursorToCenterOfWindow)

    Return:
        (void)

    Called Functions:
        MoveCursorToCenterOfWindow(KWMFocus.Window)

    Calling Functions:
        application::CaptureApplication(...)
        display::GiveFocusToScreen(...)
        notifications::FocusedAXObserverCallback(...)
        space::UpdateActiveSpace()
        windowtree::FocusFirstLeafNode()
        windowtree::FocusLastLeafNode()
        windowtree::FocusWindowByID(...)
        windowtree::ShiftWindowFocus(...)
        windowtree::ShiftWindowFocusDirected(...)
        windowtree::ShouldBSPTreeUpdate(...)
        windowtree::ShouldMonocleTreeUpdate(...)
        windowtree::RemoveWindowFromBSPTree(...)
        windowtree::RemoveWindowFromMonocleTree(...)
        windowtree::DetachAndReinsertWindow(...)
        windowtree::SwapFocusedWindowWithMarked()
        windowtree::SwapFocusedWindowDirected()
        windowtree::SwapFocusedWindowWithNearest()
        workspace::didActivateApplication(...)

    Notes:
      - It looks like this gets called whenever stuff is updated on the window.
      - Maybe look into the above to see if there's common code that it and 
        this function can be wrapped into.
*/
void MoveCursorToCenterOfFocusedWindow();

/* Remove the WindowRef's PID from KWMCache

    Map:
        PID -> clear KWMCache.WindowRefs[PID]

    Parameters:
        PID - the PID of the process to remove from the cache

    Global References:
    [M] KWMCache.WindowRefs

    Mutations:
        KWMCache.WindowRefs[PID] is cleared

    Return:
        (void)

    Called Functions:
        (none)

    Calling Functions:
        GetWindowRef(...)
*/
void FreeWindowRefCache(int PID);

/* QUERY */

/* If we can resize the WindowRef, resize it and find+remove the Window in its Tree

    Map:
        WindowRef -> Try to reposition WindowRef && remove Window from Tree

    Parameters:
    [M] WindowRef - WindowRef to set the position/size of.
        Window - Window being referenced
        NewWindowPos - Position to set the WindowRef to
        NewWindowSize - Size to set the WindowRef to

    Global References:
    [M] KWMTiling.FloatingWindowLst

    Mutations:
        KWMTiling.FloatingWindowLst append Window->WID
        Tree on (Screen of (Display of Window))) -> REMOVE Window->WID

    Return:
        bool - true  : WindowRef was successfully mutated 
               false : WindowRef was not successfully mutated

    Called Functions:
        AXUIElementSetAttributeValue(WindowREf, kAXPositionAttribute, NewWindowPos)
        AXUIElementSetAttributeValue(WindowRef, kAXSizeAttribute, NewWindowSize)
        GetDisplayOfWindow(Window)
        DoesSpaceExistInMapOfScreen(...)
        GetActiveSpaceOfScreen(...)
        RemoveWindowFromBSPTree(..., Window->WID, false)
        RemoveWindowFromMonocleTree(..., Window->WID)

    Calling Functions:
        SetWindowDimensions(WindowRef, Window, ...)

    Notes:
        Should NOT be mutating in a query function
        The tree removal stuff seems like a separate routine to be called
        whenever a window is resized -- it pops it out of the tree.
*/
bool IsWindowNonResizable(AXUIElementRef WindowRef, window_info *Window, CFTypeRef NewWindowPos, CFTypeRef NewWindowSize);

#endif
