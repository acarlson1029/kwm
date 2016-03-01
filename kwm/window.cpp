#include "window.h"
#include "display.h"
#include "space.h"
#include "tree.h"
#include "notifications.h"
#include "border.h"
#include "node.h"
#include "application.h"
#include "windowref.h"

#include <cmath>

extern kwm_screen KWMScreen;
extern kwm_focus KWMFocus;
extern kwm_toggles KWMToggles;
extern kwm_mode KWMMode;
extern kwm_tiling KWMTiling;
extern kwm_cache KWMCache;
extern kwm_path KWMPath;
extern kwm_border MarkedBorder;
extern kwm_border FocusedBorder;

bool WindowsAreEqual(window_info *Window, window_info *Match)
{
    if(!KWMScreen.ForceRefreshFocus && Window && Match)
    {
        if(Window->PID == Match->PID &&
           Window->WID == Match->WID &&
           Window->Layer == Match->Layer)
            return true;
    }

    return false;
}

std::vector<window_info> FilterWindowListAllDisplays()
{
    std::vector<window_info> FilteredWindowLst;
    for(std::size_t WindowIndex = 0; WindowIndex < KWMTiling.FocusLst.size(); ++WindowIndex)
    {
        if(KWMTiling.FocusLst[WindowIndex].Layer == 0)
        {
            CFTypeRef Role, SubRole;
            if(GetWindowRole(&KWMTiling.FocusLst[WindowIndex], &Role, &SubRole))
            {
                if((CFEqual(Role, kAXWindowRole) && CFEqual(SubRole, kAXStandardWindowSubrole)) ||
                   IsAppSpecificWindowRole(&KWMTiling.FocusLst[WindowIndex], Role, SubRole))
                        FilteredWindowLst.push_back(KWMTiling.FocusLst[WindowIndex]);
            }
        }
    }

    return FilteredWindowLst;
}

bool FilterWindowList(screen_info *Screen)
{
    std::vector<window_info> FilteredWindowLst;
    for(std::size_t WindowIndex = 0; WindowIndex < KWMTiling.WindowLst.size(); ++WindowIndex)
    {
        /* Note(koekeishiya):
         * Mission-Control mode is on and so we do not try to tile windows */
        if(KWMTiling.WindowLst[WindowIndex].Owner == "Dock" &&
           KWMTiling.WindowLst[WindowIndex].Name == "")
        {
                ClearFocusedWindow();
                ClearMarkedWindow();
                return false;
        }

        CaptureApplication(&KWMTiling.WindowLst[WindowIndex]);
        if(KWMTiling.WindowLst[WindowIndex].Layer == 0 &&
           Screen == GetDisplayOfWindow(&KWMTiling.WindowLst[WindowIndex]))
        {
            CFTypeRef Role, SubRole;
            if(GetWindowRole(&KWMTiling.WindowLst[WindowIndex], &Role, &SubRole))
            {
                if((CFEqual(Role, kAXWindowRole) && CFEqual(SubRole, kAXStandardWindowSubrole)) ||
                   IsAppSpecificWindowRole(&KWMTiling.WindowLst[WindowIndex], Role, SubRole))
                        FilteredWindowLst.push_back(KWMTiling.WindowLst[WindowIndex]);
            }
        }
    }

    KWMTiling.WindowLst = FilteredWindowLst;
    return true;
}

bool IsFocusedWindowFloating()
{
    return KWMFocus.Window && (IsWindowFloating(KWMFocus.Window->WID, NULL) || IsApplicationFloating(KWMFocus.Window));
}

bool IsWindowFloating(int WindowID, int *Index)
{
    for(std::size_t WindowIndex = 0; WindowIndex < KWMTiling.FloatingWindowLst.size(); ++WindowIndex)
    {
        if(WindowID == KWMTiling.FloatingWindowLst[WindowIndex])
        {
            if(Index)
                *Index = WindowIndex;

            return true;
        }
    }

    return false;
}

bool IsAnyWindowBelowCursor()
{
    CGPoint Cursor = GetCursorPos();
    for(std::size_t WindowIndex = 0; WindowIndex < KWMTiling.FocusLst.size(); ++WindowIndex)
    {
        window_info *Window = &KWMTiling.FocusLst[WindowIndex];
        if(Cursor.x >= Window->X &&
           Cursor.x <= Window->X + Window->Width &&
           Cursor.y >= Window->Y &&
           Cursor.y <= Window->Y + Window->Height)
            return true;
    }

    return false;
}

bool IsWindowBelowCursor(window_info *Window)
{
    Assert(Window, "IsWindowBelowCursor()")

    CGPoint Cursor = GetCursorPos();
    if(Cursor.x >= Window->X &&
       Cursor.x <= Window->X + Window->Width &&
       Cursor.y >= Window->Y &&
       Cursor.y <= Window->Y + Window->Height)
        return true;

    return false;
}

bool IsWindowOnActiveSpace(int WindowID)
{
    for(std::size_t WindowIndex = 0; WindowIndex < KWMTiling.WindowLst.size(); ++WindowIndex)
    {
        if(WindowID == KWMTiling.WindowLst[WindowIndex].WID)
        {
            DEBUG("IsWindowOnActiveSpace() window found")
            return true;
        }
    }

    DEBUG("IsWindowOnActiveSpace() window was not found")
    return false;
}

int GetFocusedWindowID()
{
    return (KWMFocus.Window && KWMFocus.Window->Layer == 0) ? KWMFocus.Window->WID : -1;
}

void ClearFocusedWindow()
{
    ClearBorder(&FocusedBorder);
    KWMFocus.Window = NULL;
    KWMFocus.Cache = KWMFocus.NULLWindowInfo;
}

