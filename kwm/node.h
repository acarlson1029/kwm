#ifndef __NODE__
#define __NODE__

#include "types.h"

/*! @header node.h
    @brief Functions that operate on Tree Nodes

    @note
    These functions have no concept of what Element is stored in the Node.
 */

/*! @functiongroup Constructors
 */

/*! @brief Create an empty RootNode for the Tree

    @return `tree_node` Pointer to the created RootNode for the Tree
 
    @discussion
    An "empty" Node is defined as an object whose Parent, LeftChild, RightChild,
    and Element all point to NULL.

    @note
    This function calls `new`

    @see DestroyNode()
*/
tree_node *CreateRootNode();

/*! @brief Create a new Leaf Node derived from its Parent.
 
    @param Parent The Node to which the Leaf will point
 
    @return `tree_node` pointer to the created LeafNode

    @discussion
    The newly created Node will have its Parent attribute already set.

    @note
    This function calls CreateRootNode(), which calls `new`.

    @see CreateRootNode()
    @see DestroyNode()
 */ 
tree_node *CreateLeafNode(const tree_node &Parent);

/*! @functiongroup Destructors
 */

/*! @brief Destroy a Node, freeing its memory from the heap.

    @param Node (mutate) Destroy the Node, freeing its memory.

    @warning This is an in-place deletion. The Tree will not be mutated to 
    reflect the absence of this Node.

    @pre Readjust the Tree containing the Node first -- its Parents and
    Children will be left orphaned and the Tree broken otherwise.

    @discussion
    If you mean to remove a Node from the Tree data structure, use 
    RemoveNodeFromTree() instead.

    @note
    This function calls `delete`

    @see RemoveNodeFromTree()
    @see CreateRootNode()
 */
void DestroyNode(tree_node *Node);

/*! @brief Remove a Node from the Tree

    @param Leaf (mutate) The Leaf to be removed from the Tree,
    and object deleted.

    @return Determine whether the Node was actually removed.

    @discussion
    Removal of the Leaf will rebalance the existing Tree structure to accomodate
    the missing Node and maintain the structural rule that each Parent have
    exactly two Children.
    The Leaf's Sibling Node is copied into their Parent Node, and both Children
    are destroyed.
    
    This function will return false if the parameter is either:
      - NULL
      - not a Leaf Node
      - the Root Node of the the Tree

    @note
    This function calls DestroyNode(), which calls `delete`

    @see DestroyNode()
 */
bool RemoveNodeFromTree(tree_node *Leaf);

/*! @functiongroup Queries
    @warning These queries operate on const references -- check for NULL pointers
    before calling them.
 */

/*! @brief Test whether the Node is a Leaf (has no Children)

    @param Node
    @return

    @note
    A Node can be a RootNode and a LeafNode.
 */
bool IsLeafNode(const tree_node &Node);

/*! @brief Test whether the Node is the LeftChild of its Parent

    @param Node
    @return

    @note
    This applies to non-Leaf Nodes as well
    @note
    Nodes without a Parent (i.e. Root Node) will return `false`

    @warning `false` does NOT imply IsRightChild(Node)
    Ex: A RootNode will be neither a LeftChild or RightChild

    @see IsRightChild()
 */
bool IsLeftChild(const tree_node &Node);

/*! @brief Test whether the Node is the RightChild of its Parent

    @param Node
    @return

    @note
    This applies to non-Leaf Nodes as well
    @note
    Nodes without a Parent (i.e. Root Node) will return `false`

    @warning `false` does NOT imply IsLeftChild(Node)
    Ex: A RootNode will be neither a LeftChild or RightChild

    @see IsLeftChild()
 */
bool IsRightChild(const tree_node &Node);

/*! @brief Test whether the Node is the Left Leaf of its Parent

    @param Node
    @return IsLeafNode(None) && IsLeftChild(Node)

    @note
    Nodes without a Parent (i.e. Root Node) will return `false`

    @warning `false` does NOT imply IsLeftChild() or IsRightChild() or IsRightLeaf()
    Ex: A RootNode will be none of these things.

    @see
    IsLeafNode()
    IsLeftChild()
    IsRightChild()
    IsRightLeaf()
 */
bool IsLeftLeaf(const tree_node &Node);

/*! @brief Test whether the Node is the Right Leaf of its Parent

    @param Node
    @return IsLeafNode(None) && IsRightChild(Node)

    @note
    Nodes without a Parent (i.e. Root Node) will return `false`

    @warning `false` does NOT imply IsRightChild(), IsLeftChild(), or IsLeaf()
    Ex: A RootNode will be none of these things.

    @see
    IsLeafNode()
    IsLeftLeaf()
    IsLeftChild()
    IsRightChild()
 */
