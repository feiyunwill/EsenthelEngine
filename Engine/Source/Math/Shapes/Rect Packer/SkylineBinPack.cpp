/** @file SkylineBinPack.cpp
	@author Jukka Jylänki

	@brief Implements different bin packer algorithms that use the SKYLINE data structure.

	This work is released to Public Domain, do whatever you want with it.
*/
#include "SkylineBinPack.h"

SkylineBinPack::SkylineBinPack()
:binWidth(0),
binHeight(0),
allowRotate(false)
{
}

SkylineBinPack::SkylineBinPack(int width, int height, bool useWasteMap, bool allowRotate)
{
	Init(width, height, useWasteMap, allowRotate);
}

void SkylineBinPack::Init(int width, int height, bool useWasteMap_, bool allowRotate_)
{
	binWidth = width;
	binHeight = height;
	useWasteMap = useWasteMap_;
   allowRotate = allowRotate_;

#ifdef DEBUG
	disjointRects.Clear();
#endif

	usedSurfaceArea = 0;
	skyLine.clear();
	SkylineNode node;
	node.x = 0;
	node.y = 0;
	node.width = binWidth;
	skyLine.add(node);

	if (useWasteMap)
	{
		wasteMap.Init(width, height, allowRotate);
		wasteMap.GetFreeRectangles().clear();
	}
}

void SkylineBinPack::Insert(Memp<RectSizeIndex> rects, Memp<RectIndex> dst, LevelChoiceHeuristic method)
{
	dst.clear();

	while(rects.elms() > 0)
	{
		Rect bestNode;
		int bestScore1 = INT_MAX;
		int bestScore2 = INT_MAX;
		int bestSkylineIndex = -1;
		int bestRectIndex = -1;
		for(size_t i = 0; i < rects.elms(); ++i)
		{
			Rect newNode;
			int score1;
			int score2;
			int index;
			switch(method)
			{
			case LevelBottomLeft:
				newNode = FindPositionForNewNodeBottomLeft(rects[i].width, rects[i].height, score1, score2, index);
				debug_assert(disjointRects.Disjoint(newNode));
				break;
			case LevelMinWasteFit:
				newNode = FindPositionForNewNodeMinWaste(rects[i].width, rects[i].height, score2, score1, index);
				debug_assert(disjointRects.Disjoint(newNode));
				break;
			default: assert(false); break;
			}
			if (newNode.height != 0)
			{
				if (score1 < bestScore1 || (score1 == bestScore1 && score2 < bestScore2))
				{
					bestNode = newNode;
					bestScore1 = score1;
					bestScore2 = score2;
					bestSkylineIndex = index;
					bestRectIndex = i;
				}
			}
		}

		if (bestRectIndex == -1)
			return;

		// Perform the actual packing.
		debug_assert(disjointRects.Disjoint(bestNode));
#ifdef DEBUG
		disjointRects.Add(bestNode);
#endif
		AddSkylineLevel(bestSkylineIndex, bestNode);
		usedSurfaceArea += rects[bestRectIndex].width * rects[bestRectIndex].height;

      RectIndex &dest=dst.New();
      SCAST(Rect, dest)=bestNode;
      dest.index=rects[bestRectIndex].index;
      rects.remove(bestRectIndex);
	}
}

Rect SkylineBinPack::Insert(int width, int height, LevelChoiceHeuristic method)
{
	// First try to pack this rectangle into the waste map, if it fits.
	Rect node = wasteMap.Insert(width, height, true, GuillotineBinPack::RectBestShortSideFit, 
		GuillotineBinPack::SplitMaximizeArea);
	debug_assert(disjointRects.Disjoint(node));

	if (node.height != 0)
	{
		Rect newNode;
		newNode.x = node.x;
		newNode.y = node.y;
		newNode.width = node.width;
		newNode.height = node.height;
		usedSurfaceArea += width * height;
		debug_assert(disjointRects.Disjoint(newNode));
#ifdef DEBUG
		disjointRects.Add(newNode);
#endif
		return newNode;
	}
	
	switch(method)
	{
	case LevelBottomLeft: return InsertBottomLeft(width, height);
	case LevelMinWasteFit: return InsertMinWaste(width, height);
	default: assert(false); return node;
	}
}