bool FocusWindowOfOSX()
{
    int WindowID;
    if(GetWindowFocusedByOSX(&WindowID))
    {
        if(IsSpaceTransitionInProgress() ||
           !IsActiveSpaceManaged())
            return false;

        for(std::size_t WindowIndex = 0; WindowIndex < KWMTiling.WindowLst.size(); ++WindowIndex)
        {
            if(KWMTiling.WindowLst[WindowIndex].WID == WindowID)
            {
                SetWindowFocus(&KWMTiling.WindowLst[WindowIndex]);
                return true;
            }
        }
    }

    return false;
}

bool ShouldWindowGainFocus(window_info *Window)
{
    return Window->Layer == CGWindowLevelForKey(kCGNormalWindowLevelKey) ||
           Window->Layer == CGWindowLevelForKey(kCGFloatingWindowLevelKey) ||
           Window->Layer == CGWindowLevelForKey(kCGTornOffMenuWindowLevelKey) ||
           Window->Layer == CGWindowLevelForKey(kCGDockWindowLevelKey) ||
           Window->Layer == CGWindowLevelForKey(kCGMainMenuWindowLevelKey) ||
           Window->Layer == CGWindowLevelForKey(kCGMaximumWindowLevelKey) ||
           Window->Layer == CGWindowLevelForKey(kCGModalPanelWindowLevelKey) ||
           Window->Layer == CGWindowLevelForKey(kCGUtilityWindowLevelKey) ||
           Window->Layer == CGWindowLevelForKey(kCGOverlayWindowLevelKey) ||
           Window->Layer == CGWindowLevelForKey(kCGHelpWindowLevelKey) ||
           Window->Layer == CGWindowLevelForKey(kCGPopUpMenuWindowLevelKey);
}

void FocusFirstLeafNode()
{
    if(!DoesSpaceExistInMapOfScreen(KWMScreen.Current))
        return;

    space_info *Space = GetActiveSpaceOfScreen(KWMScreen.Current);
    SetWindowFocusByNode(GetFirstLeafNode(Space->RootNode));
    MoveCursorToCenterOfFocusedWindow();
}

void FocusLastLeafNode()
{
    if(!DoesSpaceExistInMapOfScreen(KWMScreen.Current))
        return;

    space_info *Space = GetActiveSpaceOfScreen(KWMScreen.Current);
    SetWindowFocusByNode(GetLastLeafNode(Space->RootNode));
    MoveCursorToCenterOfFocusedWindow();
}

void FocusWindowBelowCursor()
{
    if(IsSpaceTransitionInProgress() ||
       !IsActiveSpaceManaged())
           return;

    for(std::size_t WindowIndex = 0; WindowIndex < KWMTiling.FocusLst.size(); ++WindowIndex)
    {
        if(KWMTiling.FocusLst[WindowIndex].Owner == "kwm-overlay")
            continue;

        if(KWMTiling.FocusLst[WindowIndex].Owner == "Dock" &&
           KWMTiling.FocusLst[WindowIndex].X == 0 &&
           KWMTiling.FocusLst[WindowIndex].Y == 0)
            continue;

        if(IsWindowBelowCursor(&KWMTiling.FocusLst[WindowIndex]) && ShouldWindowGainFocus(&KWMTiling.FocusLst[WindowIndex]))
        {
            if(WindowsAreEqual(KWMFocus.Window, &KWMTiling.FocusLst[WindowIndex]))
                KWMFocus.Cache = KWMTiling.FocusLst[WindowIndex];
            else
                SetWindowFocus(&KWMTiling.FocusLst[WindowIndex]);

            return;
        }
    }
}

void UpdateWindowTree()
{
    if(IsSpaceTransitionInProgress() ||
       !IsActiveSpaceManaged())
        return;

    UpdateActiveWindowList(KWMScreen.Current);
    if(KWMToggles.EnableTilingMode &&
       FilterWindowList(KWMScreen.Current))
    {
        std::vector<window_info*> WindowsOnDisplay = GetAllWindowsOnDisplay(KWMScreen.Current->ID);
        space_info *Space = GetActiveSpaceOfScreen(KWMScreen.Current);

        if(!IsActiveSpaceFloating())
        {
            if(!Space->Initialized)
                CreateWindowNodeTree(KWMScreen.Current, &WindowsOnDisplay);
            else if(Space->Initialized &&
                    !WindowsOnDisplay.empty() &&
                    !Space->RootNode)
                CreateWindowNodeTree(KWMScreen.Current, &WindowsOnDisplay);
            else if(Space->Initialized &&
                    !WindowsOnDisplay.empty() &&
                    Space->RootNode)
                ShouldWindowNodeTreeUpdate(KWMScreen.Current);
            else if(Space->Initialized && WindowsOnDisplay.empty())
            {
                DestroyNodeTree(Space->RootNode, Space->Mode);
                Space->RootNode = NULL;
                ClearFocusedWindow();
            }
        }
    }
}

void UpdateActiveWindowList(screen_info *Screen)
{
    static CGWindowListOption OsxWindowListOption = kCGWindowListOptionOnScreenOnly |
                                                    kCGWindowListExcludeDesktopElements;

    Screen->OldWindowListCount = KWMTiling.WindowLst.size();
    KWMTiling.WindowLst.clear();

    CFArrayRef OsxWindowLst = CGWindowListCopyWindowInfo(OsxWindowListOption, kCGNullWindowID);
    if(!OsxWindowLst)
        return;

    CFIndex OsxWindowCount = CFArrayGetCount(OsxWindowLst);
    for(CFIndex WindowIndex = 0; WindowIndex < OsxWindowCount; ++WindowIndex)
    {
        CFDictionaryRef Elem = (CFDictionaryRef)CFArrayGetValueAtIndex(OsxWindowLst, WindowIndex);
        KWMTiling.WindowLst.push_back(window_info());
        CFDictionaryApplyFunction(Elem, GetWindowInfo, NULL);
    }
    CFRelease(OsxWindowLst);
    KWMTiling.FocusLst = KWMTiling.WindowLst;
}

