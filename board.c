#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "board.h"
#include "notify.h"

Board* newBoard (int size) {
  Board *board;
  int x;
  board = SafeMalloc (sizeof (Board));
  board->byType = SafeCalloc (NumTypes, sizeof(Particle*));
  board->size = size;
  board->cell = SafeMalloc (size * sizeof(State*));
  board->sync = SafeMalloc (size * sizeof(State*));
  for (x = 0; x < size; ++x) {
    board->cell[x] = SafeCalloc (size, sizeof(State));
    board->sync[x] = SafeCalloc (size, sizeof(State));
  }
  board->syncQuad = newQuadTree (size);
  board->asyncQuad = newQuadTree (size);
  board->syncUpdateQuad = newQuadTree (size);
  board->syncParticles = 0;
  board->syncFiringRateAfterLastSync = 0.;
  board->overloadThreshold = SafeMalloc ((board->syncQuad->K + 1) * sizeof(double));
  for (x = 0; x <= board->syncQuad->K; ++x)
    board->overloadThreshold[x] = 1.;
  board->updatesPerCell = 0.;
  board->syncUpdates = 0;

  initializePalette (&board->palette);

  return board;
}

void deleteBoard (Board* board) {
  unsigned long t;
  int x;
  SafeFree(board->overloadThreshold);
  deleteQuadTree (board->syncUpdateQuad);
  deleteQuadTree (board->syncQuad);
  deleteQuadTree (board->asyncQuad);
  for (x = 0; x < board->size; ++x) {
    SafeFree(board->cell[x]);
    SafeFree(board->sync[x]);
  }
  SafeFree(board->cell);
  SafeFree(board->sync);
  for (t = 0; t < NumTypes; ++t)
    if (board->byType[(Type) t])
      deleteParticle (board->byType[(Type) t]);
  SafeFree(board->byType);
  SafeFree(board);
}

void writeBoardStateUnguarded (Board* board, int x, int y, State state) {
  Type t;
  Particle *p, *pOld;
  /* get new Type & Particle, and old Particle */
  t = StateType(state);
  p = board->byType[t];
  pOld = readBoardParticleUnguarded(board,x,y);
  /* decrement old count */
  if (pOld) {
    --pOld->count;
    if (pOld->synchronous)
      --board->syncParticles;
  }
  /* update cell array & quad trees */
  if (p == NULL) {
    board->cell[x][y] = board->sync[x][y] = (t == EmptyType) ? state : EmptyState;  /* handle the EmptyType specially */
    updateQuadTree (board->asyncQuad, x, y, 0.);
    updateQuadTree (board->syncQuad, x, y, 0.);
  } else {
    board->cell[x][y] = board->sync[x][y] = state;
    updateQuadTree (board->asyncQuad, x, y, p->asyncFiringRate);
    updateQuadTree (board->syncQuad, x, y, p->syncFiringRate);
    /* update new count */
    ++p->count;
    if (p->synchronous)
      ++board->syncParticles;
  }
  /* TODO: check for BoardWatcher's, call appropriate ParticleNotifyFunction(s) */
}

void writeSyncBoardStateUnguarded (Board* board, int x, int y, State state) {
  board->sync[x][y] = state;
}

PaletteIndex readBoardColor (Board* board, int x, int y) {
  Particle *p;
  State s;
  Type t;
  PaletteIndex c;
  c = 0;  /* default to black */
  s = readBoardState (board, x, y);
  t = StateType(s);
  if (t == EmptyType)
    c = s & PaletteMask;  /* hard-wired shortcut for empties */
  else {
    p = board->byType[t];
    if (p)
      c = getParticleColor (p, s);
  }
  return c;
}

Particle* newBoardParticle (Board* board, char* name, Type type, int nRules) {
  Particle* p;
  p = newParticle (name, nRules);
  p->type = type;
  board->byType[type] = p;
  return p;
}

void addParticleToBoard (Particle* p, Board* board) {
  int r, n, m;
  StochasticRule *rule;
  board->byType[p->type] = p;
  p->totalRate = p->totalOverloadRate = 0.;
  for (r = 0; r < p->nRules; ++r) {
    rule = &p->rule[r];
    p->totalRate += rule->rate;
    p->totalOverloadRate += rule->overloadRate;
    for (n = 1; n < NumRuleOperations; ++n)
      for (m = n; m > 0; --m) {
	if (rule->op[n].src.x == rule->op[n-m].dest.x
	    && rule->op[n].src.y == rule->op[n-m].dest.y)
	  rule->cumulativeOpSrcIndex[n] = m;
	if (rule->op[n].dest.x == rule->op[n-m].dest.x
	    && rule->op[n].dest.y == rule->op[n-m].dest.y)
	  rule->cumulativeOpDestIndex[n] = m;
      }
  }
  p->syncFiringRate = p->synchronous ? 1. : 0.;
  p->asyncFiringRate = p->synchronous ? 0. : MIN (p->totalRate, 1.);
}

