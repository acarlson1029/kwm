#include "container.h"
#include "window.h"    // containers hold windows; windows are the next abstraction level down
#include "windowref.h" // ResizeWindowToContainerSize

extern kwm_screen KWMScreen; // for KWMScreen.SplitRatio config setting

node_container LeftVerticalContainerSplit(const container_offset &Offset, const node_container &Container)
{
    node_container LeftContainer = Container;

    LeftContainer.Boundary.Width = (Container.Boundary.Width * Container.SplitRatio) - (Offset.VerticalGap / 2);

    return LeftContainer;
}

node_container RightVerticalContainerSplit(const container_offset &Offset, const node_container &Container)
{
    node_container RightContainer = Container;

    RightContainer.Boundary.X    += (Container.Boundary.Width * Container.SplitRatio) + (Offset.VerticalGap / 2);
    RightContainer.Boundary.Width = (Container.Boundary.Width * (1 - Container.SplitRatio)) - (Offset.VerticalGap / 2);

    return RightContainer;
}

node_container UpperHorizontalContainerSplit(const container_offset &Offset, const node_container &Container)
{
    node_container UpperContainer = Container;

    UpperContainer.Boundary.Height = (Container.Boundary.Height * Container.SplitRatio) - (Offset.HorizontalGap / 2);

    return UpperContainer;
}

node_container LowerHorizontalContainerSplit(const container_offset &Offset, const node_container &Container)
{
    node_container LowerContainer = Container;

    LowerContainer.Boundary.Y     += (Container.Boundary.Height * Container.SplitRatio) + (Offset.HorizontalGap / 2);
    LowerContainer.Boundary.Height = (Container.Boundary.Height * (1 - Container.SplitRatio)) - (Offset.HorizontalGap / 2);

    return LowerContainer;
}

split_mode GetOptimalSplitMode(const node_container &Container)
{
    return (Container.Boundary.Width / Container.Boundary.Height) >= 1.618 ? SplitModeVertical : SplitModeHorizontal;
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

void SetRootNodeContainer(const bound_rect &SpaceBoundary, node_container* Container)
{
    Assert(Container, "SetRootNodeContainer()")

    Container->Type = ContainerRoot;
    Container->Boundary = SpaceBoundary;
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

bool ModifyContainerSplitRatio(node_container *Container, const double &Delta)
{
    if(Container->SplitRatio + Delta <= 0.0 ||
       Container->SplitRatio + Delta >= 1.0)
        return false;

    Container->SplitRatio += Delta;
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
