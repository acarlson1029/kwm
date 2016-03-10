#ifndef __TREE__
#define __TREE__

#include "types.h"

/*! @header tree.h
    @abstract
    Functions describing an abstract Binary Tree data structure which contains
    Nodes.

    Each Node contains pointer to a stored Element, and pointers to other Nodes:
      * Parent
      * LeftChild
      * RightChild

    Every Node in the Tree must have exactly Zero or Two children.
    Only Leaf Nodes (i.e. no children) contain a valid Element in the Tree.

    @note
    These functions should be type-agnostic toward the Element stored in the 
    Nodes of the Tree.

    Mutation functions have been provided to traverse the Tree and operate
    on the Elements for scopes which contain a Tree data structure.

    The Tree stores pointers to the Elements, so the pointer can be retrieved
    from the Tree and the Element mutated in-place within calling scopes.

    @todo Should I group all of these functions into a tree namespace? Or should I just make a class?
 */

/*! @functiongroup Constructors
 */

/*! @brief Create an empty Tree

    @return `tree_node` pointer to the Root Node of the Tree
 */
tree_node *CreateTree();

/*! @brief Create a Tree populated with a list of Elements

    @param Elements The list of Elements to be added to the Tree

    @return `tree_node` pointer to the Root Node of the Tree

    @pre Fully construct the Elements, otherwise it will be time-consuming to find them
    in the fully populated Tree and postprocess them.

    @note
    The Elements are added to the Tree based on the location determined
    by AddElementToTree().
    Set configuration options related to this in order to tune Tree creation.

    @post Traverse the Tree to apply any Element mutations that are dependent
    on an Element's position within the Tree

    @see
    CreateTree()
    AddElementToTree()
 */
template <typename T> tree_node *CreateTreeFromElements(const std::vector<T*> &Elements);

/*! @functiongroup Destructors
 */

/*! @brief Destroy the subtree specified by RootNode, deleting memory for all 
    of the Nodes

    @param The Root of the Tree to destroy

    @note
    This function can be called on a subtree of an existing tree, 
    destroying only the Nodes within that subtree.

    @warning
    All of the Nodes in the Tree will be deleted and lost.
    If you intend to remove elements instead (i.e. for reinsertion), use 
    RemoveElementFromTree()

    @pre If Destroying a subtree, make sure the parent Tree is rebalanced for
    the removal of the RootNode parameter.

    @see
    PostOrderMutation()
    DestroyNode()
 */
void DestroyTree(tree_node *RootNode);

/*! @functiongroup Searching for Nodes by Element

    @pre These functions operate on a const reference to the Tree's Root Node
    Make sure the RootNode is not NULL

    @todo Should traversal functions have an option to branch Right before Left?
 */

/*! @brief Pre-order search for a Node containing the Element in the Tree

    @param RootNode The Root of the subtree to search
    @param Element The Element used for the search

    @return `tree_node` pointer to the Node containing the Element, or NULL

    @discussion
    Pre-order search checks the Root Node first, then the Nodes in the Left 
    branch, and then the Nodes in the Right branch of the Tree.

    Returns as soon as the first match is found.
    If no match is found, return a NULL pointer.
    
    @note
    This function is recursive.
 */
template <typename T> tree_node *PreOrderSearch(const tree_node &RootNode, const T &Element);

/*! @brief Post-order search for a Node containing the Element in the Tree

    @param RootNode The Root of the subtree to search
    @param Element The Element used for the search

    @return `tree_node` pointer to the Node containing the Element, or NULL

    @discussion
    Post-order search checks the Nodes in the Left branch first, then the Nodes
    in the Right branch, and then the Root Node of the Tree

    Returns as soon as the first match is found.
    If no match is found, return a NULL pointer.
    
    @note
    This function is recursive.
 */
template <typename T> tree_node *PostOrderSearch(const tree_node &RootNode, const T &Element);

/*! @brief In-order search for a Node containing the Element in the Tree

    @param RootNode The Root of the subtree to search
    @param Element The Element used for the search

    @return `tree_node` pointer to the Node containing the Element, or NULL

    @discussion
    In-order search checks the Nodes in the Left branch first, then the Root
    Node, and then the Nodes in the Right branch of the Tree.

    Returns as soon as the first match is found.
    If no match is found, return a NULL pointer.
    
    @note
    This function is recursive.
 */
