/* Functions that operate on Trees and Nodes */
#ifndef TREE_H
#define TREE_H

#include "types.h"

/* Create a BSP Tree starting at the RootNode from a list of Windows

    Map:
        Tree ~> Tree

    Parameters:
    [M] RootNode - Make this the Root of the created Tree.
        SpaceBoundary - the boundary rect of the Space
        Offset   - the Space gap offset padding.
        Windows  - the list of Windows to be added to the tree.
        
    Mutations:
        RootNode will be updated with the full Tree (NodeChildren pointers set)

    Return:
        bool - true  : Tree created successfully
               false : No Tree created

    Called Functions:
        node :: IsLeafNode(..)
        node :: CreateLeafNodePair(Offset, ..., SplitModeOptimal)

    Calling Functions:
        CreateTreeFromWindowIDList()
    
    Notes:

    TODO: This function should just call AddElementToTree for each Window
    TODO: This function should genericize the input to "Elements" instead of "Windows"
*/
bool CreateBSPTree(tree_node *RootNode, const bound_rect &SpaceBoundary, const container_offset &Offset, const std::vector<window_info*> &Windows);

/* Create a Monocle Tree starting at the RootNode from a list of Windows

    Map:
        Tree ~> Tree

    Parameters:
    [M] RootNode - Make this the Root of the created Tree.
        SpaceBoundary - the boundary rect of the Space
        Offset   - the Space gap offset padding.
        Windows  - the list of Windows to be added to the tree.
        
    Mutations:
        RootNode will be updated with the full Tree (NodeChildren pointers set)

    Return:
        bool - true  : Tree created successfully
               false : No Tree created (empty Element list)

    Called Functions:
        node :: CreateRootNode()

    Calling Functions:
        CreateTreeFromWindowIDList()
    
    Notes:

    TODO: This function should just call AddElementToTree for each Window
    TODO: This function should genericize the input to "Elements" instead of "Windows"
*/
bool CreateMonocleTree(tree_node *RootNode, const bound_rect &SpaceBoundary, const container_offset &Offset, const std::vector<window_info*> &Windows);

/* Create a BSP or Monocle Tree for a list of Windows

    Map:
        Tree ~> Tree

    Parameters:
        SpaceBoundary - the boundary rect of the Space
        Offset  - the Offset to use when creating the container.
        Windows - the Elements to add to the tree.
        Mode    - the Space Tiling Mode
                  note: SpaceModeBSP and SpaceModeMonocle get Trees, others get NULL
        
    Mutations:
        (none)

    Return:
        tree_node - the RootNode of the created Tree
                    NULL if tree not created

    Called Functions:
        tree :: CreateBSPTree()
        tree :: CreateMonocleTree()
        
    Calling Functions:
        windowtree :: CreateWindowNodeTree()
    
    Notes:
    
*/
tree_node *CreateTreeFromWindowIDList(const bound_rect &SpaceBoundary, const container_offset &Offset, const std::vector<window_info*> &Windows, const space_tiling_option &Mode);

/* Recursively destroy the subtree and deallocate memory starting at Node
    Input:
        Node - root of the subtree to destroy
        Mode - whether the tree is BSP or Monocle
    Output:
        Node - destroy Node and all Nodes in subtree.
*/
void DestroyNodeTree(tree_node *Node, space_tiling_option Mode);

/* Create new Nodes for Element, and insert them into the Tree */
void AddElementToBSPTree(const container_offset &Offset, tree_node *NewParent, int WindowID, const split_mode &SplitMode);
void AddElementToMonocleTree(const bound_rect &SpaceBoundary, const container_offset &Offset, tree_node *NewParent, int WindowID);
void AddElementToTree(const bound_rect &SpaceBoundary, const container_offset &Offset, tree_node *NewParent, int WindowID, const split_mode &SplitMode, const space_tiling_option &Mode);

/* Remove Element from Tree, and delete the Node if necessary. Rearranges the Tree */
void RemoveElementFromBSPTree(const bound_rect &SpaceBoundary, space_info *Space, tree_node *Node);
void RemoveElementFromMonocleTree(space_info *Space, tree_node *Node);
void RemoveElementFromTree(const bound_rect &SpaceBoundary, space_info *Space, tree_node *Root, int WindowID, const space_tiling_option &Mode);

/* Promote Element in Tree. Assign the Element its Node's Parent, using its Container
 * This is useful for things like toggling a Window to fill its parent container, or 
 * toggling a window to go fullscreen (Root node container) */
// TODO Can these functions be combined?
bool ToggleElementInTree(const bound_rect &SpaceBoundary, tree_node *Root, const int &WindowID, const space_tiling_option &Mode, const container_offset &Offset);
bool ToggleElementInRoot(const bound_rect &SpaceBoundary, tree_node *Root, const int &WindowID, const space_tiling_option &Mode, const container_offset &Offset);

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
        SpaceBoundary - the boundary rect of the Space
        Root - the root node of the tree to resize containers for,
               based off of Root.Container
    Output:
        tree_node *Root - mutate the containers of all nodes in the tree starting from Root.
 */
void ResizeTreeNodes(const bound_rect &SpaceBoundary, const container_offset &Offset, tree_node *Root);

/* Change split_mode for node, 
   recursively create new containers for subtree, and
   recursively resize all windows to fit in subtree node containers.
    Input:
        SpaceBoundary - the boundary rect of the Space
        Node - root of subtree to process
    Output:
        tree_node *Node - Mutate SplitMode, Container, 
                          containers all nodes in subtree, and 
                          window size in all nodes of subtree.
*/
void ToggleSubtreeSplitMode(const bound_rect &SpaceBoundary, const container_offset &Offset, tree_node *Node);

void ModifySubtreeSplitRatio(const bound_rect &SpaceBoundary, tree_node *Root, const double &Delta, const container_offset &Offset, const space_tiling_option &Mode);

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
void PreOrderTraversal(void (*f)(const bound_rect &SpaceBoundary, const container_offset &Offset, tree_node *Root), const bound_rect &SpaceBoundary, const container_offset &Offset, tree_node *Root);
/* Search the Nodes in the tree, until is_match(Node) returns true */
tree_node *LevelOrderSearch(bool (*is_match)(tree_node *), tree_node *Root);

#endif
