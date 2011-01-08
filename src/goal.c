#include <stdio.h>
#include <math.h>
#include "goal.h"
#include "tool.h"
#include "game.h"

/* private function prototypes */
int testEntropyGoal (Goal* goal, Board *board);
int testEnclosuresGoal (Goal* goal, Board *board);
Goal* newGoal (enum GoalType type, int dblDataSize, int intDataSize);

/* function definitions */
Goal* newGoal (enum GoalType type, int dblDataSize, int intDataSize) {
  Goal* g;
  g = SafeMalloc (sizeof (Goal));
  g->goalType = type;
  g->l = g->r = g->parent = NULL;
  g->tree = NULL;
  g->dblData = dblDataSize ? SafeMalloc(dblDataSize*sizeof(double)) : NULL;
  g->intData = intDataSize ? SafeMalloc(intDataSize*sizeof(unsigned long)) : NULL;
  g->context = NULL;
  return g;
}

Goal* newTrueGoal() {
  return newGoal (TrueGoal, 0, 0);
}

Goal* newFalseGoal() {
  return newGoal (FalseGoal, 0, 0);
}

Goal* newAreaGoal (XYSet* area, Goal *subGoal) {
  Goal* g;
  g = newGoal (AreaGoal, 0, 0);
  g->tree = (RBTree*) area;
  g->l = subGoal;
  return g;
}

Goal* newEnclosuresGoal (State wallMask,
			 StateSet* wallSet,
			 unsigned long minNumEnclosures,
			 unsigned long maxNumEnclosures,
			 unsigned long minEnclosureArea,
			 unsigned long maxEnclosureArea,
			 unsigned char allowDiagonalConnections,
			 Goal* subGoal) {
  Goal* g;
  g = newGoal (EnclosuresGoal, 0, 6);
  g->tree = (RBTree*) wallSet;
  g->intData[0] = wallMask;
  g->intData[1] = allowDiagonalConnections;
  g->intData[2] = minEnclosureArea;
  g->intData[3] = maxEnclosureArea;
  g->intData[4] = minNumEnclosures;
  g->intData[5] = maxNumEnclosures;
  g->l = subGoal;
  return g;
}

Goal* newCachedGoal (Goal* l, int reps) {
  Goal* g;
  g = newGoal (CachedGoal, 0, 2);
  g->intData[0] = reps;
  g->intData[1] = 0;
  g->l = l;
  return g;
}

Goal* newAndGoal (Goal* l, Goal* r, int lazy) {
  Goal* g;
  g = newGoal (lazy ? LazyAndGoal : AndGoal, 0, 0);
  g->l = l;
  g->r = r;
  return g;
}

Goal* newOrGoal (Goal* l, Goal* r, int lazy) {
  Goal* g;
  g = newGoal (lazy ? LazyOrGoal : OrGoal, 0, 0);
  g->l = l;
  g->r = r;
  return g;
}

Goal* newNotGoal (Goal* l) {
  Goal* g;
  g = newGoal (NotGoal, 0, 0);
  g->l = l;
  return g;
}

Goal* newEntropyGoal (StateSet* typeSet, State stateMask, unsigned long minCount, unsigned long maxCount, double minEntropy, double maxEntropy) {
  Goal* g;
  g = newGoal (EntropyGoal, 2, 3);
  g->tree = (RBTree*) typeSet;
  g->intData[0] = stateMask;
  g->intData[1] = minCount;
  g->intData[2] = maxCount;
  g->dblData[0] = minEntropy;
  g->dblData[1] = maxEntropy;
  return g;
}

Goal* newRepeatGoal (Goal* subGoal, unsigned long minReps) {
  Goal* g;
  g = newGoal (RepeatGoal, 0, 1);
  g->intData[0] = minReps;
  g->l = subGoal;
  return g;
}

Goal *newBoardTimeGoal (double minUpdatesPerCell, double maxUpdatesPerCell) {
  Goal *g;
  g = newGoal (BoardTimeGoal, 2, 0);
  g->dblData[0] = minUpdatesPerCell;
  g->dblData[1] = maxUpdatesPerCell;
  return g;
}

Goal *newCheckToolGoal (void *tool, double minReserve, double maxReserve) {
  Goal *g;
  g = newGoal (CheckToolGoal, 2, 0);
  g->dblData[0] = minReserve;
  g->dblData[1] = maxReserve;
  g->context = tool;
  return g;
}