bool SkylineBinPack::RectangleFits(int skylineNodeIndex, int width, int height, int &y) const
{
	int x = skyLine[skylineNodeIndex].x;
	if (x + width > binWidth)
		return false;
	int widthLeft = width;
	int i = skylineNodeIndex;
	y = skyLine[skylineNodeIndex].y;
	while(widthLeft > 0)
	{
		y = Max(y, skyLine[i].y);
		if (y + height > binHeight)
			return false;
		widthLeft -= skyLine[i].width;
		++i;
		assert(i < (int)skyLine.elms() || widthLeft <= 0);
	}
	return true;
}

int SkylineBinPack::ComputeWastedArea(int skylineNodeIndex, int width, int height, int y) const
{
	int wastedArea = 0;
	const int rectLeft = skyLine[skylineNodeIndex].x;
	const int rectRight = rectLeft + width;
	for(; skylineNodeIndex < (int)skyLine.elms() && skyLine[skylineNodeIndex].x < rectRight; ++skylineNodeIndex)
	{
		if (skyLine[skylineNodeIndex].x >= rectRight || skyLine[skylineNodeIndex].x + skyLine[skylineNodeIndex].width <= rectLeft)
			break;

		int leftSide = skyLine[skylineNodeIndex].x;
		int rightSide = Min(rectRight, leftSide + skyLine[skylineNodeIndex].width);
		assert(y >= skyLine[skylineNodeIndex].y);
		wastedArea += (rightSide - leftSide) * (y - skyLine[skylineNodeIndex].y);
	}
	return wastedArea;
}

bool SkylineBinPack::RectangleFits(int skylineNodeIndex, int width, int height, int &y, int &wastedArea) const
{
	bool fits = RectangleFits(skylineNodeIndex, width, height, y);
	if (fits)
		wastedArea = ComputeWastedArea(skylineNodeIndex, width, height, y);
	
	return fits;
}

void SkylineBinPack::AddWasteMapArea(int skylineNodeIndex, int width, int height, int y)
{
	const int rectLeft = skyLine[skylineNodeIndex].x;
	const int rectRight = rectLeft + width;
	for(; skylineNodeIndex < (int)skyLine.elms() && skyLine[skylineNodeIndex].x < rectRight; ++skylineNodeIndex)
	{
		if (skyLine[skylineNodeIndex].x >= rectRight || skyLine[skylineNodeIndex].x + skyLine[skylineNodeIndex].width <= rectLeft)
			break;

		int leftSide = skyLine[skylineNodeIndex].x;
		int rightSide = Min(rectRight, leftSide + skyLine[skylineNodeIndex].width);
		assert(y >= skyLine[skylineNodeIndex].y);

		Rect waste;
		waste.x = leftSide;
		waste.y = skyLine[skylineNodeIndex].y;
		waste.width = rightSide - leftSide;
		waste.height = y - skyLine[skylineNodeIndex].y;

		debug_assert(disjointRects.Disjoint(waste));
		wasteMap.GetFreeRectangles().add(waste);
	}
}

void SkylineBinPack::AddSkylineLevel(int skylineNodeIndex, const Rect &rect)
{
	// First track all wasted areas and mark them into the waste map if we're using one.
	if (useWasteMap)
		AddWasteMapArea(skylineNodeIndex, rect.width, rect.height, rect.y);

	SkylineNode newNode;
	newNode.x = rect.x;
	newNode.y = rect.y + rect.height;
	newNode.width = rect.width;
	skyLine.NewAt(skylineNodeIndex)=newNode;

	assert(newNode.x + newNode.width <= binWidth);
	assert(newNode.y <= binHeight);

	for(size_t i = skylineNodeIndex+1; i < skyLine.elms(); ++i)
	{
		assert(skyLine[i-1].x <= skyLine[i].x);

		if (skyLine[i].x < skyLine[i-1].x + skyLine[i-1].width)
		{
			int shrink = skyLine[i-1].x + skyLine[i-1].width - skyLine[i].x;

			skyLine[i].x += shrink;
			skyLine[i].width -= shrink;

			if (skyLine[i].width <= 0)
			{
				skyLine.remove(i, true);
				--i;
			}
			else
				break;
		}
		else
			break;
	}
	MergeSkylines();
}

void SkylineBinPack::MergeSkylines()
{
	for(size_t i = 0; i < skyLine.elms()-1; ++i)
		if (skyLine[i].y == skyLine[i+1].y)
		{
			skyLine[i].width += skyLine[i+1].width;
			skyLine.remove(i+1, true);
			--i;
		}
}

