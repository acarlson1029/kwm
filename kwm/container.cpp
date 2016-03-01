#include "container.h"
#include "windowref.h" // used for ResizeWindowToContainerSize
#include "space.h"  // Used for the GetActiveSpaceOfScreen functions call; can do this in node.cpp and pass the Space?
#include "window.h" // containers hold windows; windows are the next abstraction level down

extern kwm_screen KWMScreen;

node_container LeftVerticalContainerSplit(screen_info *Screen, const node_container &Container)
{
    space_info *Space = GetActiveSpaceOfScreen(Screen);

    node_container LeftContainer = Container;

    LeftContainer.Width = (Container.Width * Container.SplitRatio) - (Space->Offset.VerticalGap / 2);

    return LeftContainer;
}

node_container RightVerticalContainerSplit(screen_info *Screen, const node_container &Container)
{
    space_info *Space = GetActiveSpaceOfScreen(Screen);
    node_container RightContainer = Container;

    RightContainer.X = Container.X + (Container.Width * Container.SplitRatio) + (Space->Offset.VerticalGap / 2);
    RightContainer.Width = (Container.Width * (1 - Container.SplitRatio)) - (Space->Offset.VerticalGap / 2);

    return RightContainer;
}

node_container UpperHorizontalContainerSplit(screen_info *Screen, const node_container &Container)
{
    space_info *Space = GetActiveSpaceOfScreen(Screen);
    node_container UpperContainer = Container;

    UpperContainer.Height = (Container.Height * Container.SplitRatio) - (Space->Offset.HorizontalGap / 2);

    return UpperContainer;
}

node_container LowerHorizontalContainerSplit(screen_info *Screen, const node_container &Container)
{
    space_info *Space = GetActiveSpaceOfScreen(Screen);
    node_container LowerContainer = Container;

    LowerContainer.Y = Container.Y + (Container.Height * Container.SplitRatio) + (Space->Offset.HorizontalGap / 2);
    LowerContainer.Height = (Container.Height * (1 - Container.SplitRatio)) - (Space->Offset.HorizontalGap / 2);

    return LowerContainer;
}

split_mode GetOptimalSplitMode(const node_container &Container)
{
    return (Container.Width / Container.Height) >= 1.618 ? SplitModeVertical : SplitModeHorizontal;
}

node_container CreateNodeContainer(screen_info *Screen, const node_container &ParentContainer, const container_type &ContainerType)
{
    node_container Container;

    switch(ContainerType)
    {
        case ContainerLeft:
        {
            Container = LeftVerticalContainerSplit(Screen, ParentContainer);
        } break;
        case ContainerRight:
        {
            Container = RightVerticalContainerSplit(Screen, ParentContainer);
        } break;
        case ContainerUpper:
        {
            Container = UpperHorizontalContainerSplit(Screen, ParentContainer);
        } break;
        case ContainerLower:
        {
            Container = LowerHorizontalContainerSplit(Screen, ParentContainer);
        } break;
        case ContainerRoot:
        default:
        {
            DEBUG("CreateNodeContainer() Trying to create node_container for invalid container_type: " << ContainerType)
        } break;
    }

    // FIXME  - (acarlson 02/28/16): Not sure if SplitRatio is correct -- should it be coming from a source already?
    if(Container.SplitRatio == 0)
        Container.SplitRatio = KWMScreen.SplitRatio;

    Container.SplitMode = GetOptimalSplitMode(Container);
    Container.Type = ContainerType;

    return Container;
}

// TODO -- can move the node logic up into node.cpp
void CreateNodeContainerPair(screen_info *Screen, tree_node *Parent, const split_mode &SplitMode)
{
    Assert(Parent, "CreateNodeContainerPair() Parent")

    switch(SplitMode)
    {
        case SplitModeVertical:
        {
            Parent->LeftChild->Container = CreateNodeContainer(Screen, Parent->Container, ContainerLeft);
            Parent->RightChild->Container = CreateNodeContainer(Screen, Parent->Container, ContainerRight);
        } break;
        case SplitModeHorizontal:
        {
            Parent->LeftChild->Container = CreateNodeContainer(Screen, Parent->Container, ContainerUpper);
            Parent->RightChild->Container = CreateNodeContainer(Screen, Parent->Container, ContainerLower);
        } break;
        default:
        {
            DEBUG("CreateNodeContainerPair() Invalid SplitMode given: " << SplitMode)
        } break;

    }
}

void SetRootNodeContainer(screen_info *Screen, node_container* Container)
{
    Assert(Container, "SetRootNodeContainer()")

    space_info *Space = GetActiveSpaceOfScreen(Screen);

    Container->Type = ContainerRoot;
    Container->X = Screen->X + Space->Offset.PaddingLeft;
    Container->Y = Screen->Y + Space->Offset.PaddingTop;
    Container->Width = Screen->Width - Space->Offset.PaddingLeft - Space->Offset.PaddingRight;
    Container->Height = Screen->Height - Space->Offset.PaddingTop - Space->Offset.PaddingBottom;
    Container->SplitMode = GetOptimalSplitMode(*Container);
    Container->SplitRatio = KWMScreen.SplitRatio;
}

void ChangeSplitRatio(double Value)
{
    if(Value > 0.0 && Value < 1.0)
    {
        DEBUG("ChangeSplitRatio() New Split-Ratio is " << Value)
        KWMScreen.SplitRatio = Value;
    }
}

void ResizeContainer(screen_info *Screen, node_container *Container)
{
    *Container = CreateNodeContainer(Screen, *Container, Container->Type);
}

bool ModifyContainerSplitRatio(node_container *Container, const double &Offset)
{
    // TODO Define these as MAX OFFSET and MIN OFFSET somewhere in types.h
    if(Container->SplitRatio + Offset <= 0.0 ||
       Container->SplitRatio + Offset >= 1.0)
        return false;

    Container->SplitRatio += Offset;
    return true;
}

void ToggleContainerSplitMode(node_container *Container)
{
    Container->SplitMode = Container->SplitMode == SplitModeVertical ? SplitModeHorizontal : SplitModeVertical;
}

void ResizeElementInContainer(const int &WindowID, node_container *Container)
{
    window_info *Window = GetWindowByID(WindowID);
    ResizeWindowToContainerSize(Window, Container);
}
