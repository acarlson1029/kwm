/* Functions that operate on Nodes and Containers */

#ifndef NODE_H
#define NODE_H

#include "types.h"

/* Create a new Leaf Node derived from the Parent

    Map:
        Node ~> Node

    Parameters:
        Offset        - the Screen padding Offset
        Parent        - the Parent Node from which the Leaf is derived
        WindowID      - the Element stored in the Node
        ContainerType - the type of container split:
                          * LeftVerticalContainerSplit
                          * RightVerticalContainerSplit
                          * UpperHorizontalContainerSplit
                          * LowerHorizontalContainerSplit
        
    Global References:
        (none)

    Mutations:
        (none)

    Return:
        tree_node - the Leaf Node that was derived from the Parent Node.

    Called Functions:
        container :: CreateNodeContainer(Offset, Parent->Container, ContainerType)

    Calling Functions:
        container :: CreateLeafNodePair(Offset, Parent, ...)
        serialize :: DeserializeChildNode(...)
    
    Notes:

    TODO: Refactor calling function DeserializeChildNode()
*/
tree_node *CreateLeafNode(const container_offset &Offset, tree_node *Parent, int WindowID, const container_type &ContainerType);

/* Create an empty RootNode for the Tree

    Map:
        Node ~> Node

    Parameters:
        Screen - the Display on which to create the Tree's RootNode
        Offset - the Screen's padding Offset for the RootNode's Container.

    Global References:
        (none)

    Mutations:
        (none)

    Return:
        tree_node - the created RootNode for the Tree

    Called Functions:
        SetRootNodeContainer(Screen, Offset, ..)

    Calling Functions:
        serialize :: DeserializeNodeTree(...)
        tree      :: CreateTreeFromWindowIDList(Screen, Offset, ...)
        tree      :: CreateMonocleTree(.., Screen, Offset, ..)
        tree      :: AddElementToMonocleTree(Screen, Offset, ...)
    
    Notes:

    TODO: Refactor DeserializeNodeTree to abstract away.
*/
tree_node *CreateRootNode(screen_info *Screen, const container_offset &Offset);

/* Create new Children Leaf Nodes from Parent Node

    Map:
        Node ~> Node

    Parameters:
        Offset         - the Space's Offset for container padding
    [M] Parent         - the Parent Node for creating and storing the Children
        FirstWindowID  - the WindowID of the first Element 
        SecondWindowID - the WindowID of the second Element
        SplitMode      - how to split the Parent's Container when creating Children:
                           * SplitModeOptimal (uses golden ratio)
                           * SplitModeVertical
                           * SplitModeHorizontal
        
    Global References:
        KWMScreen.SplitRatio       - default SplitRatio
        KWMTiling.SpawnAsLeftChild - preference for new window location.

    Mutations:
        Parent - invalidate Element, update SplitMode, reset SplitRatio, and store created Children

    Return:
        (void)

    Called Functions:
        CreateLeafNode(Offset, Parent, ...)

    Calling Functions:
        serialize::CreateLeafNodePair(...)
        tree::CreateBSPTree(..., Offset, SplitModeOptimal)
        tree::AddElementToBSPTree(Offset, Parent, Parent->WindowID, WindowID, SplitMode)
    
    Notes:
*/
void CreateLeafNodePair(const container_offset &Offset, tree_node *Parent, int FirstWindowID, int SecondWindowID, const split_mode &SplitMode);

/* Set the Element in the Node's Container

    Map:
        Node ~> Element

    Parameters:
        Screen   - the Display of the Space of the Tree of the Node
        Offset   - the Space padding offset/gaps
    [M] Node     - the Node of which to set the Element
        WindowID - the Element to set in the Node
        
    Global References:
        (none)

    Mutations:
        Node->Container.WindowID is set.
        (see ResizeNodeContainer)

    Return:
        (void)

    Called Functions:
    [M] node :: ResizeNodeContainer(Screen, Offset, Node)

    Calling Functions:
        tree :: ToggleElementInTree()
        tree :: ToggleElementInRoot()
    
    Notes:
*/
void SetElementInNode(screen_info *Screen, const container_offset &Offset, tree_node *Node, const int &WindowID);