Goal *newCheckPortalGoal (void *portal, int portalState, int minCount, int maxCount) {
  Goal *g;
  g = newGoal (CheckPortalGoal, 0, 3);
  g->intData[0] = portalState;
  g->intData[1] = minCount;
  g->intData[2] = maxCount;
  g->context = portal;
  return g;
}

Goal *newCheckGameStateGoal (int gameState) {
  Goal *g;
  g = newGoal (CheckGameStateGoal, 0, 1);
  g->intData[0] = gameState;
  return g;
}

Goal *newChargeToolPseudoGoal (void *tool, double reserveDelta) {
  Goal *g;
  g = newGoal (ChargeToolPseudoGoal, 1, 0);
  g->dblData[0] = reserveDelta;
  g->context = tool;
  return g;
}

Goal *newSetPortalStatePseudoGoal (void *portal, int portalState) {
  Goal *g;
  g = newGoal (SetPortalStatePseudoGoal, 0, 1);
  g->intData[0] = portalState;
  g->context = portal;
  return g;
}

Goal *newSetGameStatePseudoGoal (int gameState) {
  Goal *g;
  g = newGoal (SetGameStatePseudoGoal, 0, 1);
  g->intData[0] = gameState;
  return g;
}

Goal *newUseToolPseudoGoal (void *tool, double duration) {
  Goal *g;
  g = newGoal (UseToolPseudoGoal, 1, 0);
  g->dblData[0] = duration;
  g->context = tool;
  return g;
}

Goal *newPrintMessagePseudoGoal (const char* message) {
  Goal *g;
  g = newGoal (PrintMessagePseudoGoal, 0, 0);
  g->context = (void*) message;
  return g;
}

void deleteGoal (Goal* goal) {
  SafeFreeOrNull (goal->intData);
  SafeFreeOrNull (goal->dblData);
  if (goal->tree) deleteRBTree (goal->tree);
  if (goal->l) deleteGoal (goal->l);
  if (goal->r) deleteGoal (goal->r);
  SafeFree (goal);
}

List* getEnclosures (Board* board, XYSet* area, State wallMask, StateSet* wallSet, unsigned int minEnclosureArea, unsigned int maxEnclosureArea, unsigned char allowDiagonalConnections) {
  List *enclosureList;
  XYList *enclosure, *pointsToVisit;
  int **mark, xLoop, yLoop, x, y, dx, dy, currentMark, enclosureDone, enclosureArea;
  XYCoord tempXYCoord;
  State state;

  /* enclosureList is a List of List's (or more specifically, a list of XYList's),
     so the copy, delete & print functions are the List versions of those functions
  */
  enclosureList = newList (ListDeepCopyVoid, ListDeleteVoid, ListPrintVoid);

  /* create an array of enclosure indices */
  mark = SafeMalloc (board->size * sizeof(int*));
  for (x = 0; x < board->size; ++x)
    mark[x] = SafeCalloc (board->size, sizeof(int));

  /* mark the walls as -1 */
  for (x = 0; x < board->size; ++x)
    for (y = 0; y < board->size; ++y) {
      state = readBoardState(board,x,y) & wallMask;
      if (StateSetFind(wallSet,state))
	mark[x][y] = -1;
    }

  /* if we've been given a sub-area of the board to look at, mark everything outside that area as -1 */
  if (area)
    for (x = 0; x < board->size; ++x)
      for (y = 0; y < board->size; ++y)
	if (XYSetFind(area,x,y,tempXYCoord) == NULL)
	  mark[x][y] = -1;

  /* loop over the board, starting a breadth-first search from every unvisited cell */
  pointsToVisit = newXYList();
  currentMark = 0;
  for (xLoop = 0; xLoop < board->size; ++xLoop)
    for (yLoop = 0; yLoop < board->size; ++yLoop)
      if (mark[xLoop][yLoop] == 0) {
	++currentMark;
	enclosure = newXYList();

	x = xLoop;
	y = yLoop;
	enclosureDone = 0;
	while (!enclosureDone) {
	  mark[x][y] = currentMark;
	  (void) XYListAppend (enclosure, x, y);

	  /* loop over the neighborhood */
	  for (dx = -1; dx <= +1; ++dx)
	    for (dy = -1; dy <= +1; ++dy)
	      if ((dx || dy) && (allowDiagonalConnections || (dx == 0 || dy == 0)))
		if (onBoard(board,x+dx,y+dy) && mark[x+dx][y+dy] == 0)
		  (void) XYListAppend (pointsToVisit, x+dx, y+dy);

	  while (mark[x][y] != 0) {
	    if (XYListEmpty (pointsToVisit)) {
	      enclosureDone = 1;
	      break;
	    }
	    XYListPop (pointsToVisit, x, y);
	  }
	}

	enclosureArea = XYListSize (enclosure);
	if (enclosureArea >= minEnclosureArea && enclosureArea <= maxEnclosureArea)
	  ListAppend (enclosureList, (void*) enclosure);
	else
	  deleteXYList (enclosure);
      }

  /* cleanup & return */
  deleteXYList (pointsToVisit);
  for (x = 0; x < board->size; ++x)
    SafeFree (mark[x]);
  SafeFree (mark);

  return enclosureList;
}

