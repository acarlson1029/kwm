/* Map Window arguments onto the Tree structure. */

#ifndef WINDOWTREE_H
#define WINDOWTREE_H

#include "types.h"
/* Focus */
void SetWindowFocusByNode(tree_node *Node); // whereshould this go? it just calls SetWindowFocus(Window)
void FocusFirstLeafNode();
void FocusLastLeafNode();
void FocusWindowByID(int WindowID);
void ShiftWindowFocus(int Shift);
void ShiftWindowFocusDirected(int Degrees);

/* Determine whether to create the tree, update the tree, or destroy the tree */
void UpdateWindowTree();
void CreateWindowNodeTree(screen_info *Screen, std::vector<window_info*> *Windows);
void ShouldWindowNodeTreeUpdate(screen_info *Screen);
void ShouldBSPTreeUpdate(screen_info *Screen, space_info *Space);
void ShouldMonocleTreeUpdate(screen_info *Screen, space_info *Space);

/* Add/Remove windows */
void AddWindowToBSPTree(screen_info *Screen, int WindowID);
void AddWindowToBSPTree();
void AddWindowToMonocleTree(screen_info *Screen, int WindowID);
void AddWindowToTreeOfUnfocusedMonitor(screen_info *Screen, window_info *Window);
void RemoveWindowFromBSPTree(screen_info *Screen, int WindowID, bool Refresh);
void RemoveWindowFromBSPTree();
void RemoveWindowFromMonocleTree(screen_info *Screen, int WindowID);

/* Add/Remove Window from Tree to let it float */
void ToggleWindowFloating(int WindowID);
/* uses KWMFocus window as arg to ToggleWindowFloating() */
void ToggleFocusedWindowFloating(); // calls window->tree function
/* Add then Remove Window from Tree */
void DetachAndReinsertWindow(int WindowID, int Degrees);
/* Toggle window filling parent container */
void ToggleFocusedWindowParent();
/* Toggle window filling fullscreen */
void ToggleFocusedWindowFullscreen();
/* Swapping Nodes */
void SwapFocusedWindowWithMarked();
void SwapFocusedWindowDirected(int Degrees);
void SwapFocusedWindowWithNearest(int Shift);

/* Automatically find current window and call the ModifySubtree function */
void ModifySubtreeSplitRatioFromWindow(const double &Offset);

void ResizeElementInTree(screen_info *Screen, window_info *Window);
void ResizeWindowToContainerSize();

#endif

