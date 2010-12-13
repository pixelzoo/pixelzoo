#ifndef PLAY_STATE_INCLUDED
#define PLAY_STATE_INCLUDED

#include "board.h"
#include "tool.h"
#include "challenge.h"

typedef struct PlayState {
  Board *board;
  StringMap *stageChallenges, *toolBox;
  char *currentStage, *selectedTool;
} PlayState;

/*
  Game threads (all on timers):
  Evolve thread: evolve board, recalculate overload, sleep
  Redraw thread: redraw board, sleep
  Challenge thread: test goals -> award rewards, sleep
  Tool thread: use current selected tool (if active), recharge inactive tools, sleep

  UI events:
  Key press (or toolbar touch): select current tool
  Mouse down (or board touch): set current tool active flag
  Mouse move (or board touch): set current tool x, y
  Mouse up (or board touch release): clear tool active flag
 */


#endif /* PLAY_STATE_INCLUDED */
