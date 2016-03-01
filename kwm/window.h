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
void FocusFirstLeafNode();
void FocusLastLeafNode();

void UpdateWindowTree();
std::vector<window_info> FilterWindowListAllDisplays();
bool FilterWindowList(screen_info *Screen);
void UpdateActiveWindowList(screen_info *Screen);
void CreateWindowNodeTree(screen_info *Screen, std::vector<window_info*> *Windows);
void ShouldWindowNodeTreeUpdate(screen_info *Screen);
void AddWindowToTreeOfUnfocusedMonitor(screen_info *Screen, window_info *Window);

void ShouldBSPTreeUpdate(screen_info *Screen, space_info *Space);
void AddWindowToBSPTree(screen_info *Screen, int WindowID);
void AddWindowToBSPTree();
void RemoveWindowFromBSPTree(screen_info *Screen, int WindowID, bool Refresh);
void RemoveWindowFromBSPTree();

void ShouldMonocleTreeUpdate(screen_info *Screen, space_info *Space);
void AddWindowToMonocleTree(screen_info *Screen, int WindowID);
void RemoveWindowFromMonocleTree(screen_info *Screen, int WindowID);

void ToggleWindowFloating(int WindowID);
void ToggleFocusedWindowFloating();
void ToggleFocusedWindowParent();
void ToggleFocusedWindowFullscreen();

void DetachAndReinsertWindow(int WindowID, int Degrees);
void SwapFocusedWindowWithMarked();
void SwapFocusedWindowDirected(int Degrees);
void SwapFocusedWindowWithNearest(int Shift);
void FocusWindowByID(int WindowID);
void ShiftWindowFocus(int Shift);
void ShiftWindowFocusDirected(int Degrees);
bool FindClosestWindow(int Degrees, window_info *Target, bool Wrap);
double GetWindowDistance(window_info *A, window_info *B);
void GetCenterOfWindow(window_info *Window, int *X, int *Y);
bool WindowIsInDirection(window_info *A, window_info *B, int Degrees, bool Wrap);

void ClearMarkedWindow();
void MarkWindowContainer(window_info *Window);
void MarkFocusedWindowContainer();

void SetWindowFocusByNode(tree_node *Node);

CGPoint GetCursorPos();
window_info *GetWindowByID(int WindowID);
std::string GetUTF8String(CFStringRef Temp);
void GetWindowInfo(const void *Key, const void *Value, void *Context);
bool GetWindowRole(window_info *Window, CFTypeRef *Role, CFTypeRef *SubRole);
bool IsApplicationInCache(int PID, std::vector<AXUIElementRef> *Elements);
void FreeWindowRefCache(int PID);
void ModifySubtreeSplitRatioFromWindow(const double &Offset);

#endif
