/* Functions that operate on Containers and their Elements */
#ifndef CONTAINER_H
#define CONTAINER_H

#include "types.h"

/* Change KWMScreen.SplitRatio (default split for new Containers)

    Map:
        Interpreter ~> Container

    Parameters:
        Value - the new SplitRatio to use ( 0 < Value < 1)

    Global References:
    [M] KWMScreen.SplitRatio

    Mutations:
        KWMScreen.SplitRatio is set to Value.

    Return:
        (void)

    Calling Functions:
        interpreter::KwmConfigCommand(..)
    
    Notes:
      - Only called by interpreter
      - Maybe move this out into an interpreter only file
*/
void ChangeSplitRatio(double Value);

/* Determine whether the Container should split Vertically or Horizontally

    Map:
        Boundary ~> Boundary

    Parameters:
        Rect - the rectangular dimensions (X, Y, Width, Height)

    Mutations:
        (none)

    Return:
        split_mode - SplitModeVertical or SplitModeHorizontal

    Called Functions::
        (none)

    Calling Functions:
        serialize :: FillDeserializedTree(...)
        node      :: CreateLeafNodePair(...)
        container :: CreateNodeContainer(...)
        container :: SetRootNodeContainer(..., Container)
    
    Notes:
      - Uses the Golden Ratio to determine which split is best.

    TODO: unlink serialize::FillDeserializedTree from this abstraction level
    TODO: Add in a global configuration for the default optimal split ratio
*/
split_mode GetOptimalSplitMode(const bound_rect &Rect);


////////////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS

/* Create a Split Container from an existing Container

    Map:
        Container ~> Container

    Parameters:
       Offset    - the gap offset for the New Container
       Container - Base Container for the New Container

    Mutations:
        (none)

    Return:
        node_container - a new Container based on the provided Container and Offset

    Called Functions:
        (none)

    Calling Functions:
        container :: CreateNodeContainer(Offset, ...)
*/
node_container LeftVerticalContainerSplit(const container_offset &Offset, const node_container &Container);
node_container RightVerticalContainerSplit(const container_offset &Offset, const node_container &Container);
node_container UpperHorizontalContainerSplit(const container_offset &Offset, const node_container &Container);
node_container LowerHorizontalContainerSplit(const container_offset &Offset, const node_container &Container);

/* Create a Split Child Container from a Parent Container

    Map:
        Container ~> Container

    Parameters:
        Offset          - the gap offset of the new Container
        ParentContainer - the enclosing Container to base the split Container on.
        ContainerType   - the type of split for the new Container

    Global References:
        KWMScreen.SplitRatio - get the default split ratio

    Mutations:
        (none)

    Return:
        node_container - the new Container that's split from the ParentContainer

    Called Functions:
        container :: LeftVerticalContainerSplit()
        container :: RightVerticalContainerSplit()
        container :: UpperHorizontalContainerSplit()
        container :: LowerHorizontalContainerSplit()
        container :: GetOptimalSplitMode()

    Calling Functions:
        node :: CreateLeafNode()
        node :: ResizeNodeContainer()

    Notes:

    TODO: Replace KWMScreen.SplitRatio with a KWM configuration value.
*/
node_container CreateSplitContainer(const container_offset &Offset, const node_container &ParentContainer, const container_type &ContainerType);

/* Create Container with default values for fullscreen Root Node

    Map:
        Container ~> Container

    Parameters:
        Boundary - the full boundary for the Container (i.e. a Space's Boundary)

    Global References:
        KWMScreen.SplitRatio - for default split ratio

    Mutations:
i       (none)

    Return:
        (void)

    Called Functions:
        container :: GetOptimalSplitMode()

    Calling Functions:
        display :: ChangePaddingOfDisplay()
        node    :: ResizeNodeContainer()
        node    :: CreateRootNode()
    
    Notes:
    
    TODO: Remove display::ChangePaddingOfDisplay(...) from calling functions
*/
node_container CreateRootContainer(const bound_rect &Boundary)

////////////////////////////////////////////////////////////////////////////////
// MUTATORS

/* Set the SplitRatio of the Container

    Map:
        Container ~> Container

    Parameters:
    [M] *Container - the Container whose SplitRatio is to be modified.
        Ratio - the new SplitRatio (0 < Delta < 1)

    Mutations:
        Container->SplitRatio = Ratio

    Return:
        (none)

    Called Functions:
        (none)

    Calling Functions: // TODO SetContainerSplitRatio()

    Notes:
      - This function will not fail to modify the SplitRatio if the result
        is not between 0 and 1, exclusive.
*/
void SetContainerSplitRatio(node_container *Container, const double &Ratio);

/* Adjust the SplitRatio of the Container by an amount

    Map:
        Container ~> Container

    Parameters:
    [M] *Container - the Container whose SplitRatio is to be modified.
        Delta - the amount to modify the SplitRatio (0 < Delta < 1)

    Mutations:
        Container->SplitRatio += Delta

    Return:
        (none)

    Called Functions:
        (none)

    Calling Functions:
        node :: ModifyNodeSplitRatio
    
    Notes:
      - This function will not fail to modify the SplitRatio if the result
        is not between 0 and 1, exclusive.
      - Usually the Windows will be updated after the containers are,
        but do we ever modify the split ratio without updating the containers?
        Should it happen in one step, or let the calling functions do it?
      - i.e. call ResizeElementInContainer in this function.
        -> I'd rather leave it up to a calling function, since we can modify
           a bunch of containers and then multithreaded resize.
    
    TODO: performance testing for ResizeElementInContainer.
*/
void AdjustContainerSplitRatio(node_container *Container, const double &Delta);

/* Toggle the SplitMode of the Container between SplitModeVertical and SplitModeHorizontal

    Map:
        Container ~> Container

    Parameters:
    [M] *Container - the Container to toggle the SplitMode of.

    Mutations:
        Container->SplitMode = SplitModeVertical OR SplitModeHorizontal

    Return:
        (void)

    Called Functions:
        (none)

    Calling Functions:
        node :: ToggleNodeSplitMode(..)
    
    Notes:
      - This function can FAIL if Container->SplitMode isn't already either
        SplitModeVertical or SplitModeHorizontal
      - Need to call ResizeTreeNodes and ApplyNodeContainer on Subtree after 
        changing the SplitMode to see the effect.
*/
void ToggleContainerSplitMode(node_container *Container);

/* (DEPRECIATE) Resize the WindowID's Window to fit in the Container.
    Map:
        Container ~> Window

    Parameters:
       WindowID - the WID of the Window to resize.
       Container - the dimensions to which the Window will be resized.

    Mutations:
        (see ResizeWindowToContainerSize)

    Return:
        (void)

    Called Functions:
        window     :: GetWindowByID(WindowID)
        dispatcher :: ResizeWindowToContainerSize(.., Container)

    Calling Functions:
        node :: ResizeElementInNode(..)
    
    Notes:
      - Breaks abstraction barrier by using the window::GetWindowByID function
    
    TODO: Make the Container argument a const ref (unmutated)
    TODO: Update this function once we switch to Element/Container inside Node
          -> Probably won't be needed. In Element, get Window and Container.Boundary
*/
void ResizeElementInContainer(const int &WindowID, node_container *Container);

#endif