template <typename T> tree_node *InOrderSearch(const tree_node &RootNode, const T &Element);

/*! @brief Level-order search for a Node containing the Element in the Tree

    @param RootNode The Root of the subtree to search
    @param Element The Element used for the search

    @return `tree_node` pointer to the Node containing the Element, or NULL

    @discussion
    In-order search checks the Root Node first, then the Nodes in the next
    level down from Left to Right, and continues down one level each iteration.

    Returns as soon as the first match is found.
    If no match is found, return a NULL pointer.
    
    @note
    This function is NOT recursive - it uses a queue.
 */
template <typename T> tree_node *LevelOrderSearch(const tree_node &RootNode, const T &Element);

/*! @functiongroup Searching Elements by Node
 */

/*! @brief Pre-order search for an Element as a function on its Node

    @param RootNode The Root of the subtree to search
    @param f Function that takes a `tree_node` argument to determine whether
    that Node's Element is a match.

    @return <template> pointer to the Element in the Node that satisfies the
    search function, or NULL

    @discussion
    This function calls the given function on each Node as it traverses the Tree:
    @code
        template <typename T> T *f(const tree_node &Node)
        {
            if(<Node satisfies what I'm looking for>)
                return Node.Element
            else
                return NULL
        }
    @endcode

    If the function determines that Node is a match, it returns the Element
    in the Node, otherwise it returns NULL to signal that it didn't match,
    so that the traversal may continue.

    @warning
    The traversal functions should NOT be recursive and only operate on the 
    given Node

    Returns as soon as the first match is found.
    If no match is found, return a NULL pointer.

    @note
    This function is pre-order recursive.
 */
template <typename T> T *PreOrderSearch(const tree_node &RootNode, T *(*f)(const tree_node &Node));

/*! @brief Post-order search for an Element as a function on its Node

    @param RootNode The Root of the subtree to search
    @param f Function that takes a `tree_node` argument to determine whether
    that Node's Element is a match.

    @return <template> pointer to the Element in the Node that satisfies the
    search function, or NULL

    @discussion
    This function calls the given function on each Node as it traverses the Tree:
    @code
        template <typename T> T *f(const tree_node &Node)
        {
            if(<Node satisfies what I'm looking for>)
                return Node.Element
            else
                return NULL
        }
    @endcode

    If the function determines that Node is a match, it returns the Element
    in the Node, otherwise it returns NULL to signal that it didn't match,
    so that the traversal may continue.

    @warning
    The traversal functions should NOT be recursive and only operate on the 
    given Node

    Returns as soon as the first match is found.
    If no match is found, return a NULL pointer.

    @note
    This function is post-order recursive.
 */
template <typename T> T *PostOrderSearch(const tree_node &RootNode, T *(*f)(const tree_node &Node));

/*! @brief In-order search for an Element as a function on its Node

    @param RootNode The Root of the subtree to search
    @param f Function that takes a `tree_node` argument to determine whether
    that Node's Element is a match.

    @return <template> pointer to the Element in the Node that satisfies the
    search function, or NULL

    @discussion
    This function calls the given function on each Node as it traverses the Tree:
    @code
        template <typename T> T *f(const tree_node &Node)
        {
            if(<Node satisfies what I'm looking for>)
                return Node.Element
            else
                return NULL
        }

    If the function determines that Node is a match, it returns the Element
    in the Node, otherwise it returns NULL to signal that it didn't match,
    so that the traversal may continue.

    @warning
    The traversal functions should NOT be recursive and only operate on the 
    given Node

    Returns as soon as the first match is found.
    If no match is found, return a NULL pointer.

    @note
    This function is in-order recursive.
 */
template <typename T> T *InOrderSearch(const tree_node &RootNode, T *(*f)(const tree_node &Node));

/*! @brief Level-order search for an Element as a function on its Node

    @param RootNode The Root of the subtree to search
    @param f Function that takes a `tree_node` argument to determine whether
    that Node's Element is a match.

    @return <template> pointer to the Element in the Node that satisfies the
    search function, or NULL

    @discussion
    This function calls the given function on each Node as it traverses the Tree:
    @code
    template <typename T> T *f(const tree_node &Node)
    {
        if(<Node satisfies what I'm looking for>)
            return Node.Element
        else
            return NULL
    }
    @endcode

    If the function determines that Node is a match, it returns the Element
    in the Node, otherwise it returns NULL to signal that it didn't match,
    so that the traversal may continue.

    @warning
    The traversal functions should NOT be recursive and only operate on the 
    given Node

    Returns as soon as the first match is found.
    If no match is found, return a NULL pointer.

    @note
    This function is NOT recursive. It iterates in level-order using a queue.
 */
