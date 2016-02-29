/* Functions that operate on Containers and their contents */
#ifndef CONTAINER_H
#define CONTAINER_H

#include "types.h"

/* Create new node_containers from existing node_containers.
    Map:
        node_container -> node_container
    Inputs:
        Screen - for current Space offsetting
        Container - start with copy of current Container
    Outputs:
        node_container - split node_container based on the input Node
 */
node_container LeftVerticalContainerSplit(screen_info *Screen, const node_container &Container);
node_container RightVerticalContainerSplit(screen_info *Screen, const node_container &Container);
node_container UpperHorizontalContainerSplit(screen_info *Screen, const node_container &Container);
node_container LowerHorizontalContainerSplit(screen_info *Screen, const node_container &Container);

/* Determine whether to split Vertically or Horizontally based on Node->Container.
    Map:
        node_container -> split_mode
    Input:
        Container - get dimensions of container
    Output:
        split_mode - vertical split or horizontal split
*/
split_mode GetOptimalSplitMode(const node_container &Container);

/* Mux ContainerType to select which node_container to create
    Map:
        node_container -> node_container
    Input:
        Screen - pass through to create container functions
        Parent - Node whose container to use as baseline
        ContainerType - Left/Right/Top/Bottom
    Output:
        node_container - new container dimensions for split.
*/
node_container CreateNodeContainer(screen_info *Screen, const node_container &ParentContainer, const container_type &ContainerType);

/* Mux SplitMode to select which ContainerTypes to use for CreateNodeContainer
    Map:
        node_container -> node_container
    Input:
        Screen - pass through to  create container functions
        Parent - Node whose children containers are being created.
        SplitMode - whether to split horizontally or vertically.
    Output:
        tree_node *Parent - mutate Left and Right children containers
*/
void CreateNodeContainerPair(screen_info *Screen, tree_node *Parent, const split_mode &SplitMode);

/* Create Container for RootNode
    Map:
        Screen Pos -> node_container
    Input:
        Screen - for position info
    Output:
        node_container *Container
*/
void SetRootNodeContainer(screen_info *Screen, node_container *Container);

/* SET -- KWMScreen.SplitRatio */
void ChangeSplitRatio(double Value);

#endif
