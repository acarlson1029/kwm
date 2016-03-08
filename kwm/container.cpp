#include "container.h"
#include "window.h"    // containers hold windows; windows are the next abstraction level down
#include "windowref.h" // ResizeWindowToContainerSize

// TODO Move out of here to some configuration file
void ChangeSplitRatio(double Value)
{
    if(Value > 0.0 && Value < 1.0)
    {
        DEBUG("ChangeSplitRatio() New Split-Ratio is " << Value)
        KWMScreen.SplitRatio = Value;
    }
}

split_mode GetOptimalSplitMode(const bound_rect &Rect)
{
    // TODO: Replace with a global config value for the optimal ratio
    return (Rect.Width / Rect.Height) >= 1.618 ? SplitModeVertical : SplitModeHorizontal;
}

////////////////////////////////////////////////////////////////////////////////
// GLOBAL VARIABLES
extern kwm_screen KWMScreen; // for KWMScreen.SplitRatio config setting

////////////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
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

node_container CreateSplitContainer(const container_offset &Offset, const node_container &ParentContainer, const container_type &ContainerType)
{
    node_container Container;

    switch(ContainerType)
    {
        case ContainerLeft:
            Container = LeftVerticalContainerSplit(Offset, ParentContainer);
            break;
        case ContainerRight:
            Container = RightVerticalContainerSplit(Offset, ParentContainer);
            break;
        case ContainerUpper:
            Container = UpperHorizontalContainerSplit(Offset, ParentContainer);
            break;
        case ContainerLower:
            Container = LowerHorizontalContainerSplit(Offset, ParentContainer);
            break;
        case ContainerRoot:
        default:
            Assert(false, "CreateNodeContainer() Trying to create node_container for invalid container_type: " << ContainerType)
            break;
    }

    if(Container.SplitRatio <= 0.0 || 1.0 <= Container.SplitRatio)
    {
        DEBUG("CreateNodeContainer() Invalid SplitRatio inherited; Setting to default")
        Container.SplitRatio = KWMScreen.SplitRatio;
    }

    Container.SplitMode = GetOptimalSplitMode(Container);
    Container.Type = ContainerType;

    return Container;
}

node_container CreateRootContainer(const bound_rect &Boundary)
{
    node_container Container;
    Container.Type = ContainerRoot;
    Container.Boundary = Boundary;
    Container.SplitMode = GetOptimalSplitMode(Boundary);
    Container.SplitRatio = KWMScreen.SplitRatio; // TODO update this global
}

////////////////////////////////////////////////////////////////////////////////
// MUTATORS
//   Note - can "set" attrubtes of a Container explicitly
//          "set" functions will have validity checking

void SetContainerSplitRatio(node_container *Container, const double &Ratio)
{
    if(0.0 < Ratio && Ratio < 1.0)
        Container->SplitRatio = Ratio;
    else
        DEBUG("SetContainerSplitRatio() Ratio " << Ratio << " is not between 0.0 and 1.0")
}

void AdjustContainerSplitRatio(node_container *Container, const double &Delta)
{
    double NewRatio = Container->SplitRatio + Delta;

    if(0.0 < NewRatio && NewRatio < 1.0)
        Container->SplitRatio = NewRatio;
    else
        DEBUG("AdjustContainerSplitRatio() New SplitRatio " << NewRatio  << " is not between 0.0 and 1.0")
}

void ToggleContainerSplitMode(node_container *Container)
{
    if(Container->SplitMode == SplitModeVertical)
        Container->SplitMode = SplitModeHorizontal;
    else if(Container->SplitMode == SplitModeHorizontal)
        Container->SplitMode = SplitModeVertical;
    else
        DEBUG("ToggleContainerSplitMode() Can't toggle SplitMode " << Container->SplitMode)
}

// TODO Update once we switch to the Element/Container inside Node scheme
void ResizeElementInContainer(const int &WindowID, node_container *Container)
{
    window_info *Window = GetWindowByID(WindowID);
    ResizeWindowToContainerSize(Window, Container);
}
