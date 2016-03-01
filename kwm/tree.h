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
tree_node *CreateTreeFromWindowIDList(screen_info *Screen, const std::vector<window_info*> &Windows);

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
bool CreateBSPTree(tree_node *RootNode, screen_info *Screen, const std::vector<window_info*> &Windows);

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
bool CreateMonocleTree(tree_node *RootNode, screen_info *Screen, const std::vector<window_info*> &Windows);

/* Recursively destroy the subtree and deallocate memory starting at Node
    Input:
        Node - root of the subtree to destroy
        Mode - whether the tree is BSP or Monocle
    Output:
        Node - destroy Node and all Nodes in subtree.
*/
void DestroyNodeTree(tree_node *Node, space_tiling_option Mode);

/* Create new Nodes for Element, and insert them into the Tree */
void AddElementToBSPTree(screen_info *Screen, tree_node *NewParent, int WindowID, const split_mode &SplitMode);
void AddElementToMonocleTree(screen_info *Screen, tree_node *NewParent, int WindowID);
void AddElementToTree(screen_info *Screen, tree_node *NewParent, int WindowID, const split_mode &SplitMode, const space_tiling_option &Mode);

/* Remove Element from Tree, and delete the Node if necessary. Rearranges the Tree */
void RemoveElementFromBSPTree(screen_info *Screen, tree_node *Node);
void RemoveElementFromMonocleTree(screen_info *Screen, tree_node *Node);
void RemoveElementFromTree(screen_info *Screen, tree_node *Root, int WindowID, const space_tiling_option &Mode);

/* Promote Element in Tree. Assign the Element its Node's Parent, using its Container
 * This is useful for things like toggling a Window to fill its parent container, or 
 * toggling a window to go fullscreen (Root node container) */
// TODO Can these functions be combined?
bool ToggleElementInTree(screen_info *Screen, tree_node *Root, const int &WindowID, const space_tiling_option &Mode);
bool ToggleElementInRoot(screen_info *Screen, tree_node *Root, const int &WindowID, const space_tiling_option &Mode);

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

/* Recursively resize all windows in nodes in subtree 
    Map:
        Window Resize -> tree
    Input:
        tree_node *Node - subtree containing nodes to be resized
        space_tiling_window_option Mode - whether we're in SpaceModeBSP
    Output:
        tree_node *Node - resized windows in children
*/
void ApplyNodeContainer(tree_node *Node, space_tiling_option Mode);

/* Recursively resize all node containers in nodes in subtree.
    Map:
        Container Resize -> tree
    Input:
        Screen - passthrough to container functions
        Root - the root node of the tree to resize containers for,
               based off of Root.Container
    Output:
        tree_node *Root - mutate the containers of all nodes in the tree starting from Root.
 */
void ResizeTreeNodes(screen_info *Screen, tree_node *Root);

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
void ToggleSubtreeSplitMode(screen_info *Screen, tree_node *Node);

void ModifySubtreeSplitRatio(screen_info *Screen, tree_node *Root, const double &Offset);

/* Tree traversal/selection */
/* GET functions -- no mutation of inputs */
tree_node *GetFirstLeafNode(tree_node *Node);   // left-most leaf
tree_node *GetLastLeafNode(tree_node *Node);    // right-most leaf
tree_node *GetNearestNodeToTheLeft(tree_node *Node, space_tiling_option Mode);
tree_node *GetNearestNodeToTheRight(tree_node *Node, space_tiling_option Mode);
tree_node *GetNearestLeafNeighbour(tree_node *Node, space_tiling_option Mode);
tree_node *GetFirstPseudoLeafNode(tree_node *Node);
tree_node *GetNodeFromWindowID(tree_node *Node, int WindowID, space_tiling_option Mode);

/* Traversal helper functions */

/* Apply function *f to each Node in the Tree */
void PreOrderTraversal(void (*f)(screen_info *Screen, tree_node *Root), screen_info *Screen, tree_node *Root);
/* Search the Nodes in the tree, until is_match(Node) returns true */
tree_node *LevelOrderSearch(bool (*is_match)(tree_node *), tree_node *Root);

#endif
