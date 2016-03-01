#ifndef WINDOWREF_H
#define WINDOWREF_H

#include "types.h"

/* GET */
std::string GetWindowTitle(AXUIElementRef WindowRef);
CGSize GetWindowSize(AXUIElementRef WindowRef);
CGPoint GetWindowPos(AXUIElementRef WindowRef);
bool GetWindowRefFromCache(window_info *Window, AXUIElementRef *WindowRef);
bool GetWindowRef(window_info *Window, AXUIElementRef *WindowRef);
bool GetWindowFocusedByOSX(int *WindowWID);

/* SET */
void SetWindowRefFocus(AXUIElementRef WindowRef, window_info *Window, bool Notification); // does tree lookup to set focused node
void SetWindowFocus(window_info *Window); // gets WindowRef from Window, calls SetWindowRefFocus
void CenterWindowInsideNodeContainer(AXUIElementRef WindowRef, int *Xptr, int *Yptr, int *Wptr, int *Hptr); // changes WindowSize of windowref
void SetWindowDimensions(AXUIElementRef WindowRef, window_info *Window, int X, int Y, int Width, int Height); // operates on window_info positioning & container
void ResizeWindowToContainerSize(window_info *Window, node_container *Container);
void CenterWindow(screen_info *Screen, window_info *Window);
void MoveFloatingWindow(int X, int Y);
void MoveCursorToCenterOfWindow(window_info *Window); // immediately gets the WindowRef
void MoveCursorToCenterOfFocusedWindow(); // just calls MoveCursorToCenterOfWindow on KWMFocus.Window

/* QUERY */
bool IsWindowNonResizable(AXUIElementRef WindowRef, window_info *Window, CFTypeRef NewWindowPos, CFTypeRef NewWindowSize);

#endif
