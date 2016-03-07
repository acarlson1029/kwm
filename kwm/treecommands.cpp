#include "treecommands.h"
#include "tree.h"

window_info *GetWindowByIDOnSpace(const space_info &Space, const &WindowID)
{
    tree_node *Node = GetNodeFromWindowID(Space->Root, WindowID, Space->Mode);
    window_info *Window = GetWindowByID(Node->WindowID);
    return Window;
}

window_info *GetFirstWindowOnSpace(const space_info &Space)
{
    tree_node *Node = GetFirstLeafNode(Space->Root);
    window_info *Window = GetWindowByID(Node->WindowID);
    return Window;
}

window_info *GetLastWindowOnSpace(const space_info &Space)
{
    tree_node *Node = GetLastLeafNode(Space->Root);
    window_info *Window = GetWindowByID(Node->WindowID);
    return Window;
}

window_info *GetPreviousWindowOnSpace(const space_info &Space)
{
    tree_node *Node = GetNearestNodeToTheLeft(Space->FocusedNode, Space->Mode);
    window_info *Window = GetWindowByID(Node->WindowID);
    return Window;
}

window_info *GetNextWindowOnSpace(const space_info &Space)
{
    tree_node *Node = GetNearestNodeToTheRight(Space->FocusedNode, Space->Mode);
    window_info *Window = GetWindowByID(Node->WindowID);
    return Window;
}

window_info *GetRelativeWindowOnSpace(const space_info &Space, const int &Degrees) // TODO use enum instead of degrees
{
    window_info *Window;
    FindClosestWindow(Degrees, Window, true); // TODO third argument "Wrap" should be a KWM config option
    return Window;
}

window_info *GetWindowAtPositionOnSpace(const space_info &Space, const double &X, const double &Y) // TODO use CGPoint?
{
    tree_node *Node = GetNodeAtPosition(Space, X, Y);
    window_info *Window = GetWindowByID(Node->WindowID);
    return Window;
}

window_info *GetWindowOnSpaceFromArg(space_info *Space, const window_mux &Arg)
{
    if(!Space->Initialized)
        return;

    tree_node *Node = NULL;
    switch(Arg)
    {
        case first:
            Window = GetFirstWindowOnSpace(Space);
            break;
        case last:
            Window = GetLastWindowOnSpace(Space);
            break;
        case previous:
            Window = GetPreviousWindowOnSpace(Space);
            break;
        case next:
            Window = GetNextWindowOnSpace(Space);
            break;
        case above:
            Window = GetRelativeWindowOnSpace(Space, 0);
            break;
        case right:
            Window = GetRelativeWindowOnSpace(Space, 90);
            break;
        case below:
            Window = GetRelativeWindowOnSpace(Space, 180);
            break;
        case left:
            Window = GetRelativeWindowOnSpace(Space, 270);
            break;
        case focused:
            Window = Space->FocusedWindow;
            break;
        case marked:
            Window = Space->MarkedWindow;
            break;
        case current:
            Window = Space->CurrentWindow;
            break;
        case alternate:
            Window = Space->AlternateWindow;
            break;
        default:
            Assert(false, "FocusWindowOnSpace() Invalid argument")
            break;
    }
    return Window;
}

void SetWindowFocus(space_info *Space, const window_info &Window)
{
    Space->AlternateWindow = Space->CurrentWindow;
    Space->CurrentWindow = &Window;
    Space->FocusedWindow = &Window;

    // TODO Some FocusWindow callback? Draw Border or something?
}

void FocusWindowOnSpace(space_info *Space, const window_mux &Arg)
{
    window_info *Window = GetWindowOnSpaceFromArg(Space, Arg);
    if(Window)
        SetWindowFocus(Space, *Window);
}

void FocusWindowOnSpace(space_info *Space, const int &WindowID)
{
    if(!Space->Initialized)
        return;

    window_info *Window = GetWindowByIDOnSpace(Space, WindowID);

    if(Window)
        SetWindowFocus(Space, *Window);
}

void SetWindowMark(space_info *Space, const window_info &Window)
{
    Space->MarkedWindow = &Window;

    // TODO Some MarkWindow callbacks
}

void MarkWindowOnSpace(space_info *Space, const window_mux &Arg)
{
    window_info *Window = GetWindowOnSpaceFromArg(Space, Arg);
    if(Window)
        SetWindowMark(Space, *Window);
}

void MarkWindowOnSpace(space_info *Space, const int &WindowID);
{
    if(!Space->Initialized)
        return;

    window_info *Window = GetWindowByIDOnSpace(Space, WindowID);

    if(Window)
        SetWindowMark(Space, *Window);
}

