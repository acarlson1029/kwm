#ifndef WINDOW_H
#define WINDOW_H

#include "types.h"

extern int GetActiveSpaceOfDisplay(screen_info *Screen);

bool IsFocusedWindowFloating();
bool IsWindowFloating(int WindowID, int *Index);
bool IsAnyWindowBelowCursor();
bool IsWindowBelowCursor(window_info *Window);
bool IsWindowOnActiveSpace(int WindowID);
bool WindowsAreEqual(window_info *Window, window_info *Match);
void ClearFocusedWindow();
bool ShouldWindowGainFocus(window_info *Window);
int GetFocusedWindowID();
bool FocusWindowOfOSX();
void FocusWindowBelowCursor();
std::vector<window_info> FilterWindowListAllDisplays();
bool FilterWindowList(screen_info *Screen);
void UpdateActiveWindowList(screen_info *Screen);
bool FindClosestWindow(int Degrees, window_info *Target, bool Wrap);
double GetWindowDistance(window_info *A, window_info *B);
void GetCenterOfWindow(window_info *Window, int *X, int *Y);
bool WindowIsInDirection(window_info *A, window_info *B, int Degrees, bool Wrap);
void ClearMarkedWindow();
void MarkWindowContainer(window_info *Window);
void MarkFocusedWindowContainer();
CGPoint GetCursorPos();
window_info *GetWindowByID(int WindowID);
std::string GetUTF8String(CFStringRef Temp);
void GetWindowInfo(const void *Key, const void *Value, void *Context);
bool GetWindowRole(window_info *Window, CFTypeRef *Role, CFTypeRef *SubRole);
bool IsApplicationInCache(int PID, std::vector<AXUIElementRef> *Elements);

#endif
