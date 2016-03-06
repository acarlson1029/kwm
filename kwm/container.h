/* Functions that operate on Containers and their Elements */
#ifndef CONTAINER_H
#define CONTAINER_H

#include "types.h"

/* Determine whether the Container should split Vertically or Horizontally

    Map:
        Container ~> Container

    Parameters:
        Container - the Container with dimensions to look at.

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
*/
split_mode GetOptimalSplitMode(const node_container &Container);

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
        node_container - the new Container Split from the ParentContainer

    Called Functions:
        container :: LeftVerticalContainerSplit(Offset, ParentContainer)
        container :: RightVerticalContainerSplit(Offset, ParentContainer)
        container :: UpperHorizontalContainerSplit(Offset, ParentContainer)
        container :: LowerHorizontalContainerSplit(Offset, ParentContainer)
        container :: GetOptimalSplitMode(..)

    Calling Functions:
        node      :: CreateLeafNode(Offset, ...)
        node      :: ResizeNodeContainer(.., Offset, ..)
        node      :: CreateNodeContainerPair(Offset, ...) // TODO Remove if function unused.

    Notes:
      - Could replace the KWMScreen.SplitRatio with some kind of KWM preferences reference? Or is that how it's operating now?
      - Rename to make obvious that the new Container is SPLIT from the Parent
*/
node_container CreateNodeContainer(const container_offset &Offset, const node_container &ParentContainer, const container_type &ContainerType);

/* Set Container to default values for fullscreen Root Node

    Map:
        Container ~> Container

    Parameters:
        SpaceBoundary - the current Space's boundary rectangle
        Offset - the gap Offset to use for the Container
    [M] *Container - Pointer to the Container to set to Root

    Global References:
        KWMScreen.SplitRatio - for default split ratio

    Mutations:
        Container - Set the Container's values to Root defaults.

    Return:
        (void)

    Called Functions:
        container :: GetOptimalSplitMode(Container)

    Calling Functions:
        display :: ChangePaddingOfDisplay(...)
        node    :: ResizeNodeContainer(SpaceBoundary, Offset, ..)
        node    :: CreateRootNode(SpaceBoundary, Offset)
    
    Notes:
      - Can probably rename to SetRootContainer, drop "Node" from name.
    
    TODO: Abstract display::ChangePaddingOfDisplay(...) from calling functions
*/
void SetRootNodeContainer(const bound_rect &SpaceBoundary, node_container *Container);

/* Modify the SplitRatio of the Container by Offset

    Map:
        Container ~> Container

    Parameters:
    [M] *Container - the Container whose SplitRatio is to be modified.
        Offset - the amount to modify the SplitRatio (0 < Offset < 1)

    Mutations:
        Container->SplitRatio += Offset

    Return:
        bool - true : SplitRatio updated
               false: SplitRatio not updated (invalid Offset)   

    Called Functions:
        (none)

    Calling Functions:
        node :: ModifyNodeSplitRatio
    
    Notes:
      - Usually the Windows will be updated after the containers are,
        but do we ever modify the split ratio without updating the containers?
        Should it happen in one step, or let the calling functions do it?
      - i.e. call ResizeElementInContainer in this function.
        -> I'd rather leave it up to a calling function, since we can modify
           a bunch of containers and then multithreaded resize.
    
    TODO: performance testing for ResizeElementInContainer.
*/
bool ModifyContainerSplitRatio(node_container *Container, const double &Offset);

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
      - Need to call ResizeTreeNodes and ApplyNodeContainer on Subtree after 
        changing the SplitMode to see the effect.
*/
void ToggleContainerSplitMode(node_container *Container);

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

/* Resize the WindowID's Window to fit in the Container.

    Map:
        Container ~> WindowRef

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
*/
void ResizeElementInContainer(const int &WindowID, node_container *Container);

#endif