XYSet* getGoalArea (Goal* goal) {
  return
    goal->goalType == AreaGoal
    ? (XYSet*) RBTreeShallowCopy (goal->tree)
    : (goal->parent
       ? getGoalArea (goal->parent)
       : NULL);
}

int testGoalMet (Goal* goal, void *voidGame) {
  Game *game;
  Board *board;
  int lGoalMet, rGoalMet, soFar, areaPoints, randPointIndex, x, y;
  Tool *tool;
  XYSet *area;
  XYSetNode *areaNode;
  XYCoord *pos;

  Assert (goal != NULL, "testGoalMet: null goal");
  game = (Game*) voidGame;
  board = game->board;

  switch (goal->goalType) {
  case AreaGoal:
    return goal->l ? testGoalMet(goal->l,game) : 1;

  case EnclosuresGoal:
    return testEnclosuresGoal (goal, board);
    return 1;

  case CachedGoal:
    if (goal->intData[1] >= goal->intData[0])
      return 1;
    lGoalMet = testGoalMet(goal->l,game);
    if (lGoalMet)
      ++goal->intData[1];
    return lGoalMet;

  case AndGoal:
    lGoalMet = testGoalMet(goal->l,game);
    rGoalMet = testGoalMet(goal->r,game);
    return lGoalMet && rGoalMet;

  case OrGoal:
    lGoalMet = testGoalMet(goal->l,game);
    rGoalMet = testGoalMet(goal->r,game);
    return lGoalMet || rGoalMet;

  case LazyAndGoal:
    return testGoalMet(goal->l,game) ? testGoalMet(goal->r,game) : 0;

  case LazyOrGoal:
    return testGoalMet(goal->l,game) ? 1 : testGoalMet(goal->r,game);

  case NotGoal:
    return !testGoalMet(goal->l,game);

  case EntropyGoal:
    return testEntropyGoal (goal, board);

  case RepeatGoal:
    if (testGoalMet(goal->l,game))
      ++goal->intData[1];
    else
      goal->intData[1] = 0;
    return goal->intData[1] >= goal->intData[0];

  case BoardTimeGoal:
    return goal->dblData[0] <= board->updatesPerCell && (goal->dblData[1] <= 0. || board->updatesPerCell <= goal->dblData[1]);

  case CheckToolGoal:
    tool = (Tool*) goal->context;
    return goal->dblData[0] <= tool->reserve && tool->reserve <= goal->dblData[1];

  case CheckPortalGoal:
    soFar = ((ExitPortal*) goal->context)->soFar;
    return ((ExitPortal*) goal->context)->portalState == goal->intData[0] && goal->intData[1] <= soFar && (goal->intData[2] <= 0 || soFar <= goal->intData[2]);

  case CheckGameStateGoal:
    return game->gameState == goal->intData[0];

  case ChargeToolPseudoGoal:
    tool = (Tool*) goal->context;
    tool->reserve = MAX (tool->reserve + goal->dblData[0], tool->maxReserve);
    tool->hidden = 0;
    return 1;

  case SetPortalStatePseudoGoal:
    ((ExitPortal*) goal->context)->portalState = goal->intData[0];
    return 1;

  case SetGameStatePseudoGoal:
    game->gameState = goal->intData[0];
    return 1;

  case UseToolPseudoGoal:
    /* sample a point from the parent area. This is a slightly clumsy/inefficient implementation, but that shouldn't matter much here */
    area = getGoalArea (goal);
    if (area) {
      areaPoints = RBTreeSize (area);
      randPointIndex = randomInt (areaPoints);
      areaNode = RBTreeFirst (area);
      while (randPointIndex-- > 0)
	areaNode = RBTreeSuccessor (area, areaNode);
      pos = (XYCoord*) areaNode->key;
      x = pos->x;
      y = pos->y;
      deleteXYSet (area);
    } else {
      x = randomInt (board->size);
      y = randomInt (board->size);
    }
    useTool ((Tool*) goal->context, board, x, y, goal->dblData[0]);
    return 1;

  case PrintMessagePseudoGoal:
    printToGameConsole (game, (char*) goal->context, PaletteWhite, 1.);
    printf ("%s\n", (char*) goal->context);
    return 1;

  case TrueGoal:
    return 1;

  case FalseGoal:
  default:
    break;
  }

  return 0;
}