template <typename T> T *LevelOrderSearch(const tree_node &RootNode, T *(*f)(const tree_node &Node));

/*! @functiongroup Queries
 */

/*! @brief Check membership of the Element in the Tree

    @param RootNode The Root of the subtree to search
    @param Element The Element used for the search

    @return Whether the Element was found in the Tree

    @discussion
    Performs a pre-order search for the given Element in the Tree.
    If the Element was found by the search, returns true, otherwise false.

    @see
    PreOrderSearch(Element)
 */
template <typename T> bool IsElementInTree(const tree_node &RootNode, const T &Element);

/*! @functiongroup Get Element(s) From Tree
 */

/*! @brief Collect all of the Elements in the Tree left-to-right, top-to-bottom

    @param RootNode The Root of the subtree to collect the Elements from
    @param f Function that takes a `tree_node` argument to determine whether
    that Node's Element is a match.

    @return Collection of pointers to all of the Elements in the Tree

    @note This function is the same as LevelOrderSearch, except instead of 
    returning on the first match, it collects all of the matches and continues
    traversing the tree.

    @see LevelOrderSearch(f)
 */
template <typename T> std::vector<T*> *LevelOrderCollection(const tree_node &RootNode, T *(*f)(const tree_node &Node));

/*! @brief Get the first Element in the subtree starting at Node

    @param Node The Root of the subtree to search for Element

    @return Pointer to the found Element, or NULL

    @discussion
    The "First" Element is defined as the Element in the left-most Leaf node,
    regardless of the level of the Tree it's found on.

    @note
    This function will return a NULL pointer if the specified matching Node
    does not contain an Element.

    @see GetFirstLeafNode()
 */
template <typename T> T *GetFirstElement(const tree_node &Node);

/*! @brief Get the last Element in the subtree starting at Node
 
    @param Node The Root of the subtree to search for the Element

    @return Pointer to the found Element, or NULL

    @discussion
    The "Last" Element is defined as the Element in the right-most Leaf node, 
    regardless of the level of the Tree it's found on.

    @note
    This function will return a NULL Pointer if the specified matching Node 
    does not contain an Element.

    @see GetLastLeafNode()
 */
template <typename T> T *GetLastElement(const tree_node &Node);

/*! @brief Get the Element in the Leaf Node to the left of Node
 
    @param Node

    @return Pointer to the found Element, or NULL

    @note
    This function will return a NULL Pointer if the specified matching Node 
    could not be found, or does not contain an Element.

    @see GetNearestElementToTheLeft()
 */
template <typename T> T *GetNearestElementToTheLeft(const tree_node &Node);

/*! @brief Get the Element in the Leaf Node to the right of Node
 
    @param Node

    @return Pointer to the found Element, or NULL

    @note
    This function will return a NULL Pointer if the specified matching Node 
    could not be found, or does not contain an Element.

    @see GetNearestElementToTheRight()
 */
template <typename T> T *GetNearestElementToTheRight(const tree_node &Node);