void ZoomWindow(window_info *Window, const zoom_type &ZoomType)
{
    node_container *ZoomContainer;
    switch(ZoomType)
    {
        case ZoomParent:
            // This is probably wasteful -- we're finding the Node to get the Window
            // TODO break off the huge GetWindow switch into GetNode, and just extract the Window after.
            // For Focus/Mark/Current/Alternate, do this lookup
            tree_node *Node = GetNodeFromWindowID(Window->WID)
            ZoomContainer = Node->Parent.Container;
            break;
        case ZoomFullscreen:
            ZoomContainer = Space->RootNode.Container;
            break;
        case ZoomLeft:
            ZoomContainer = LeftVerticalContainerSplit(Space->Offset, Space->RootNode.Container);
            break;
        case ZoomRight:
            ZoomContainer = RightVerticalContainerSplit(Space->Offset, Space->RootNode.Container);
            break;
        case ZoomTop:
            ZoomContainer = UpperHorizontalContainerSplit(Space->Offset, Space->RootNode.Container);
            break;
        case ZoomBottom:
            ZoomContainer = LowerHorizontalContainerSplit(Space->Offset, Space->RootNode.Container);
            break;
        case ZoomTopLeft:
            ZoomContainer = UpperHorizontalContainerSplit(Space->Offset, ZoomContainer);
            ZoomContainer = LeftVerticalContainerSplit(Space->Offset, Space->RootNode.Container);
            break;
        case ZoomTopRight:
            ZoomContainer = UpperHorizontalContainerSplit(Space->Offset, ZoomContainer);
            ZoomContainer = RightVerticalContainerSplit(Space->Offset, Space->RootNode.Container);
            break;
        case ZoomBottomLeft:
            ZoomContainer = LowerHorizontalContainerSplit(Space->Offset, Space->RootNode.Container);
            ZoomContainer = LeftVerticalContainerSplit(Space->Offset, Space->RootNode.Container);
            break;
        case ZoomBottomRight:
            ZoomContainer = LowerHorizontalContainerSplit(Space->Offset, Space->RootNode.Container);
            ZoomContainer = RightVerticalContainerSplit(Space->Offset, Space->RootNode.Container);
            break;
        default:
            Assert(false, "ToggleWindowZoom() Invalid ZoomType receieved")
            break;
    }

    ResizeWindowtoContainerSize(Window, ZoomContainer);
}

void ToggleWindowZoom(space_info *Space, const window_mux &Arg, const zoom_type &ZoomType)
{
    window_info *Window = GetWindowOnSpaceFromArg(Space, Arg);
    if(!Window)
        return;

    if(!Window.ZoomToggle)
    {
        ZoomWindow(Window, ZoomType);
        Window->ZoomToggle = true;
    }
    else
    {
        Window->ZoomToggle = false;
        tree_node *Node = GetNodeFromWindowID(Window->WID)
        ResizeElementInNode(Node);
    }
}

void ToggleWindowZoom(space_info *Space, const window_mux &Arg, const bound_rect &Boundary)
{
    window_info *Window = GetWindowOnSpaceFromArg(Space, Arg);
    if(!Window)
        return;
    
    node_container *ZoomContainer = node_container();
    if(!Window.ZoomToggle)
    {
        ZoomContainer.Boundary = Boundary;
        ResizeWindowtoContainerSize(Window, ZoomContainer);
        Window->ZoomToggle = true;
    }
    else
    {
        Window->ZoomToggle = false;
        tree_node *Node = GetNodeFromWindowID(Window->WID)
        ResizeElementInNode(Node);
    }
}

void ToggleWindowZoom(space_info *Space, const int &WindowID, const zoom_type &ZoomType)
{
    if(!Space->Initialized)
        return;

    window_info *Window = GetWindowByIDOnSpace(Space, WindowID);

    if(!Window)
        return;

    if(!Window.ZoomToggle)
    {
        ZoomWindow(Window, ZoomType);
        Window->ZoomToggle = true;
    }
    else
    {
        Window->ZoomToggle = false;
        tree_node *Node = GetNodeFromWindowID(Window->WID)
        ResizeElementInNode(Node);
    }
}

void ToggleWindowZoom(space_info *Space, const int &WindowID, const bound_rect &Boundary)
{
    // TODO Should this guard be here, or should it be handled by calling functions?
    if(!Space->Initialized)
        return;

    window_info *Window = GetWindowByIDOnSpace(Space, WindowID);
    if(!Window)
        return;
    
    node_container *ZoomContainer = node_container();
    if(!Window.ZoomToggle)
    {
        ZoomContainer.Boundary = Boundary;
        ResizeWindowtoContainerSize(Window, ZoomContainer);
        Window->ZoomToggle = true;
    }
    else
    {
        Window->ZoomToggle = false;
        tree_node *Node = GetNodeFromWindowID(Window->WID)
        ResizeElementInNode(Node);
    }
}

void ToggleWindowFloat(space_info *Space, const window_mux &Arg)
{
    window_info *Window = GetWindowOnSpaceFromArg(Space, Arg);
    if(!Window)
        return;

    if(!Window->FloatToggle)
    {
        Window->FloatToggle = true;
    }
    else
    {
        Window->FloatToggle = false;
        tree_node *Node = GetNodeFromWindowID(Window->WID)
        ResizeElementInNode(Node);
    }
}

void ToggleWindowFloat(space_info *Space, const int &WindowID);
{
    if(!Space->Initialized)
        return;

    window_info *Window = GetWindowByIDOnSpace(Space, WindowID);
    if(!Window)
        return;

    if(!Window->FloaToggle)
    {
        Window->FloatToggle = true;
    }
    else
    {
        Window->FloatToggle = false;
        tree_node *Node = GetNodeFromWindowID(Window->WID)
        ResizeElementInNode(Node);
    }
}