Rect SkylineBinPack::InsertBottomLeft(int width, int height)
{
	int bestHeight;
	int bestWidth;
	int bestIndex;
	Rect newNode = FindPositionForNewNodeBottomLeft(width, height, bestHeight, bestWidth, bestIndex);

	if (bestIndex != -1)
	{
		debug_assert(disjointRects.Disjoint(newNode));
		// Perform the actual packing.
		AddSkylineLevel(bestIndex, newNode);

		usedSurfaceArea += width * height;
#ifdef DEBUG
		disjointRects.Add(newNode);
#endif
	}
	else
		memset(&newNode, 0, sizeof(Rect));

	return newNode;
}

Rect SkylineBinPack::FindPositionForNewNodeBottomLeft(int width, int height, int &bestHeight, int &bestWidth, int &bestIndex) const
{
	bestHeight = INT_MAX;
	bestIndex = -1;
	// Used to break ties if there are nodes at the same level. Then pick the narrowest one.
	bestWidth = INT_MAX;
	Rect newNode;
	memset(&newNode, 0, sizeof(newNode));
	for(size_t i = 0; i < skyLine.elms(); ++i)
	{
		int y;
		if (RectangleFits(i, width, height, y))
		{
			if (y + height < bestHeight || (y + height == bestHeight && skyLine[i].width < bestWidth))
			{
				bestHeight = y + height;
				bestIndex = i;
				bestWidth = skyLine[i].width;
				newNode.x = skyLine[i].x;
				newNode.y = y;
				newNode.width = width;
				newNode.height = height;
				debug_assert(disjointRects.Disjoint(newNode));
			}
		}
		if (allowRotate && RectangleFits(i, height, width, y))
		{
			if (y + width < bestHeight || (y + width == bestHeight && skyLine[i].width < bestWidth))
			{
				bestHeight = y + width;
				bestIndex = i;
				bestWidth = skyLine[i].width;
				newNode.x = skyLine[i].x;
				newNode.y = y;
				newNode.width = height;
				newNode.height = width;
				debug_assert(disjointRects.Disjoint(newNode));
			}
		}
	}

	return newNode;
}

Rect SkylineBinPack::InsertMinWaste(int width, int height)
{
	int bestHeight;
	int bestWastedArea;
	int bestIndex;
	Rect newNode = FindPositionForNewNodeMinWaste(width, height, bestHeight, bestWastedArea, bestIndex);

	if (bestIndex != -1)
	{
		debug_assert(disjointRects.Disjoint(newNode));
		// Perform the actual packing.
		AddSkylineLevel(bestIndex, newNode);

		usedSurfaceArea += width * height;
#ifdef DEBUG
		disjointRects.Add(newNode);
#endif
	}
	else
		memset(&newNode, 0, sizeof(newNode));

	return newNode;
}

Rect SkylineBinPack::FindPositionForNewNodeMinWaste(int width, int height, int &bestHeight, int &bestWastedArea, int &bestIndex) const
{
	bestHeight = INT_MAX;
	bestWastedArea = INT_MAX;
	bestIndex = -1;
	Rect newNode;
	memset(&newNode, 0, sizeof(newNode));
	for(size_t i = 0; i < skyLine.elms(); ++i)
	{
		int y;
		int wastedArea;

		if (RectangleFits(i, width, height, y, wastedArea))
		{
			if (wastedArea < bestWastedArea || (wastedArea == bestWastedArea && y + height < bestHeight))
			{
				bestHeight = y + height;
				bestWastedArea = wastedArea;
				bestIndex = i;
				newNode.x = skyLine[i].x;
				newNode.y = y;
				newNode.width = width;
				newNode.height = height;
				debug_assert(disjointRects.Disjoint(newNode));
			}
		}
		if (allowRotate && RectangleFits(i, height, width, y, wastedArea))
		{
			if (wastedArea < bestWastedArea || (wastedArea == bestWastedArea && y + width < bestHeight))
			{
				bestHeight = y + width;
				bestWastedArea = wastedArea;
				bestIndex = i;
				newNode.x = skyLine[i].x;
				newNode.y = y;
				newNode.width = height;
				newNode.height = width;
				debug_assert(disjointRects.Disjoint(newNode));
			}
		}
	}

	return newNode;
}

/// Computes the ratio of used surface area.
float SkylineBinPack::Occupancy() const
{
	return (float)usedSurfaceArea / (binWidth * binHeight);
}
