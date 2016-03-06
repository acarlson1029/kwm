#include "windowtree.h"
#include "windowref.h"
#include "window.h"
#include "tree.h"
#include "space.h"
#include "display.h"
#include "application.h" // IsApplicationFloating
#include "node.h" // IsLeafNode, SwapNodeWindowIDs, ResizeElementInNode
#include "border.h" //UpdateBorder

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
        if(!IsSpaceFloating(Screen->ActiveSpace))
            Space->RootNode = CreateTreeFromWindowIDList(Space->Boundary, Space->Offset, *Windows, Space->Mode);
        else
            Space->RootNode = NULL;
    }
    else if(Space->Initialized)
    {
        if(!IsSpaceFloating(Screen->ActiveSpace))
            Space->RootNode = CreateTreeFromWindowIDList(Space->Boundary, Space->Offset, *Windows, Space->Mode);
        else
            Space->RootNode = NULL;
        ResizeTreeNodes(Space->Boundary, Space->Offset, Space->RootNode);
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

        // TODO Replace this loop with a regular leaf traversal function.
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

        // TODO -- Replace this with a traversal function
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
        CurrentNode = GetNodeFromWindowID(RootNode, KWMFocus.Window->WID, Space->Mode); // TODO: change Space->Mode to SpaceModeBSP
    }
    else if(DoNotUseMarkedContainer || (KWMScreen.MarkedWindow == -1 && !UseFocusedContainer))
    {
        // Get the first LeafNode via level-order search
        CurrentNode = LevelOrderSearch(IsLeafNode, RootNode);
    }
    else
    {
        CurrentNode = GetNodeFromWindowID(RootNode, KWMScreen.MarkedWindow, Space->Mode); // TODO: change Space->Mode to SpaceModeBSP
        ClearMarkedWindow();
    }
    AddElementToTree(Space->Boundary, Space->Offset, CurrentNode, WindowID, KWMScreen.SplitMode, SpaceModeBSP);
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

    AddElementToTree(Space->Boundary, Space->Offset, CurrentNode, WindowID, SplitModeUnset, SpaceModeMonocle);
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
        AddElementToTree(Space->Boundary, Space->Offset, CurrentNode, Window->WID, KWMScreen.SplitMode, Space->Mode);
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
    RemoveElementFromTree(Space->Boundary, Space, Space->RootNode, WindowID, SpaceModeBSP);

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
    RemoveElementFromTree(Space->Boundary, Space, Space->RootNode, WindowID, SpaceModeMonocle);

    SetWindowFocusByNode(GetFirstLeafNode(Space->RootNode));
    MoveCursorToCenterOfFocusedWindow();
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
