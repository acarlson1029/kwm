/* Functions that operate on Trees and Nodes */
#ifndef TREE_H
#define TREE_H

#include "types.h"

/* Constructors, Destructors, Mutators */

/* Create either a BSP Tree or MonocoleTree from the given list of Windows
    Map:
        std::vector<window_info*> -> Tree
    Input:
        Screen - used to determine the mode (BSP or Monocle)
        WindowsPtr - list of windows to put in the tree.
    Output:
        tree_node - Root of full tree.
*/
tree_node *CreateTreeFromWindowIDList(screen_info *Screen, std::vector<window_info*> *WindowsPtr);

/* Create a BSP Tree starting at the root node from a list of windows. 
    Map:
        std::vector<window_info*> -> Tree
    Input:
        RootNode - the root of the BSP tree to be created.
        Screen - pass through to create Containers.
        WindowsPtr - list of windows to put in the tree.
    Output:
        tree_node* RootNode - mutate the RootNode to populate with children created in this function.
        bool - Success status of the tree creation process.
*/
bool CreateBSPTree(tree_node *RootNode, screen_info *Screen, std::vector<window_info*> *WindowsPtr);

/* Create a Monocle Tree starting at the root node from a list of windows.
    Map:
        std::vector<window_info*> -> Tree
    Intput:
        RootNode - the root of the Monocle tree to be created.
        Screen - pass through to create Containers.
        WindowsPtr - list of windows to put in the tree.
    Output:
        tree_node* RootNode - mutate the RootNode to populate with children created in this function.
        bool - Success status of the tree creation process.
*/
bool CreateMonocleTree(tree_node *RootNode, screen_info *Screen, std::vector<window_info*> *WindowsPtr);

/* Recursively destroy the subtree and deallocate memory starting at Node
    Input:
        Node - root of the subtree to destroy
        Mode - whether the tree is BSP or Monocle
    Output:
        Node - destroy Node and all Nodes in subtree.
*/
void DestroyNodeTree(tree_node *Node, space_tiling_option Mode);

/* Recursively swap left and right children according to Deg
    Input:
        Node - root of the subtree to start swapping
        Deg - number of degrees to rotate (valid 90, 180, 270)
            90:  Rotate right, switch from SplitModeVertical to SplitModeHorizontal
            180: Keep split mode, just swap windows.
            270: Rotate left, switch from SplitModeHorizontal to SplitModeVertical
    Output:
        tree_node *Node - mutate LeftChild, RightChild, and Container for every node in subtree.
*/
void RotateTree(tree_node *Node, int Deg);

/* TODO: Have traversal functions in tree.h and base functions in node.h */
/* Recursively create Containers for Tree starting from Node.
    Map:
        Recursive node_container -> node_container
    Input:
        screen_info *Screen - for Container functions
        tree_node *Node     - tree containing nodes to recursively process
    Output:
        tree_node *Node     - mutated children Containers
 */
void ResizeNodeContainer(screen_info *Screen, tree_node *Node);

/* Recursively create node_container pairs for Tree starting from Node.
    Map:
        Recursive node_container -> node_container
    Input:
        screen_info *Screen - for Container functions
        tree_node *Node     - tree containing nodes to recursively process
    Output:
        tree_node *Node     - created children Containers
*/
void CreateNodeContainers(screen_info *Screen, tree_node *Node, bool OptimalSplit);

/* Recursively resize all windows in nodes in subtree 
    Map:
        Window Resize -> node
    Input:
        tree_node *Node - subtree containing nodes to be resized
        space_tiling_window_option Mode - whether we're in SpaceModeBSP
    Output:
        tree_node *Node - resized windows in children
*/
void ApplyNodeContainer(tree_node *Node, space_tiling_option Mode);

/* Change split_mode for node, 
   recursively create new containers for subtree, and
   recursively resize all windows to fit in subtree node containers.
    Input:
        Screen - used for creating containers
        Node - root of subtree to process
    Output:
        tree_node *Node - Mutate SplitMode, Container, 
                          containers all nodes in subtree, and 
                          window size in all nodes of subtree.
*/
void ToggleNodeSplitMode(screen_info *Screen, tree_node *Node);

/* Tree traversal/selection */
/* GET functions -- no mutation of inputs */
tree_node *GetFirstLeafNode(tree_node *Node);   // left-most leaf
tree_node *GetLastLeafNode(tree_node *Node);    // right-most leaf
tree_node *GetNearestNodeToTheLeft(tree_node *Node, space_tiling_option Mode);
tree_node *GetNearestNodeToTheRight(tree_node *Node, space_tiling_option Mode);
tree_node *GetNearestLeafNeighbour(tree_node *Node, space_tiling_option Mode);
tree_node *GetFirstPseudoLeafNode(tree_node *Node);
tree_node *GetNodeFromWindowID(tree_node *Node, int WindowID, space_tiling_option Mode);

#endif