void CreateWindowNodeTree(screen_info *Screen, std::vector<window_info*> *Windows)
{
    for(std::size_t WindowIndex = 0; WindowIndex < Windows->size(); ++WindowIndex)
    {
        if(Screen != GetDisplayOfWindow((*Windows)[WindowIndex]))
            return;
    }

    space_info *Space = GetActiveSpaceOfScreen(Screen);
    if(!Space->Initialized)
    {
        Assert(Space, "CreateWindowNodeTree()")
        DEBUG("CreateWindowNodeTree() Create Space " << Screen->ActiveSpace)

        Space->Mode = GetSpaceModeOfDisplay(Screen->ID);
        if(Space->Mode == SpaceModeDefault)
            Space->Mode = KWMMode.Space;

        Space->Initialized = true;
        Space->Offset = Screen->Offset;
        Space->RootNode = CreateTreeFromWindowIDList(Screen, *Windows);
    }
    else if(Space->Initialized)
    {
        Space->RootNode = CreateTreeFromWindowIDList(Screen, *Windows);
        ResizeTreeNodes(Screen, Space->RootNode);
        ApplyNodeContainer(Space->RootNode, Space->Mode);
    }

    if(Space->RootNode)
    {
        ApplyNodeContainer(Space->RootNode, Space->Mode);
        FocusWindowBelowCursor();
    }
}

void ShouldWindowNodeTreeUpdate(screen_info *Screen)
{
    if(Screen->ActiveSpace == -1 || Screen->OldWindowListCount == -1)
        return;

    space_info *Space = GetActiveSpaceOfScreen(Screen);
    if(Space->Mode == SpaceModeBSP)
        ShouldBSPTreeUpdate(Screen, Space);
    else if(Space->Mode == SpaceModeMonocle)
        ShouldMonocleTreeUpdate(Screen, Space);
}

void ShouldBSPTreeUpdate(screen_info *Screen, space_info *Space)
{
    if(KWMTiling.WindowLst.size() > Screen->OldWindowListCount)
    {
        for(std::size_t WindowIndex = 0; WindowIndex < KWMTiling.WindowLst.size(); ++WindowIndex)
        {
            if(!GetNodeFromWindowID(Space->RootNode, KWMTiling.WindowLst[WindowIndex].WID, Space->Mode))
            {
                if(!IsApplicationFloating(&KWMTiling.WindowLst[WindowIndex]) &&
                   !IsWindowFloating(KWMTiling.WindowLst[WindowIndex].WID, NULL))
                {
                    DEBUG("ShouldBSPTreeUpdate() Add Window")
                    tree_node *Insert = GetFirstPseudoLeafNode(Space->RootNode);
                    if(Insert)
                    {
                        Insert->WindowID = KWMTiling.WindowLst[WindowIndex].WID;
                        ApplyNodeContainer(Insert, SpaceModeBSP);
                    }
                    else
                    {
                        AddWindowToBSPTree(Screen, KWMTiling.WindowLst[WindowIndex].WID);
                    }

                    SetWindowFocus(&KWMTiling.WindowLst[WindowIndex]);
                    MoveCursorToCenterOfFocusedWindow();
                }
            }
        }
    }
    else if(KWMTiling.WindowLst.size() < Screen->OldWindowListCount)
    {
        std::vector<int> WindowIDsInTree;

        tree_node *CurrentNode = GetFirstLeafNode(Space->RootNode);
        while(CurrentNode)
        {
            WindowIDsInTree.push_back(CurrentNode->WindowID);
            CurrentNode = GetNearestNodeToTheRight(CurrentNode, SpaceModeBSP);
        }

        for(std::size_t IDIndex = 0; IDIndex < WindowIDsInTree.size(); ++IDIndex)
        {
            bool Found = false;
            for(std::size_t WindowIndex = 0; WindowIndex < KWMTiling.WindowLst.size(); ++WindowIndex)
            {
                if(KWMTiling.WindowLst[WindowIndex].WID == WindowIDsInTree[IDIndex])
                {
                    Found = true;
                    break;
                }
            }

            if(!Found)
            {
                DEBUG("ShouldBSPTreeUpdate() Remove Window " << WindowIDsInTree[IDIndex])
                RemoveWindowFromBSPTree(Screen, WindowIDsInTree[IDIndex], true);
            }
        }

        if(!KWMFocus.Window)
        {
            ClearFocusedWindow();
            if(IsAnyWindowBelowCursor() && KWMMode.Focus != FocusModeDisabled)
                FocusWindowBelowCursor();
            else if(FocusWindowOfOSX())
                MoveCursorToCenterOfFocusedWindow();
        }
    }
}