int testEnclosuresGoal (Goal* goal, Board *board) {
  List *enclosureList;
  ListNode *enclosureListNode, *enclosureNode;
  XYList *enclosure;
  XYSet *pointSet, *parentArea;
  Goal *tempAreaGoal;
  long wallMask, allowDiagonals, minEncSize, maxEncSize, minCount, maxCount, count;
  wallMask = goal->intData[0];
  allowDiagonals = goal->intData[1];
  minEncSize = goal->intData[2];
  maxEncSize = goal->intData[3];
  minCount = goal->intData[4];
  maxCount = goal->intData[5];
  parentArea = getGoalArea (goal);
  enclosureList = getEnclosures (board, parentArea, wallMask, (StateSet*) goal->tree, minEncSize, maxEncSize, allowDiagonals);
  count = 0;
  for (enclosureListNode = enclosureList->head; enclosureListNode && count < minCount; enclosureListNode = enclosureListNode->next) {
    enclosure = (XYList*) enclosureListNode->value;
    pointSet = newXYSet();
    for (enclosureNode = enclosure->head; enclosureNode; enclosureNode = enclosureNode->next)
      (void) XYSetInsert (pointSet, ((XYCoord*)enclosureNode->value)->x, ((XYCoord*)enclosureNode->value)->y);
    if (goal->l == NULL)
      ++count;
    else {
      tempAreaGoal = newAreaGoal (pointSet, NULL);
      goal->l->parent = tempAreaGoal;
      if (testGoalMet (goal->l, board))
	++count;
      goal->l->parent = goal;  /* this is not really necessary, but what the heck */
      deleteGoal (tempAreaGoal);  /* this also deletes pointSet */
    }
  }
  deleteList (enclosureList);
  if (parentArea)
    deleteXYSet (parentArea);
  return count >= minCount && (maxCount == 0 || count <= maxCount);
}

int testEntropyGoal (Goal* goal, Board *board) {
  XYSet *parentArea;
  int x, y, population;
  StateOffset minPopulation, maxPopulation;
  double prob, entropy, minEntropy, maxEntropy;
  StateSet *allowedTypes;
  StateMap *stateCount;
  StateMapNode *stateCountNode;
  State state, type, stateMask, maskedState;  /* NB we store type as a State, because allowedTypes is a StateSet */
  Stack *stateCountEnum;
  stateMask = goal->intData[0];
  minPopulation = goal->intData[1];
  maxPopulation = goal->intData[2];
  minEntropy = goal->dblData[0];
  maxEntropy = goal->dblData[1];
  allowedTypes = (StateSet*) goal->tree;
  /* count state types */
  stateCount = newStateMap(IntCopy,IntDelete,IntPrint);
  population = 0;
  parentArea = getGoalArea (goal);
  for (x = 0; x < board->size; ++x)
    for (y = 0; y < board->size; ++y) {
      state = readBoardStateUnguarded(board,x,y);
      type = StateType (state);
      if (StateMapFind (allowedTypes, type)) {
	maskedState = state & stateMask;
	stateCountNode = StateMapFind (stateCount, maskedState);
	if (stateCountNode)
	  ++*(int*)stateCountNode->value;
	else
	  (void) StateMapInsert (stateCount, maskedState, IntNew(1));
	++population;
      }
    }
  /* calculate entropy */
  entropy = 0.;
  stateCountEnum = RBTreeEnumerate (stateCount, NULL, NULL);
  while ((stateCountNode = StackPop (stateCountEnum))) {
    prob = (double) *(int*)stateCountNode->value / (double) population;
    entropy -= prob * log(prob);
  }
  entropy /= log(2);
  /* cleanup & return */
  deleteStack (stateCountEnum);
  deleteStateMap (stateCount);
  if (parentArea)
    deleteXYSet (parentArea);
  return population >= minPopulation
    && (maxPopulation == 0 || population <= maxPopulation)
    && entropy >= minEntropy
    && (maxEntropy <= 0 || entropy <= maxEntropy);
}