/*! @brief Get the Element with the given ID

    @param Node The Root of the subtree to search
    @param Key The Key to match
    @param f Function whch takes a templated Element argument and a Key to 
    determine whether that Node's Element is a match.

    @return Pointer to the found value, or NULL

    @discussion
    Call the given function on each Node's Element  as it traverses the Tree:
    @code
    template <typename R>
    template <typename T>
    template <typename K>
    R *f(const T &Element, const K &Key)
    {
        R *Val;
        if(<E satisfies the Key I'm looking for>)
        {
            Val = <some attribute of the Element, or the Element itself>;
            return Val;
        }
        else
            return NULL;
    }
    @endcode

    If the function determines that Element is a match, it returns the  Value, 
    otherwise it returns NULL to signal that it didn't match, so that the 
    traversal may continue.

    @warning
    The traversal functions should NOT be recursive and only operate on the 
    given Element.
    
    @note On Usage
    Since this function processes the given templated Element, you just need
    a unique identifier (represented in the Key typename), and your own code to
    process/compare the two.

    Example:
    Generally, it's easier to grab whatever data structure you're interested in
    and parse it after-the-fact, but because of the power of templates here,
    you can write very specific, powerful, reusable functions to do all of the
    dirty work in one step.

    Here's an example which searches a Window Stack for the AXUIElementRef of a
    Window given only its WID.

    @code
    AXUIElementRef *f(const window_stack &Stack, const int &WID)
    {
        std::vector<window_info*> Windows = Stack.Windows;
        std::vector<window_info*>::iterator WindowIt, end;
        for(WindowIt=Windows.begin(), end=Windows.end(); WindowIt != end; ++WindowIt)
        {
            if((*WindowIt)->WID == WID)
                return (*WindowIt)->Reference;
        }
        return NULL;
    }
    @endcode

    @note On Breaking Abstraction and Closures
    The intention here was to use the already defined traversal searching
    functions, but that would require overloading the existing ones to take
    function pointers with an additional Key parameter.

    IDEALLY this should have been accomplished with a closure, but that's only
    supported since C++11.

    @note
    This function is pre-order recursive.
 */
template <typename T, typename K, typename R>
R *GetElementByKey(const tree_node &Node, const K &Key, R *(*f)(const T &Element, const K &Key));

/*! Insert, Add, and Remove Elements
 */

/*! @brief Insert the given Element into the Tree at the specified Node

    @param Node (mutate) The Node to split into a Parent with a new Leaf
    @param Element The Element to insert at the given Node

    @return Whether or not the new Element was added to the Tree

    @pre Find the Node that you want to split to create a Leaf
    @post Any operations on the Element that are dependent on its position in
    the Tree need to be performed after insertion.

    @discussion
    Split the existing Node into a Parent Node with two Children.
    The Left Child will be the newly created Leaf from the Element.
    The Right Child will be the subtree specified by Node.

    Insertion can fail if the given Node does not exist (NULL ptr)

    @todo Add a switch to change the default left/right ordering for the new leaves
 */
template <typename T> bool InsertElementAtNode(tree_node *Node, const T &Element);

/*! @brief Add the given Element into the Tree

    @param RootNode (mutate) The Root of the Tree structure
    @param Element The Element to add

    @discussion
    By default, add the new Node at the top-left-most position.
    Perform a Left-to-Right Level-Order search for the first Leaf Node, and
    insert the Element at this location.

    @todo Add a configuration setting to change the default insertion location.

    @see InsertElementAtNode()
 */
template <typename T> void AddElementToTree(tree_node *RootNode, const T &Element);

/*! @brief Remove the given Element from the Tree

    @param RootNode (mutate) The Root of the Tree
    @param Element The Element to remove from the Tree

    @return Check whether the Element was removed
    
    @discussion
    Finds the corresponding Node in the Tree which contains the Element, and
    removes that Node from the Tree.

    Returns false if the Element was not found in the Tree.

    @post If return is false, calling function may need to check another Tree[.](http://i.imgur.com/6MxySx6.jpg)

    @see RemoveNodeFromTree()
 */
template <typename T> bool RemoveElementFromTree(tree_node *RootNode, const T &Element);

/*! @brief Swap the location of the Elements in the Tree

    @param RootNode (mutate) The Root of the Tree
    @param ElementOne The first Element to find
    @param ElementTwo The second Element to find

    @discussion
    Swaps the Elements in each Node to the respective other.

    @post
    Any attributes of the Element which are properties of its location within
    the Tree will need to be udpated in the calling scope after the swap.
 */
template <typename T> void SwapElementsInTree(tree_node *RootNode, const T &ElementOne, const T &ElementTwo);

/*! @functiongroup Mutations */

void RebalanceTree(tree_node *RootNode);
void RotateTree(tree_node *RootNode, const rotation_target &Rotation);
void FlipTree(tree_node *RootNode, const rotation_target &Rotation);
void PreOrderMutation(tree_node *RootNode, void (*f)(tree_node *Node));
void PostOrderMutation(tree_node *RootNode, void (*f)(tree_node *Node));
void InOrderMutation(tree_node *RootNode, void (*f)(tree_node *Node));
void LevelOrderMutation(tree_node *RootNode, void (*f)(tree_node *Node));

#endif