void AddWindowToBSPTree(screen_info *Screen, int WindowID)
{
    if(!DoesSpaceExistInMapOfScreen(Screen))
        return;

    space_info *Space = GetActiveSpaceOfScreen(Screen);
    tree_node *RootNode = Space->RootNode;
    tree_node *CurrentNode = NULL;

    DEBUG("AddWindowToBSPTree() Create pair of leafs")
    bool UseFocusedContainer = KWMFocus.Window &&
                               IsWindowOnActiveSpace(KWMFocus.Window->WID) &&
                               KWMFocus.Window->WID != WindowID;

    bool DoNotUseMarkedContainer = IsWindowFloating(KWMScreen.MarkedWindow, NULL) ||
                                   (KWMScreen.MarkedWindow == WindowID);

    if(KWMScreen.MarkedWindow == -1 && UseFocusedContainer)
    {
        CurrentNode = GetNodeFromWindowID(RootNode, KWMFocus.Window->WID, Space->Mode);
    }
    else if(DoNotUseMarkedContainer || (KWMScreen.MarkedWindow == -1 && !UseFocusedContainer))
    {
        // Get the first LeafNode via level-order search
        CurrentNode = LevelOrderSearch(IsLeafNode, RootNode);
    }
    else
    {
        CurrentNode = GetNodeFromWindowID(RootNode, KWMScreen.MarkedWindow, Space->Mode);
        ClearMarkedWindow();
    }
    AddElementToTree(Screen, CurrentNode, WindowID, KWMScreen.SplitMode, SpaceModeBSP);
}

void AddWindowToBSPTree()
{
    if(!KWMScreen.Current)
        return;

    AddWindowToBSPTree(KWMScreen.Current, KWMFocus.Window->WID);
}

void RemoveWindowFromBSPTree(screen_info *Screen, int WindowID, bool Refresh)
{
    if(!DoesSpaceExistInMapOfScreen(Screen))
        return;

    space_info *Space = GetActiveSpaceOfScreen(Screen);
    RemoveElementFromTree(Screen, Space->RootNode, WindowID, Space->Mode);

    if(Refresh)
    {
        SetWindowFocusByNode(GetFirstLeafNode(Space->RootNode));
        MoveCursorToCenterOfFocusedWindow();
    }
}

void RemoveWindowFromBSPTree()
{
    if(!KWMScreen.Current)
        return;

    RemoveWindowFromBSPTree(KWMScreen.Current, KWMFocus.Window->WID, true);
}

void ShouldMonocleTreeUpdate(screen_info *Screen, space_info *Space)
{
    if(KWMTiling.WindowLst.size() > Screen->OldWindowListCount)
    {
        DEBUG("ShouldMonocleTreeUpdate() Add Window")
        for(std::size_t WindowIndex = 0; WindowIndex < KWMTiling.WindowLst.size(); ++WindowIndex)
        {
            if(!GetNodeFromWindowID(Space->RootNode, KWMTiling.WindowLst[WindowIndex].WID, Space->Mode))
            {
                if(!IsApplicationFloating(&KWMTiling.WindowLst[WindowIndex]))
                {
                    AddWindowToMonocleTree(Screen, KWMTiling.WindowLst[WindowIndex].WID);
                    SetWindowFocus(&KWMTiling.WindowLst[WindowIndex]);
                    MoveCursorToCenterOfFocusedWindow();
                }
            }
        }
    }
    else if(KWMTiling.WindowLst.size() < Screen->OldWindowListCount)
    {
        DEBUG("ShouldMonocleTreeUpdate() Remove Window")
        std::vector<int> WindowIDsInTree;

        tree_node *CurrentNode = Space->RootNode;
        while(CurrentNode)
        {
            WindowIDsInTree.push_back(CurrentNode->WindowID);
            CurrentNode = GetNearestNodeToTheRight(CurrentNode, SpaceModeMonocle);
        }

        if(WindowIDsInTree.size() >= 2)
        {
            for(std::size_t IDIndex = 0; IDIndex < WindowIDsInTree.size(); ++IDIndex)
            {
                bool Found = false;
                for(std::size_t WindowIndex = 0; WindowIndex < KWMTiling.WindowLst.size(); ++WindowIndex)
                {
                    if(KWMTiling.WindowLst[WindowIndex].WID == WindowIDsInTree[IDIndex])
                    {
                        Found = true;
                        break;
                    }
                }

                if(!Found)
                    RemoveWindowFromMonocleTree(Screen, WindowIDsInTree[IDIndex]);
            }
        }
        else
        {
            tree_node *WindowNode = GetNodeFromWindowID(Space->RootNode, WindowIDsInTree[0], SpaceModeMonocle);
            if(!WindowNode)
                return;

            free(WindowNode);
            Space->RootNode = NULL;
        }
    }
}

void AddWindowToMonocleTree(screen_info *Screen, int WindowID)
{
    if(!DoesSpaceExistInMapOfScreen(Screen))
        return;

    space_info *Space = GetActiveSpaceOfScreen(Screen);
    tree_node *CurrentNode = GetLastLeafNode(Space->RootNode);

    AddElementToTree(Screen, CurrentNode, WindowID, SplitModeUnset, SpaceModeMonocle);
}

void RemoveWindowFromMonocleTree(screen_info *Screen, int WindowID)
{
    if(!DoesSpaceExistInMapOfScreen(Screen))
        return;

    space_info *Space = GetActiveSpaceOfScreen(Screen);
    RemoveElementFromTree(Screen, Space->RootNode, WindowID, Space->Mode);

    SetWindowFocusByNode(GetFirstLeafNode(Space->RootNode));
    MoveCursorToCenterOfFocusedWindow();
}

