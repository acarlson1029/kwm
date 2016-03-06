#include "windowtree.h" // SetWindowFocusByNode, AddWindowToBSPTree, ResizeElementInTree
#include "windowref.h" // MoveCursorToCenterOfFocusedWindow, SetWindowFocus
#include "window.h" // GetWindowByID, FindClosestWindow, WindowIsInDirection, ClearMarkedWindow, IsWindowOnActiveSpace, IsWindowFloating,
#include "tree.h"
#include "space.h" // TODO DoesSpaceExistInMapOfScreen, GetActiveSpaceOfScreen 
#include "display.h"
#include "node.h" // SwapNodeWindowIDs
#include "border.h" //UpdateBorder
#include "dispatcher.h"

// TODO See which externs we don't need.
extern kwm_screen KWMScreen;
extern kwm_focus KWMFocus;
extern kwm_mode KWMMode;
extern kwm_toggles KWMToggles;
extern kwm_tiling KWMTiling;

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

    if(ToggleElementInTree(Space->Boundary, Space->RootNode, KWMFocus.Window->WID, Space->Mode, Space->Offset))
        UpdateBorder("focused");
}

void ToggleFocusedWindowFullscreen()
{
    if(!KWMFocus.Window || !DoesSpaceExistInMapOfScreen(KWMScreen.Current))
        return;

    space_info *Space = GetActiveSpaceOfScreen(KWMScreen.Current);
    if(ToggleElementInRoot(Space->Boundary, Space->RootNode, KWMFocus.Window->WID, Space->Mode, Space->Offset))
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
        ModifySubtreeSplitRatio(Space->Boundary, Node, Offset, Space->Offset, Space->Mode);
    }
}

void ResizeWindowToContainerSize()
{
    if(KWMFocus.Window)
        ResizeElementInTree(KWMScreen.Current, KWMFocus.Window);
}


