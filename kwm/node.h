/* Functions that operate on Nodes and Containers */

#ifndef NODE_H
#define NODE_H

#include "types.h"

/* Create new tree_node from Parent
    Map:
        <args> -> tree_node
    Input:
        Screen - for creating container
        Parent - for tree_node::Parent
        WindowID - for tree_node::WindowID
        ContainerType - for creating container
    Output:
        tree_node (new)
*/
tree_node *CreateLeafNode(screen_info *Screen, tree_node *Parent, int WindowID, const container_type &ContainerType);

/* Create new children tree_nodes from Parent
    Map:
        <args> -> Parent->LeftChild, Parent->RightChild
    Input:
        Screen - pass to CreateLeafNode()
        Parent - Node for which to create children
        FirstWindowID - WindowID of first child
        SecondWindowID - WindowID of second child
        SplitMode - create vertical or horizontal containers
    Output:
        node_info *Parent - mutate to delete its WindowID, 
                            update its Container, 
                            and create two children leaf nodes
 */
void CreateLeafNodePair(screen_info *Screen, tree_node *Parent, int FirstWindowID, int SecondWindowID, const split_mode &SplitMode);

/* Create an empty RootNode for the tree
    Map:
        <none> -> tree_node
    Input:
        <none>
    Output:
        tree_node* - populate elements with invalid/null/uninitialized data.
*/
tree_node *CreateRootNode(screen_info *Screen);

void SetElementInNode(screen_info *Screen, tree_node *Node, const int &WindowID);
void ClearElementInNode(screen_info *Screen, tree_node *Node);

/* Mutate Node->Container based on Screen state */
void ResizeNodeContainer(screen_info *Screen, tree_node *Node);

/* Node properties */
bool IsLeafNode(tree_node *Node);
bool IsLeftChild(tree_node *Node);
bool IsRightChild(tree_node *Node);
bool IsLeftLeaf(tree_node *Node);
bool IsRightLeaf(tree_node *Node);
bool IsRootNode(tree_node *Node);

// TODO -- This calls the recursive function ApplyNodeContainer
/* Swap the WindowIDs in two nodes and resize them to their new node_container.
    Input:
        A - first node to swap.
        B - second node to swap.
    Output:
        A - mutate WindowID and A->Container
        B - mutate WindowID and A->Container
*/
void SwapNodeWindowIDs(tree_node *A, tree_node *B);

bool ModifyNodeSplitRatio(tree_node *Node, const double &Offset);
void ToggleNodeSplitMode(tree_node *Node);

#endif
