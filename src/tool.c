#include <stdio.h>
#include "tool.h"
#include "util.h"
#include "stringmap.h"

/* by default, tool paints empty space onto empty space, i.e. does nothing */
#define DefaultToolOldState EmptyState
#define DefaultToolNewState EmptyState

Tool* newTool (char *name, int size) {
  Tool *tool;
  int x, y;
  tool = SafeMalloc (sizeof(Tool));
  tool->name = StringCopy (name);

  tool->brushIntensity = newQuadTree (size);
  for (x = 0; x < size; ++x)
    for (y = 0; y < size; ++y)
      updateQuadTree (tool->brushIntensity, x, y, 1.);
  tool->brushState = NULL;
  tool->defaultBrushState = DefaultToolNewState;
  tool->brushCenter.x = tool->brushCenter.y = size / 2;

  tool->overwriteDisallowLoc = NULL;
  tool->overwriteStates = NULL;
  tool->overwriteMask = TypeMask;

  tool->sprayRate = tool->rechargeRate = tool->reserve = tool->maxReserve = 1.;

  return tool;
}

void deleteTool (void *voidTool) {
  Tool *tool;
  tool = (Tool*) voidTool;
  deleteQuadTree (tool->brushIntensity);
  if (tool->brushState)
    deleteXYMap (tool->brushState);
  if (tool->overwriteDisallowLoc)
    deleteXYSet (tool->overwriteDisallowLoc);
  if (tool->overwriteStates)
    deleteStateSet (tool->overwriteStates);
  StringDelete (tool->name);
  SafeFree (tool);
}

void useTool (Tool *tool, Board *board, int x, int y, double duration) {
  int particles, xOffset, yOffset, xPaint, yPaint;
  State maskedOldState, newState;
  XYCoord xyTmp;
  XYMapNode *xyNode;
  particles = (int) (.5 + tool->sprayRate * duration);
  while (particles-- > 0 && topQuadRate(tool->brushIntensity) > 0. && tool->reserve > 0.) {
    sampleQuadLeaf (tool->brushIntensity, &xOffset, &yOffset);
    newState = tool->defaultBrushState;
    if (tool->brushState)
      if ((xyNode = XYMapFind(tool->brushState,x,y,xyTmp)))
	newState = *(State*)xyNode->value;
    xPaint = x + xOffset - tool->brushCenter.x;
    yPaint = y + yOffset - tool->brushCenter.y;
    if (onBoard (board, xPaint, yPaint)) {
      if (tool->overwriteDisallowLoc == NULL || XYSetFind (tool->overwriteDisallowLoc, xPaint, yPaint, xyTmp) == NULL) {
	maskedOldState = readBoardStateUnguarded(board,xPaint,yPaint) & tool->overwriteMask;
	if (tool->overwriteStates == NULL || StateSetFind (tool->overwriteStates, maskedOldState)) {
	  writeBoardStateUnguarded (board, xPaint, yPaint, newState);
	  tool->reserve = MAX (tool->reserve - 1, 0.);
	}
      }
    }
  }
}

void rechargeTool (Tool *tool, double duration) {
    tool->reserve = MIN (tool->reserve + tool->rechargeRate * duration, tool->maxReserve);
}

void printTool (void *voidTool) {
  Tool *tool;
  tool = (Tool*) voidTool;
  printf ("Tool %s : reserve %g / %g\n", tool->name, tool->reserve, tool->maxReserve);
}