int testRuleCondition (RuleCondition* cond, Board* board, int x, int y, int overloaded) {
  State lhs, rhs;
  if (randomDouble() < (overloaded ? cond->overloadIgnoreProb : cond->ignoreProb))
    return 1;
  x += cond->loc.x;
  y += cond->loc.y;
  lhs = readBoardState(board,x,y) & cond->mask;
  rhs = cond->rhs;
  switch (cond->opcode) {
  case EQ: return lhs == rhs;
  case NEQ: return lhs != rhs;
  case GT: return lhs > rhs;
  case LT: return lhs < rhs;
  case GEQ: return lhs >= rhs;
  case LEQ: return lhs <= rhs;
  case TRUE: return 1;
  case FALSE: default: break;
  }
  return 0;
}

State execRuleOperation (RuleOperation* op, Board* board, int x, int y, State oldSrcState, State oldDestState, int overloaded, BoardWriteFunction write) {
  State newState;
  if (randomDouble() < (overloaded ? op->overloadFailProb : op->failProb))
    return oldDestState;
  x += op->dest.x;
  y += op->dest.y;
  if (onBoard(board,x,y)) {  /* only check once */
    newState = (oldDestState & (StateMask ^ (op->mask << op->leftShift)))
      | (((((oldSrcState & op->preMask) >> op->rightShift) + op->offset) & op->mask) << op->leftShift);
    (*write) (board, x, y, newState);
    return newState;
  }
  return oldDestState;
}

void evolveBoardCell (Board* board, int x, int y) {
  Particle* p;
  int n, overloaded;
  double rand;
  StochasticRule* rule;
  p = readBoardParticle (board, x, y);
  if (p) {
    /*
      Assert (!p->synchronous, "evolveBoardCell called on async particle");
    */
    overloaded = boardOverloaded (board, x, y);
    /* sample a rule at random */
    rand = randomDouble() * (overloaded ? p->totalOverloadRate : p->totalRate);
    for (n = 0; n < p->nRules; ++n) {
      rule = &p->rule[n];
      if ((rand -= (overloaded ? rule->overloadRate : rule->rate)) <= 0) {
	(void) attemptRule (rule, board, x, y, overloaded, writeBoardStateUnguarded);
	return;
      }
    }
  }
}

void evolveBoardCellSync (Board* board, int x, int y) {
  Particle* p;
  int n, swap, overloaded, *ruleOrder;
  double rand, remainingRate, ruleRate;
  StochasticRule* rule;
  /* do an update */
  p = readBoardParticle (board, x, y);
  if (p && p->synchronous) {
    overloaded = boardOverloaded (board, x, y);
    /* attempt each rule in random sequence, stopping when one succeeds */
    ruleOrder = SafeMalloc (p->nRules * sizeof(int));  /* weighted Fisher-Yates shuffle */
    for (n = 0; n < p->nRules; ++n)
      ruleOrder[n] = n;
    remainingRate = (overloaded ? p->totalOverloadRate : p->totalRate);
    for (n = 0; n < p->attempts; ++n) {
      if (p->shuffle) {
	rand = randomDouble() * remainingRate;
	for (swap = n; 1; ++swap) {
	  rule = &p->rule[ruleOrder[swap]];
	  ruleRate = (overloaded ? rule->overloadRate : rule->rate);
	  rand -= ruleRate;
	  if (rand < 0. || swap == p->nRules - 1)
	    break;
	}
	ruleOrder[swap] = ruleOrder[n];
	remainingRate -= ruleRate;
      } else {
	rule = &p->rule[n];
	ruleRate = (overloaded ? rule->overloadRate : rule->rate);
	if (randomDouble() > ruleRate)
	  continue;
      }
      if (attemptRule (rule, board, x, y, overloaded, writeSyncBoardStateUnguarded))
	break;
    }
    SafeFree (ruleOrder);
  }
}

void syncBoard (Board* board) {
  int x, y, size;
  State *cellCol, *syncCol;
  size = board->size;
  /* update only the cells that changed */
  for (x = 0; x < size; ++x) {
    cellCol = board->cell[x];
    syncCol = board->sync[x];
    for (y = 0; y < size; ++y)
      if (syncCol[y] != cellCol[y])
	writeBoardStateUnguarded (board, x, y, syncCol[y]);
  }
  /* freeze the update queue */
  copyQuadTree (board->syncQuad, board->syncUpdateQuad);
  board->syncFiringRateAfterLastSync = boardSyncFiringRate(board);
}