/* Clear the Element in the Node's Container
 
     Map:
         Node ~> Element
     
     Parameters:
         Screen   - the Display of the Space of the Tree of the Node
         Offset   - the Space padding offset/gaps
         [M] Node - the Node of which to set the Element
     
     Global References:
        (none)
     
     Mutations:
        Node->Container.WindowID is cleared.
        (see ResizeNodeContainer)
     
     Return:
        (void)
     
     Called Functions:
        [M] node :: ResizeNodeContainer(Screen, Offset, Node)
     
     Calling Functions:
        tree :: ToggleElementInTree()
        tree :: ToggleElementInRoot()
     
     Notes:
 */
void ClearElementInNode(screen_info *Screen, const container_offset &Offset, tree_node *Node);

/* Resize the Node's Container with new constraints

    Map:
        Node ~> Container

    Parameters:
        Screen - the Display of the Space of the Tree of the Node
        Offset - the Space padding offset/gaps
    [M] Node - the Node for which the Container is updated
        
    Global References:
        (none)

    Mutations:
        Node->Container is resized

    Return:
        (void)

    Called Functions:
        container::SetRootNodeContainer(Screen, Offset, &Node->Container)
        container::CreateNodeContainer(Offset, Node->Parent, Node->Type)

    Calling Functions:
        tree::ResizeTreeNodes
        node::SetElementInNode(Screen, Offset, Node, ..)
        node::ClearElementInNode(Screen, Offset, Node)
    
    Notes:

    TODO: Add a Container function to resize the container explicitly without going over the abstraction barrier.
*/
void ResizeNodeContainer(screen_info *Screen, const container_offset &Offset, tree_node *Node);

/* Whether the given Node is a Leaf (has no children)
    
    Map:
        Node ~> Node
 
    Parameters:
        Node - the Node in question
    
    Global References:
        (none)
 
    Mutations:
        (none)
 
    Return:
        bool - true  : is a leaf
               false : is not a leaf
 
    Called Functions:
        (none)
 
    Calling Functions:
        AddElementToBSPTree
        AddWindowToBSPTree
        AddWindowToTreeOfUnfocusedMonitor
        CreateBSPTree
        FillDeserializedTree
        GetFirstLeafNode
        GetLastLeafNode
        GetNearestLeafNeighbor
        GetNearestNodeToTheLeft
        GetNearestNodeToTheRight
        IsLeftLeaf
        IsRightLeaf
        ModifySubtreeSplitRatioFromWindow
        RotateTree
        SaveBSPTreeToFile
        SerializeParentNode
        ToggleElementInRoot
        ToggleElementInTree
        ToggleSubtreeSplitMode
*/
bool IsLeafNode(tree_node *Node);

/* Whether the Node is on its Parent's Left
 
    Map:
        Node ~> Node
 
    Parameters:
        Node - the Node in question
 
    Global References:
        (none)
 
    Mutations:
        (none)
 
    Return:
        bool - true  : is left of parent
               false : is not left of parent
               note: false does not imply IsRightChild (may be root)
 
    Called Functions:
        (none)
 
    Calling Functions:
        node :: IsLeftLeaf(Node)
 
    Notes:
*/
bool IsLeftChild(tree_node *Node);

/* Whether the Node is on its Parent's Left
 
    Map:
        Node ~> Node
 
    Parameters:
        Node - the Node in question
 
    Global References:
         (none)
 
     Mutations:
         (none)
 
     Return:
         bool - true  : is right of parent
                false : is not right of parent
         note: false does not imply IsLeftChild (may be root)
 
     Called Functions:
         (none)
 
     Calling Functions:
         node :: IsRightLeaf(Node)
 
    Notes:
 */
bool IsRightChild(tree_node *Node);