void AddWindowToTreeOfUnfocusedMonitor(screen_info *Screen, window_info *Window)
{
    if(!Screen || !Window || Screen == GetDisplayOfWindow(Window))
        return;

    if(Window->WID == KWMScreen.MarkedWindow)
        ClearMarkedWindow();

    if(!IsSpaceInitializedForScreen(Screen))
    {
        CenterWindow(Screen, Window);
        Screen->ForceContainerUpdate = true;
        return;
    }

    space_info *Space = GetActiveSpaceOfScreen(Screen);
    if(Space->RootNode)
    {
        tree_node *CurrentNode = NULL;
        if(Space->Mode == SpaceModeBSP)
        {
            DEBUG("AddWindowToTreeOfUnfocusedMonitor() BSP Space")
            CurrentNode = LevelOrderSearch(IsLeafNode, Space->RootNode);
            Screen->ForceContainerUpdate = true;
        }
        else if(Space->Mode == SpaceModeMonocle)
        {
            DEBUG("AddWindowToTreeOfUnfocusedMonitor() Monocle Space")
            CurrentNode = GetLastLeafNode(Space->RootNode);
        }
        AddElementToTree(Screen, CurrentNode, Window->WID, KWMScreen.SplitMode, Space->Mode);
    }
    else
    {
        CenterWindow(Screen, Window);
        Screen->ForceContainerUpdate = true;
    }
}

void ToggleWindowFloating(int WindowID)
{
    if(IsWindowOnActiveSpace(WindowID) &&
       KWMScreen.Current->Space[KWMScreen.Current->ActiveSpace].Mode == SpaceModeBSP)
    {
        int WindowIndex;
        if(IsWindowFloating(WindowID, &WindowIndex))
        {
            KWMTiling.FloatingWindowLst.erase(KWMTiling.FloatingWindowLst.begin() + WindowIndex);
            AddWindowToBSPTree(KWMScreen.Current, WindowID);

            if(KWMMode.Focus != FocusModeDisabled && KWMMode.Focus != FocusModeAutofocus && KWMToggles.StandbyOnFloat)
                KWMMode.Focus = FocusModeAutoraise;
        }
        else
        {
            KWMTiling.FloatingWindowLst.push_back(WindowID);
            RemoveWindowFromBSPTree(KWMScreen.Current, WindowID, true);

            if(KWMMode.Focus != FocusModeDisabled && KWMMode.Focus != FocusModeAutofocus && KWMToggles.StandbyOnFloat)
                KWMMode.Focus = FocusModeStandby;
        }
    }
}

void ToggleFocusedWindowFloating()
{
    if(KWMFocus.Window)
        ToggleWindowFloating(KWMFocus.Window->WID);
}

void ToggleFocusedWindowParent()
{
    if(!KWMFocus.Window || !DoesSpaceExistInMapOfScreen(KWMScreen.Current))
        return;

    space_info *Space = GetActiveSpaceOfScreen(KWMScreen.Current);
    if(Space->Mode != SpaceModeBSP)
        return;

    if(ToggleElementInTree(KWMScreen.Current, Space->RootNode, KWMFocus.Window->WID, Space->Mode))
        UpdateBorder("focused");
}

void ToggleFocusedWindowFullscreen()
{
    if(!KWMFocus.Window || !DoesSpaceExistInMapOfScreen(KWMScreen.Current))
        return;

    space_info *Space = GetActiveSpaceOfScreen(KWMScreen.Current);
    if(ToggleElementInRoot(KWMScreen.Current, Space->RootNode, KWMFocus.Window->WID, Space->Mode))
            UpdateBorder("focused");
}

void DetachAndReinsertWindow(int WindowID, int Degrees)
{
    if(WindowID == KWMScreen.MarkedWindow)
    {
        int Marked = KWMScreen.MarkedWindow;
        if(Marked == -1 || (KWMFocus.Window && Marked == KWMFocus.Window->WID))
            return;

        ToggleWindowFloating(Marked);
        ClearMarkedWindow();
        ToggleWindowFloating(Marked);
        MoveCursorToCenterOfFocusedWindow();
    }
    else
    {
        if(WindowID == KWMScreen.MarkedWindow ||
           WindowID == -1)
            return;

        window_info InsertWindow = {};
        if(FindClosestWindow(Degrees, &InsertWindow, false))
        {
            ToggleWindowFloating(WindowID);
            KWMScreen.MarkedWindow = InsertWindow.WID;
            ToggleWindowFloating(WindowID);
            MoveCursorToCenterOfFocusedWindow();
        }
    }
}

void SwapFocusedWindowWithMarked()
{
    if(!KWMFocus.Window || KWMScreen.MarkedWindow == KWMFocus.Window->WID || KWMScreen.MarkedWindow == -1)
        return;

    if(DoesSpaceExistInMapOfScreen(KWMScreen.Current))
    {
        space_info *Space = GetActiveSpaceOfScreen(KWMScreen.Current);
        tree_node *FocusedWindowNode = GetNodeFromWindowID(Space->RootNode, KWMFocus.Window->WID, Space->Mode);
        if(FocusedWindowNode)
        {
            tree_node *NewFocusNode = GetNodeFromWindowID(Space->RootNode, KWMScreen.MarkedWindow, Space->Mode);
            if(NewFocusNode)
            {
                SwapNodeWindowIDs(FocusedWindowNode, NewFocusNode);
                MoveCursorToCenterOfFocusedWindow();
            }
        }
    }

    ClearMarkedWindow();
}

void SwapFocusedWindowWithNearest(int Shift)
{
    if(!KWMFocus.Window || !DoesSpaceExistInMapOfScreen(KWMScreen.Current))
        return;

    space_info *Space = GetActiveSpaceOfScreen(KWMScreen.Current);
    tree_node *FocusedWindowNode = GetNodeFromWindowID(Space->RootNode, KWMFocus.Window->WID, Space->Mode);
    if(FocusedWindowNode)
    {
        tree_node *NewFocusNode = NULL;;

        if(Shift == 1)
            NewFocusNode = GetNearestNodeToTheRight(FocusedWindowNode, Space->Mode);
        else if(Shift == -1)
            NewFocusNode = GetNearestNodeToTheLeft(FocusedWindowNode, Space->Mode);

        if(NewFocusNode)
        {
            SwapNodeWindowIDs(FocusedWindowNode, NewFocusNode);
            MoveCursorToCenterOfFocusedWindow();

            if(FocusedWindowNode->WindowID == KWMScreen.MarkedWindow ||
               NewFocusNode->WindowID == KWMScreen.MarkedWindow)
                   UpdateBorder("marked");
        }
    }
}

