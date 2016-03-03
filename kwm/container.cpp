#include "container.h"
#include "window.h"    // containers hold windows; windows are the next abstraction level down
#include "windowref.h" // ResizeWindowToContainerSize

extern kwm_screen KWMScreen;

node_container LeftVerticalContainerSplit(const container_offset &Offset, const node_container &Container)
{
    node_container LeftContainer = Container;

    LeftContainer.Width = (Container.Width * Container.SplitRatio) - (Offset.VerticalGap / 2);

    return LeftContainer;
}

node_container RightVerticalContainerSplit(const container_offset &Offset, const node_container &Container)
{
    node_container RightContainer = Container;

    RightContainer.X = Container.X + (Container.Width * Container.SplitRatio) + (Offset.VerticalGap / 2);
    RightContainer.Width = (Container.Width * (1 - Container.SplitRatio)) - (Offset.VerticalGap / 2);

    return RightContainer;
}

node_container UpperHorizontalContainerSplit(const container_offset &Offset, const node_container &Container)
{
    node_container UpperContainer = Container;

    UpperContainer.Height = (Container.Height * Container.SplitRatio) - (Offset.HorizontalGap / 2);

    return UpperContainer;
}

node_container LowerHorizontalContainerSplit(const container_offset &Offset, const node_container &Container)
{
    node_container LowerContainer = Container;

    LowerContainer.Y = Container.Y + (Container.Height * Container.SplitRatio) + (Offset.HorizontalGap / 2);
    LowerContainer.Height = (Container.Height * (1 - Container.SplitRatio)) - (Offset.HorizontalGap / 2);

    return LowerContainer;
}

split_mode GetOptimalSplitMode(const node_container &Container)
{
    return (Container.Width / Container.Height) >= 1.618 ? SplitModeVertical : SplitModeHorizontal;
}

node_container CreateNodeContainer(const container_offset &Offset, const node_container &ParentContainer, const container_type &ContainerType)
{
    node_container Container;

    switch(ContainerType)
    {
        case ContainerLeft:
        {
            Container = LeftVerticalContainerSplit(Offset, ParentContainer);
        } break;
        case ContainerRight:
        {
            Container = RightVerticalContainerSplit(Offset, ParentContainer);
        } break;
        case ContainerUpper:
        {
            Container = UpperHorizontalContainerSplit(Offset, ParentContainer);
        } break;
        case ContainerLower:
        {
            Container = LowerHorizontalContainerSplit(Offset, ParentContainer);
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
void CreateNodeContainerPair(const container_offset &Offset, tree_node *Parent, const split_mode &SplitMode)
{
    Assert(Parent, "CreateNodeContainerPair() Parent")

    switch(SplitMode)
    {
        case SplitModeVertical:
        {
            Parent->LeftChild->Container = CreateNodeContainer(Offset, Parent->Container, ContainerLeft);
            Parent->RightChild->Container = CreateNodeContainer(Offset, Parent->Container, ContainerRight);
        } break;
        case SplitModeHorizontal:
        {
            Parent->LeftChild->Container = CreateNodeContainer(Offset, Parent->Container, ContainerUpper);
            Parent->RightChild->Container = CreateNodeContainer(Offset, Parent->Container, ContainerLower);
        } break;
        default:
        {
            DEBUG("CreateNodeContainerPair() Invalid SplitMode given: " << SplitMode)
        } break;

    }
}

void SetRootNodeContainer(const screen_info &Screen, const container_offset &Offset, node_container* Container)
{
    Assert(Container, "SetRootNodeContainer()")

    Container->Type       = ContainerRoot;
    Container->X          = Screen.X + Offset.PaddingLeft;
    Container->Y          = Screen.Y + Offset.PaddingTop;
    Container->Width      = Screen.Width - Offset.PaddingLeft - Offset.PaddingRight;
    Container->Height     = Screen.Height - Offset.PaddingTop - Offset.PaddingBottom;
    Container->SplitMode  = GetOptimalSplitMode(*Container);
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