/* Is Node a Leaf on Parent's Left
 
    Map:
        Node ~> Node
 
    Parameters:
        Node - the Node in question
 
    Global References:
        (none)
 
    Mutations:
        (none)
 
    Return:
        bool - true  : is left node and is leaf
               false : not (left node and is leaf)
 
    Called Functions:
        node :: IsLeftChild(Node)
        node :: IsLeafNode(Node)
 
    Calling Functions:
        serialize :: CreateDeserializedNodeContainer
*/
bool IsLeftLeaf(tree_node *Node);

/* Is Node a Leaf on Parent's Left
 
     Map:
         Node ~> Node
 
     Parameters:
         Node - the Node in question
 
     Global References:
         (none)
 
     Mutations:
         (none)
 
     Return:
         bool - true  : is right node and is leaf
                false : not (right node and is leaf)
 
     Called Functions:
         node :: IsRightChild(Node)
         node :: IsLeafNode(Node)
 
     Calling Functions:
         UNSUED
 */
bool IsRightLeaf(tree_node *Node);

/* (NYI) Is Node the RootNode of a Tree
 
     Map:
         Node ~> Node
 
     Parameters:
         Node - the Node in question
 
     Global References:
         (none)
 
     Mutations:
         (none)
 
     Return:
         bool - true  : is root node
                false : is not root node
 
    Called Functions:
        TODO
 
    Calling Functions:
        UNSUED
 */
bool IsRootNode(tree_node *Node);

/* Swap the Elements in two Nodes and resize their Elements to fit their Containers

    Map:
        Node ~> Container/Element

    Parameters:
    [M] A - first Node
    [M] B - second Node

    Global References:
        (none)

    Mutations:
        A->WindowID set to B->WindowID
        A->Container resized
        B->WindowID set to A->WindowID
        B->Container resized

    Return:
        (void)

    Called Functions:
        node :: ResizeElementInNode(..)

    Calling Functions:
        windowref :: SwapFocusedWindowDirected()
        windowref :: SwapFocusedWindowWithMarked()
        windowref :: SwapFocusedWindowWithNearest()
    
    Notes:
*/
void SwapNodeWindowIDs(tree_node *A, tree_node *B);

/* Change the SplitRatio of the Node's Container

    Map:
        Node ~> Container

    Parameters:
    [M] Node - the Node to modify
        Offset - the amount to change the SplitRatio (0 < Offset < 1)
        
    Global References:
        (none)

    Mutations:
        (see ModifyContainerSplitRatio)

    Return:
        bool - true  : SplitRatio changed successfully
               false : SplitRatio unchanged (likely invalid arg)

    Called Functions:
        container :: ModifyContainerSplitRatio(&Node->Container, Offset)

    Calling Functions:
        tree :: ModifySubtreeSplitRatio(...)
    
    Notes:
*/
bool ModifyNodeSplitRatio(tree_node *Node, const double &Offset);

/* Change the SplitMode of the Node's Container

    Map:
        Node ~> Container

    Parameters:
    [M] Node - the Node whose Container is to be changed
        
    Global References:
        (none)

    Mutations:
        (see ToggleContainerSplitMode)

    Return:
        (void)

    Called Functions:
        container :: ToggleContainerSplitMode(&Node->Container)

    Calling Functions:
        tree :: ToggleSubtreeSplitMode(.., Offset, Node)
    
    Notes:
*/
void ToggleNodeSplitMode(tree_node *Node);

/* Resize the Element in the Node

    Map:
        Node ~> Container

    Parameters:
    [M] Node - the Node whose Element is to be resized
        
    Global References:
        (none)

    Mutations:
        (see ResizeElementInContainer)

    Return:
        (void)

    Called Functions:
        container :: ResizeElementInContainer

    Calling Functions:
        tree::ApplyNodeContainer(Node, ..)
        node::SwapNodeWindowIDs(Node)
        windowtree::ResizeElementInTree()
    
    Notes:
*/
void ResizeElementInNode(tree_node *Node);
#endif
