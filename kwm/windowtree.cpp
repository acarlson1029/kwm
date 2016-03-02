#include "windowtree.h"
#include "windowref.h"
#include "window.h"
#include "tree.h"
#include "space.h"
#include "display.h"
#include "application.h"
#include "node.h"
#include "border.h"

extern kwm_screen KWMScreen;
extern kwm_focus KWMFocus;
extern kwm_mode KWMMode;
extern kwm_toggles KWMToggles;
extern kwm_tiling KWMTiling;

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

void AddWindowToMonocleTree(screen_info *Screen, int WindowID)
{
    if(!DoesSpaceExistInMapOfScreen(Screen))
        return;

    space_info *Space = GetActiveSpaceOfScreen(Screen);
    tree_node *CurrentNode = GetLastLeafNode(Space->RootNode);

    AddElementToTree(Screen, CurrentNode, WindowID, SplitModeUnset, SpaceModeMonocle);
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

void RemoveWindowFromMonocleTree(screen_info *Screen, int WindowID)
{
    if(!DoesSpaceExistInMapOfScreen(Screen))
        return;

    space_info *Space = GetActiveSpaceOfScreen(Screen);
    RemoveElementFromTree(Screen, Space->RootNode, WindowID, Space->Mode);

    SetWindowFocusByNode(GetFirstLeafNode(Space->RootNode));
    MoveCursorToCenterOfFocusedWindow();
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

void ResizeElementInTree(screen_info *Screen, window_info *Window)
{
    Assert(Window, "ResizeElementInTree()")
    if(DoesSpaceExistInMapOfScreen(Screen))
    {
        space_info *Space = GetActiveSpaceOfScreen(Screen);
        tree_node *Node = GetNodeFromWindowID(Space->RootNode, Window->WID, Space->Mode);
        if(Node)
            ResizeElementInNode(Node);
    }
}

void ResizeWindowToContainerSize()
{
    if(KWMFocus.Window)
        ResizeElementInTree(KWMScreen.Current, KWMFocus.Window);
}