int boardOverloaded (Board* board, int x, int y) {
  int n;
  for (n = 0; n <= board->syncQuad->K; ++n)
    if (boardLocalFiringRate(board,x,y,n) > board->overloadThreshold[n])
      return 1;
  return 0;
}

int attemptRule (StochasticRule* rule, Board* board, int x, int y, int overloaded, BoardWriteFunction write) {
  int k, mSrc, mDest;
  RuleCondition *cond;
  RuleOperation *op;
  State intermediateState[NumRuleOperations], oldSrcState, oldDestState;
  for (k = 0; k < NumRuleConditions; ++k) {
    cond = &rule->cond[k];
    if (!testRuleCondition (cond, board, x, y, overloaded))
      return 0;
  }
  for (k = 0; k < NumRuleOperations; ++k) {
    op = &rule->op[k];
    mSrc = rule->cumulativeOpSrcIndex[k];
    mDest = rule->cumulativeOpDestIndex[k];
    oldSrcState = mSrc ? intermediateState[k - mSrc] : getRuleOperationOldSrcState(op,board,x,y);
    oldDestState = mDest ? intermediateState[k - mDest] : getRuleOperationOldDestState(op,board,x,y);
    intermediateState[k] = execRuleOperation (op, board, x, y, oldSrcState, oldDestState, overloaded, write);
  }
  return 1;
}

void evolveBoard (Board* board, double targetUpdatesPerCell, double maxTimeInSeconds, double* updateRate_ret, double* minUpdateRate_ret) {
  int actualUpdates, x, y;
  double effectiveUpdates, targetUpdates, elapsedClockTime, syncRate, asyncRate, totalRate;
  clock_t start, now;

  /* start the clocks */
  start = clock();
  actualUpdates = 0;
  effectiveUpdates = elapsedClockTime = 0.;
  targetUpdates = targetUpdatesPerCell * boardCells(board);

  /* main loop */
  while (1) {

    /* check if realtime clock deadline reached */
    now = clock();
    elapsedClockTime = ((double) now - start) / (double) CLOCKS_PER_SEC;
    if (elapsedClockTime > maxTimeInSeconds)
      break;

    /* check if target update count reached */
    if (effectiveUpdates >= targetUpdates) {
      effectiveUpdates = targetUpdates;
      break;
    }

    /* randomly do an asynchronous update, or process a synchronous one from the queue.
       We aim to do one actual update (nearly) every time we go through this loop,
       spending time on async/sync cells that is proportionate to their representation on the board.
       However, every single "actual" async update corresponds to an "effective" number of async updates N, where N>=1
       (due to empty space, low-firing rules, etc.)
       Thus, whenever we do an actual async update, we update the board clocks accordingly.
     */

    /* calculate the sync & async rates */
    syncRate = (boardSyncFiringRate(board) + board->syncFiringRateAfterLastSync) / 2.;
    asyncRate = boardAsyncFiringRate(board);
    totalRate = syncRate + asyncRate;

    /* update the clocks */
    if (totalRate > 0)
      effectiveUpdates += (double) 1. / totalRate;
    else
      effectiveUpdates += boardCells(board);

    /* decide: sync or async? */
    if (randomDouble() * totalRate < syncRate) {

      /* sync */
      if (topQuadRate (board->syncUpdateQuad) > 0.) {  /* any more synchronized cells on the queue? */

	/* randomly process a pending synchronized cell update */
	sampleQuadLeaf (board->syncUpdateQuad, &x, &y);
	updateQuadTree (board->syncUpdateQuad, x, y, 0.);

	evolveBoardCellSync (board, x, y);
	++actualUpdates;

      } else {
	/* no more sync cells on queue, so flush all synchronized updates to board */
	syncBoard (board);
	++board->syncUpdates;
      }

    } else {
      /* async */
      if (topQuadRate(board->asyncQuad) > 0.) {  /* any asynchronous cells on the queue? */
	/* evolve a random cell */
	sampleQuadLeaf (board->asyncQuad, &x, &y);
	evolveBoardCell (board, x, y);
	++actualUpdates;
      }
    }

  }
  /* update board time */
  board->updatesPerCell += effectiveUpdates / boardCells(board);
  /* calculate update rates */
  if (updateRate_ret)
    *updateRate_ret = (elapsedClockTime > 0) ? (effectiveUpdates / elapsedClockTime) : 0.;
  if (minUpdateRate_ret)
    *minUpdateRate_ret = (elapsedClockTime > 0) ? (actualUpdates / elapsedClockTime) : 0.;
}
