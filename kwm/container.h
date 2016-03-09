/*
 *  container.h
 */

#ifndef __CONTAINER__
#define __CONTAINER__

#include "types.h"

/*! 
    @header container.h
    @brief Functions that operate on node_container objects.
 */

/*!
 *  @brief Update the global KWMScreen.SplitRatio variable
 *
 *  @var KWMScreen.SplitRatio
 *
 *  todo: Move to top-level configuration.
 *
 *  @param Value the new SplitRatio to use when creating node_containers
 *
 */
void ChangeSplitRatio(const double &Value);
 // TODO: Move this to a top-level configuration header.

/*!
    @functiongroup Constructors
 */

/*!
 *  @brief Create a left split child `node_container`
 *
 *  @discussion Uses the parent Container's SplitRatio and Boundary 
 *  information to create a subcontainer split to the left.
 *
 *  @param Offset    The gaps to use when creating the node_container
 *  @param Container Parent node_container to base the Container on
 *
 *  @return pointer to new split sub-container
 * 
 *  @see
 *  RightVerticalContainerSplit()
 *  UpperHorizontalContainerSplit()
 *  LowerHorizontalContainerSplit()
 *
 *  @note This function calls `new`
 */
node_container *LeftVerticalContainerSplit(const container_offset &Offset, const node_container &Container);

/*!
 *  @brief Create a right split child `node_container`
 *
 *  @discussion Uses the parent Container's SplitRatio and Boundary 
 *  information to create a subcontainer split to the right.
 *
 *  @param Offset    The gaps to use when creating the `node_container`
 *  @param Container Parent `node_container` to base the Container on
 *
 *  @return pointer to new split sub-container
 * 
 *  @see
 *  LeftVerticalContainerSplit()
 *  UpperHorizontalContainerSplit()
 *  LowerHorizontalContainerSplit()
 *
 *  @note This function calls `new`
 *
 */
node_container *RightVerticalContainerSplit(const container_offset &Offset, const node_container &Container);

/*!
 *  @brief Create an upper split child `node_container`
 *
 *  @discussion Uses the parent Container's SplitRatio and Boundary 
 *  information to create a subcontainer split in the upper portion.
 *
 *  @param Offset    Gaps to use when creating the `node_container`
 *  @param Container Parent `node_container` to inherit attributes of
 *
 *  @return Pointer to new split sub-container
 * 
 *  @see
 *  LeftVerticalContainerSplit
 *  RightVerticalContainerSplit
 *  LowerHorizontalContainerSplit
 *
 *  @note This function calls `new`
 *
 */
node_container *UpperHorizontalContainerSplit(const container_offset &Offset, const node_container &Container);

/*!
 *  @brief Create a lower split child `node_container`
 *
 *  @discussion Uses the parent Container's SplitRatio and Boundary 
 *  information to create a subcontainer split to the left.
 *
 *  @param Offset    Gaps to use when creating the `node_container`
 *  @param Container Parent `node_container` to inherit attributes of
 *
 *  @return Pointer to new split sub-container
 * 
 *  @see
 *  LeftVerticalContainerSplit
 *  RightVerticalContainerSplit
 *  UpperHorizontalContainerSplit
 *
 *  @note This function calls `new`
 */
node_container *LowerHorizontalContainerSplit(const container_offset &Offset, const node_container &Container);

/*!
 *  @brief Create a split subcontainer from the ParentContainer
 *
 *  @param Offset          Gaps to use when creating the `node_container`
 *  @param ParentContainer `node_container` that new container inherits attributes from
 *  @param ContainerType   how to split the ParentContainer when creating the new Container
 *
 *  @var KWMScreen.SplitRatio (global) Default the SplitRatio to (if inherited a value not between 0.0 and 1.0)
 *
 *  @return Pointer to new split sub-container
 *
 *  @see
 *  LeftVerticalContainerSplit
 *  RightVerticalContainerSplit
 *  UpperHorizontalContainerSplit
 *  LowerHorizontalContainerSplit
 */
node_container *CreateSplitContainer(const container_offset &Offset, const node_container &ParentContainer, const container_type &ContainerType);

/*!
 *  @brief Create a new `node_container` for a full Space, with no splits.
 *
 *  @param Boundary Rectangular boundary that the container fills
 *
 *  @var KWMScreen.SpltRatio (global) Default split ratio to use for the Container
 *
 *  @return Pointer to new container
 *
 *  @note This function calls `new`
 *
 */
node_container *CreateRootContainer(const bound_rect &Boundary);

/*!
    @functiongroup Mutators
 */

/*!
 *  @brief Set the SplitRatio of the Container
 *
 *  @param Container
 *  @param Ratio     New SplitRatio (between 0.0 and 1.0)
 *
 *  @return (mutate) Container->SplitRatio attribute
 *
 *  @note The new SplitRatio will only affect Containers created after setting
 *  this value -- existing Containers will not be affected.
 *
 *  @note If Ratio is not between 0.0 and 1.0 (exclusive), the value will not
 *  be set.
 */
void SetContainerSplitRatio(node_container *Container, const double &Ratio);

/*!
 *  @brief Adjust the SplitRatio of the Container by an amount
 *
 *  @param Container (mutate) the Container whose SplitRatio is to be adjusted.
 *  @param Delta     the amount to modify the SplitRatio (0.0 < Delta < 1.0)
 *
 *  @return (mutate) Container->SplitRatio is adjusted by Delta
 *
 *  @note The new SplitRatio will only affect Containers created after setting
 *  this value -- existing Containers will not be affected
 *  @note This function will fail to modify the SplitRatio if the result is not
 *  between 0.0 and 1.0 (exclusive).
 */
void AdjustContainerSplitRatio(node_container *Container, const double &Delta);

/*!
 *  @brief Toggle the SplitMode of the Container between SplitModeVertical and SplitModeHorizontal
 *
 *  @param Container (mutate) modify Container->SplitMode
 *
 *  @note The new SplitMode will only affect Containers created after setting
 *  this value -- existing Containers will not be affected.
 *  @note This function can fail if Container->SplitMode isn't already either
 *  SplitModeVertical or SplitModeHorizontal.
 */
void ToggleContainerSplitMode(node_container *Container);

/*!
    @functiongroup Queries
 */

/*!
 *  @brief Check whether a point is within a Container
 *
 *  @param Container Use the Container's Boundary rect as the bounds
 *  @param X         X position (pixels)
 *  @param Y         Y position (pixels)
 *
 *  @return true if point (X,Y) is within the Container.
 */
bool IsPointInContainer(const node_container &Container, const int &X, const int &Y);

/*!
    @functiongroup Helpers
 */

/*!
 *  @brief Determine the optimal split mode from the dimensions of the rectangle
 *
 *  @param Rect The bounding box
 *
 *  @return SplitModeVertical or SplitModeHorizontal, depending on aspect ratio
 *
 *  @discussion
 *  @todo Default aspect ratio check is the Golden Ratio, should be configurable.
 */
split_mode GetOptimalSplitMode(const bound_rect &Rect);

#endif