void SwapFocusedWindowDirected(int Degrees)
{
    if(!KWMFocus.Window || !DoesSpaceExistInMapOfScreen(KWMScreen.Current))
        return;

    space_info *Space = GetActiveSpaceOfScreen(KWMScreen.Current);
    tree_node *FocusedWindowNode = GetNodeFromWindowID(Space->RootNode, KWMFocus.Window->WID, Space->Mode);
    if(FocusedWindowNode)
    {
        tree_node *NewFocusNode = NULL;
        if(Space->Mode == SpaceModeBSP)
        {
            window_info SwapWindow = {};
            if(FindClosestWindow(Degrees, &SwapWindow, KWMMode.Cycle == CycleModeScreen))
                NewFocusNode = GetNodeFromWindowID(Space->RootNode, SwapWindow.WID, Space->Mode);
        }
        else if(Space->Mode == SpaceModeMonocle)
        {
            if(Degrees == 90)
                NewFocusNode = GetNearestNodeToTheRight(FocusedWindowNode, Space->Mode);
            else if(Degrees == 270)
                NewFocusNode = GetNearestNodeToTheLeft(FocusedWindowNode, Space->Mode);
        }

        if(NewFocusNode)
        {
            SwapNodeWindowIDs(FocusedWindowNode, NewFocusNode);
            MoveCursorToCenterOfFocusedWindow();

            if(FocusedWindowNode->WindowID == KWMScreen.MarkedWindow ||
               NewFocusNode->WindowID == KWMScreen.MarkedWindow)
                UpdateBorder("marked");
        }
    }
}

bool WindowIsInDirection(window_info *A, window_info *B, int Degrees, bool Wrap)
{
    if(Wrap)
    {
        if(Degrees == 0 || Degrees == 180)
            return A->Y != B->Y && fmax(A->X, B->X) < fmin(B->X + B->Width, A->X + A->Width);
        else if(Degrees == 90 || Degrees == 270)
            return A->X != B->X && fmax(A->Y, B->Y) < fmin(B->Y + B->Height, A->Y + A->Height);
    }
    else
    {
        if(Degrees == 0)
            return B->Y + B->Height < A->Y;
        else if(Degrees == 90)
            return B->X > A->X + A->Width;
        else if(Degrees == 180)
            return B->Y > A->Y + A->Height;
        else if(Degrees == 270)
            return B->X + B->Width < A->X;
    }

    return false;
}

void GetCenterOfWindow(window_info *Window, int *X, int *Y)
{
    *X = Window->X + Window->Width / 2;
    *Y = Window->Y + Window->Height / 2;
}

double GetWindowDistance(window_info *A, window_info *B)
{
    double Dist = INT_MAX;

    if(A && B)
    {
        int X1, Y1, X2, Y2;
        GetCenterOfWindow(A, &X1, &Y1);
        GetCenterOfWindow(B, &X2, &Y2);

        int ScoreX = X1 >= X2 - 15 && X1 <= X2 + 15 ? 1 : 11;
        int ScoreY = Y1 >= Y2 - 10 && Y1 <= Y2 + 10 ? 1 : 22;
        int Weight = ScoreX * ScoreY;
        Dist = std::sqrt(std::pow(X2-X1, 2) + std::pow(Y2-Y1, 2)) + Weight;
    }

    return Dist;
}

bool FindClosestWindow(int Degrees, window_info *Target, bool Wrap)
{
    *Target = KWMFocus.Cache;
    window_info *Match = KWMFocus.Window;
    std::vector<window_info> Windows = KWMTiling.WindowLst;

    int MatchX, MatchY;
    GetCenterOfWindow(Match, &MatchX, &MatchY);

    double MinDist = INT_MAX;
    for(int Index = 0; Index < Windows.size(); ++Index)
    {
        if(!WindowsAreEqual(Match, &Windows[Index]) &&
           WindowIsInDirection(Match, &Windows[Index], Degrees, Wrap))
        {
            window_info FocusWindow = Windows[Index];

            if(Wrap)
            {
                int WindowX, WindowY;
                GetCenterOfWindow(&Windows[Index], &WindowX, &WindowY);

                window_info WrappedWindow = Windows[Index];
                if(Degrees == 0 && MatchY < WindowY)
                    WrappedWindow.Y -= KWMScreen.Current->Height;
                else if(Degrees == 180 && MatchY > WindowY)
                    WrappedWindow.Y += KWMScreen.Current->Height;
                else if(Degrees == 90 && MatchX > WindowX)
                    WrappedWindow.X += KWMScreen.Current->Width;
                else if(Degrees == 270 && MatchX < WindowX)
                    WrappedWindow.X -= KWMScreen.Current->Width;

                FocusWindow = WrappedWindow;
            }

            double Dist = GetWindowDistance(Match, &FocusWindow);
            if(Dist < MinDist)
            {
                MinDist = Dist;
                *Target = Windows[Index];
            }
        }
    }

    return MinDist != INT_MAX;
}