bool IsRightLeaf(const tree_node &Node);

/*! @brief Test whether the Node is the RootNode of a Tree

    @param Node
    @return `true` if Node has no Parent

    @warning `false` does NOT imply IsRightChild(), IsLeftChild(), or IsLeaf()

    @see
    IsLeafNode()
    IsLeftChild()
    IsRightChild()
 */
bool IsRootNode(const tree_node &Node);

/*! @functiongroup Get Node
    @warning These functions operate on const references -- check for NULL pointers
    before calling them.
    @todo These functions currently return Pointers to **COPIES** of the Node.
 */

/*! @brief Get the Node if it is a Leaf.

    @param Node
    @return `tree_node` Pointer to the returned Node, or NULL

    @note
    This function will return a NULL Pointer if the specified matching 
    Node was not found.

    @see IsLeafNode()
 */
tree_node *GetNodeIfLeaf(const tree_node &Node);

/*! @brief Get the first Leaf Node in the subtree starting at Node
 
    @param Node
    @return `tree_node` Pointer to the returned Node, or NULL

    @discussion
    The "First" Leaf Node is defined as the left-most Leaf node regardless of
    the level of the Tree it's found on.

    @note
    Checks the Node parameter itself
    @note
    This function will return a NULL Pointer if the specified matching
    Node was not found, though it should never do this.

    @see IsLeafNode()
 */
tree_node *GetFirstLeafNode(const tree_node &Node);

/*! @brief Get the last Leaf Node in the subtree starting at Node
 
    @param Node
    @return `tree_node` Pointer to the returned Node, or NULL

    @discussion
    The "Last" Leaf Node is defined as the right-most Leaf node regardless of
    the level of the Tree it's found on.

    @note
    Checks the Node parameter itself
    @note
    This function will return a NULL Pointer if the specified matching
    Node was not found, though it should never do this.

    @see IsLeafNode()
 */
tree_node *GetLastLeafNode(const tree_node &Node);

/*! @brief Get the Leaf Node to the left of Node
 
    @param Node
    @return `tree_node` Pointer to the returned Node, or NULL

    @discussion
    The "Left" Leaf Node is defined as the next Leaf Node to the Left of Node, 
    regardless of the level in the Tree it's found on.
    If Node is a RightChild, get the right-most Leaf of its Sibling (i.e. Node->Parent->LeftChild)
    If Node is a LeftChild, walk up the Tree and get the nearest Left Node of its Parent.

    @note
    This function recursively calls itself.
    @note
    This function will return a NULL Pointer if the specified matching
    Node was not found.
    Ex: In the following case, there is no left Leaf Node

    @code GetNearestLeafNodeToTheLeft(GetFirstLeafNode(Root))

    @see IsLeafNode()
    @see GetLastLeafNode()
 */
tree_node *GetNearestLeafNodeToTheLeft(const tree_node &Node);

/*! @brief Get the Leaf Node to the right of Node
 
    @param Node
    @return `tree_node` Pointer to the returned Node, or NULL

    @discussion
    The "Right" Leaf Node is defined as the next Leaf Node to the Right of Node, 
    regardless of the level in the Tree it's found on.
    If Node is a LeftChild, get the left-most Leaf of its Sibling (i.e. Node->Parent->RightChild)
    If Node is a RightChild, walk up the Tree and get the nearest Right Node of its Parent.

    @note
    This function recursively calls itself.
    @note
    This function will return a NULL Pointer if the specified matching
    Node was not found.
    Ex: In the following case, there is no right Leaf Node

    @code GetNearestLeafNodeToTheLeft(GetLastLeafNode(Root))

    @see IsLeafNode()
    @see GetFirstLeafNode()
 */
tree_node *GetNearestLeafNodeToTheRight(const tree_node &Node);

/*! @brief Get the closest Leaf Node from Node in the Tree 
 
    @param Node The Leaf Node of which the neighbor is to be found
    @return `tree_node` Pointer to the returned Node, or NULL

    @discussion
    If the Node is not a Leaf Node, return NULL.
    If the Node is a Left Leaf Node, return the nearest Right Leaf Node.
    If the Node is a Right Leaf Node, return the nearest Left Leaf Node.

    @note
    This function will return a NULL Pointer if the specified matching
    Node was not found, or Node is not a Leaf

    @see
    IsLeafNode()
    IsLeftLeaf()
    GetNearestLeafNodeToTheRight()
    GetNearestLeafNodeToTheLeft()
 */
tree_node *GetNearestLeafNeighbour(const tree_node &Node);

/*! @functiongroup Mutators
 */

#endif