void ShiftWindowFocusDirected(int Degrees)
{
    /* North = 0, East = 90, South = 180, West = 270 */
    if(!KWMFocus.Window || !DoesSpaceExistInMapOfScreen(KWMScreen.Current))
        return;

    space_info *Space = GetActiveSpaceOfScreen(KWMScreen.Current);
    if(Space->Mode == SpaceModeBSP)
    {
        window_info NewFocusWindow = {};
        if((KWMMode.Cycle == CycleModeDisabled &&
            FindClosestWindow(Degrees, &NewFocusWindow, false)) ||
           (KWMMode.Cycle == CycleModeScreen &&
            FindClosestWindow(Degrees, &NewFocusWindow, true)))
        {
            SetWindowFocus(&NewFocusWindow);
            MoveCursorToCenterOfFocusedWindow();
        }
        else if(KWMMode.Cycle == CycleModeAll)
        {
            if(FindClosestWindow(Degrees, &NewFocusWindow, true))
            {
                int ScreenIndex = KWMScreen.Current->ID;
                if(Degrees == 90 && WindowIsInDirection(KWMFocus.Window, &NewFocusWindow, 270, false))
                    ScreenIndex = GetIndexOfNextScreen();
                else if(Degrees == 270 && WindowIsInDirection(KWMFocus.Window, &NewFocusWindow, 90, false))
                    ScreenIndex = GetIndexOfPrevScreen();

                if(ScreenIndex == KWMScreen.Current->ID)
                {
                    SetWindowFocus(&NewFocusWindow);
                    MoveCursorToCenterOfFocusedWindow();
                }
                else
                {
                    screen_info *Screen = GetDisplayFromScreenID(ScreenIndex);
                    space_info *Space = GetActiveSpaceOfScreen(Screen);
                    tree_node *RootNode = Space->RootNode;
                    tree_node *FocusNode = Degrees == 90 ? GetFirstLeafNode(RootNode) : GetLastLeafNode(RootNode);
                    if(FocusNode)
                        GiveFocusToScreen(ScreenIndex, FocusNode, false);
                }
            }
        }
    }
    else if(Space->Mode == SpaceModeMonocle)
    {
        if(Degrees == 90)
            ShiftWindowFocus(1);
        else if(Degrees == 270)
            ShiftWindowFocus(-1);
    }
}

void ShiftWindowFocus(int Shift)
{
    if(!KWMFocus.Window || !DoesSpaceExistInMapOfScreen(KWMScreen.Current))
        return;

    space_info *Space = GetActiveSpaceOfScreen(KWMScreen.Current);
    tree_node *FocusedWindowNode = GetNodeFromWindowID(Space->RootNode, KWMFocus.Window->WID, Space->Mode);
    if(FocusedWindowNode)
    {
        tree_node *FocusNode = NULL;

        if(Shift == 1)
        {
            FocusNode = GetNearestNodeToTheRight(FocusedWindowNode, Space->Mode);
            if(KWMMode.Cycle == CycleModeScreen && !FocusNode)
            {
                FocusNode = GetFirstLeafNode(Space->RootNode);
            }
            else if(KWMMode.Cycle == CycleModeAll && !FocusNode)
            {
                int ScreenIndex = GetIndexOfNextScreen();
                screen_info *Screen = GetDisplayFromScreenID(ScreenIndex);
                space_info *Space = GetActiveSpaceOfScreen(Screen);
                FocusNode = GetFirstLeafNode(Space->RootNode);
                if(FocusNode)
                {
                    GiveFocusToScreen(ScreenIndex, FocusNode, false);
                    return;
                }
            }
        }
        else if(Shift == -1)
        {
            FocusNode = GetNearestNodeToTheLeft(FocusedWindowNode, Space->Mode);
            if(KWMMode.Cycle == CycleModeScreen && !FocusNode)
            {
                FocusNode = GetLastLeafNode(Space->RootNode);
            }
            else if(KWMMode.Cycle == CycleModeAll && !FocusNode)
            {
                int ScreenIndex = GetIndexOfPrevScreen();
                screen_info *Screen = GetDisplayFromScreenID(ScreenIndex);
                space_info *Space = GetActiveSpaceOfScreen(Screen);
                FocusNode = GetLastLeafNode(Space->RootNode);
                if(FocusNode)
                {
                    GiveFocusToScreen(ScreenIndex, FocusNode, false);
                    return;
                }
            }
        }

        SetWindowFocusByNode(FocusNode);
        MoveCursorToCenterOfFocusedWindow();
    }
}

void FocusWindowByID(int WindowID)
{
    window_info *Window = GetWindowByID(WindowID);
    if(Window)
    {
        screen_info *Screen = GetDisplayOfWindow(Window);
        if(Screen == KWMScreen.Current)
        {
            SetWindowFocus(Window);
            MoveCursorToCenterOfFocusedWindow();
        }

        if(Screen != KWMScreen.Current && IsSpaceInitializedForScreen(Screen))
        {
            space_info *Space = GetActiveSpaceOfScreen(Screen);
            tree_node *Root = Space->RootNode;
            tree_node *Node = GetNodeFromWindowID(Root, WindowID, Space->Mode);
            if(Node)
                GiveFocusToScreen(Screen->ID, Node, false);
        }
    }
}

void ClearMarkedWindow()
{
    KWMScreen.MarkedWindow = -1;
    ClearBorder(&MarkedBorder);
}

void MarkWindowContainer(window_info *Window)
{
    if(Window)
    {
        if(KWMScreen.MarkedWindow == Window->WID)
        {
            DEBUG("MarkWindowContainer() Unmarked " << Window->Name)
            ClearMarkedWindow();
        }
        else
        {
            DEBUG("MarkWindowContainer() Marked " << Window->Name)
            KWMScreen.MarkedWindow = Window->WID;
            UpdateBorder("marked");
        }
    }
}

void MarkFocusedWindowContainer()
{
    MarkWindowContainer(KWMFocus.Window);
}

void SetWindowFocusByNode(tree_node *Node)
{
    if(Node)
    {
        window_info *Window = GetWindowByID(Node->WindowID);
        if(Window)
        {
            DEBUG("SetWindowFocusByNode()")
            KWMScreen.ForceRefreshFocus = true;
            SetWindowFocus(Window);
            KWMScreen.ForceRefreshFocus = false;
        }
    }
}

CGPoint GetCursorPos()
{
    CGEventRef Event = CGEventCreate(NULL);
    CGPoint Cursor = CGEventGetLocation(Event);
    CFRelease(Event);

    return Cursor;
}

window_info *GetWindowByID(int WindowID)
{
    for(std::size_t WindowIndex = 0; WindowIndex < KWMTiling.FocusLst.size(); ++WindowIndex)
    {
        if(KWMTiling.FocusLst[WindowIndex].WID == WindowID)
            return &KWMTiling.FocusLst[WindowIndex];
    }

    return NULL;
}

bool GetWindowRole(window_info *Window, CFTypeRef *Role, CFTypeRef *SubRole)
{
    bool Result = false;

    std::map<int, window_role>::iterator It = KWMCache.WindowRole.find(Window->WID);
    if(It != KWMCache.WindowRole.end())
    {
        *Role = KWMCache.WindowRole[Window->WID].Role;
        *SubRole = KWMCache.WindowRole[Window->WID].SubRole;
        Result = true;
    }
    else
    {
        AXUIElementRef WindowRef;
        if(GetWindowRef(Window, &WindowRef))
        {
            AXUIElementCopyAttributeValue(WindowRef, kAXRoleAttribute, (CFTypeRef *)Role);
            AXUIElementCopyAttributeValue(WindowRef, kAXSubroleAttribute, (CFTypeRef *)SubRole);
            window_role RoleEntry = { *Role, *SubRole };
            KWMCache.WindowRole[Window->WID] = RoleEntry;
            Result = true;
        }
    }

    return Result;
}

void FreeWindowRefCache(int PID)
{
    std::map<int, std::vector<AXUIElementRef> >::iterator It = KWMCache.WindowRefs.find(PID);

    if(It != KWMCache.WindowRefs.end())
    {
        int NumElements = KWMCache.WindowRefs[PID].size();
        for(int RefIndex = 0; RefIndex < NumElements; ++RefIndex)
            CFRelease(KWMCache.WindowRefs[PID][RefIndex]);

        KWMCache.WindowRefs[PID].clear();
    }
}

void GetWindowInfo(const void *Key, const void *Value, void *Context)
{
    CFStringRef K = (CFStringRef)Key;
    std::string KeyStr = CFStringGetCStringPtr(K, kCFStringEncodingMacRoman);
    CFTypeID ID = CFGetTypeID(Value);
    if(ID == CFStringGetTypeID())
    {
        CFStringRef V = (CFStringRef)Value;
        std::string ValueStr = GetUTF8String(V);
        if(ValueStr.empty() && CFStringGetCStringPtr(V, kCFStringEncodingMacRoman))
            ValueStr = CFStringGetCStringPtr(V, kCFStringEncodingMacRoman);

        if(KeyStr == "kCGWindowName")
            KWMTiling.WindowLst[KWMTiling.WindowLst.size()-1].Name = ValueStr;
        else if(KeyStr == "kCGWindowOwnerName")
            KWMTiling.WindowLst[KWMTiling.WindowLst.size()-1].Owner = ValueStr;
    }
    else if(ID == CFNumberGetTypeID())
    {
        int MyInt;
        CFNumberRef V = (CFNumberRef)Value;
        CFNumberGetValue(V, kCFNumberSInt64Type, &MyInt);
        if(KeyStr == "kCGWindowNumber")
            KWMTiling.WindowLst[KWMTiling.WindowLst.size()-1].WID = MyInt;
        else if(KeyStr == "kCGWindowOwnerPID")
            KWMTiling.WindowLst[KWMTiling.WindowLst.size()-1].PID = MyInt;
        else if(KeyStr == "kCGWindowLayer")
            KWMTiling.WindowLst[KWMTiling.WindowLst.size()-1].Layer = MyInt;
        else if(KeyStr == "X")
            KWMTiling.WindowLst[KWMTiling.WindowLst.size()-1].X = MyInt;
        else if(KeyStr == "Y")
            KWMTiling.WindowLst[KWMTiling.WindowLst.size()-1].Y = MyInt;
        else if(KeyStr == "Width")
            KWMTiling.WindowLst[KWMTiling.WindowLst.size()-1].Width = MyInt;
        else if(KeyStr == "Height")
            KWMTiling.WindowLst[KWMTiling.WindowLst.size()-1].Height = MyInt;
    }
    else if(ID == CFDictionaryGetTypeID())
    {
        CFDictionaryRef Elem = (CFDictionaryRef)Value;
        CFDictionaryApplyFunction(Elem, GetWindowInfo, NULL);
    }
}

void ModifySubtreeSplitRatioFromWindow(const double &Offset)
{
    if(DoesSpaceExistInMapOfScreen(KWMScreen.Current))
    {
        space_info *Space = GetActiveSpaceOfScreen(KWMScreen.Current);
        tree_node *Root = Space->RootNode;

        if(IsLeafNode(Root) || Root->WindowID != -1)
            return;

        tree_node *Node = GetNodeFromWindowID(Root, KWMFocus.Window->WID, Space->Mode);
        ModifySubtreeSplitRatio(KWMScreen.Current, Node, Offset);
    }
}
