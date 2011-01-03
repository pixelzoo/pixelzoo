#ifndef GAME_INCLUDED
#define GAME_INCLUDED

#include <time.h>
#include "board.h"
#include "tool.h"
#include "stringmap.h"
#include "xymap.h"
#include "goal.h"

/* default rates */
#define DefaultUpdatesPerSecond   100
#define DefaultGoalTestsPerSecond 1

/* power-up */
typedef struct ToolCharger ToolCharger;

/* entrance portal */
typedef struct EntrancePortal {
  XYCoord pos;
  State state;
  int total, soFar;  /* number of entryState's to place at entrancePos */
  double rate;
} EntrancePortal;

/* exit portal */
typedef struct ExitPortal {
  enum PortalState { PortalWaiting = 0,    /* the startgame: exit portal closed, i.e. ignoring incoming Particle's. Player must meet openGoal */
		     PortalCounting = 1,   /* the endgame: exit portal is "open" and counting incoming Particle's */
		     PortalUnlocked = 2,   /* the "win" outcome: exit portal count reached */
		     PortalDestroyed = 3   /* the "fail" outcome: aliveGoal not met */
  } portalState;

  Type type;
  int soFar;  /* number of type's that have exited the board */
  CellWatcher *watcher;  /* (Game*) context */
} ExitPortal;

/* state of play */
typedef struct Game {
  /* board */
  Board *board;
  enum GameState { GameOn = 0,       /* board is evolving, player can use tools */
		   GameWon = 1,      /* board is evolving, player can use tools, they've won (exit portal opened, etc) */
		   GameLost = 2,     /* board is evolving, player can't use tools because they've lost (the time limit has expired, etc) */
		   GamePaused = 3,   /* board not evolving, player can't use tools, can return to GameOn state (currently unimplemented) */
		   GameQuit = 4      /* game over, no way out of this state */
  } gameState;

  /* timing */
  double updatesPerSecond;     /* rate at which to run the Board. DO NOT MODIFY WHILE RUNNING - conversions to "Board time" depend on this being constant! */
  double goalTestsPerSecond;   /* rate at which to test Goal */
  clock_t lastGoalTestTime;

  /* toolbox */
  StringMap *toolByName;     /* all Tool's, including empty/locked; this is the owning container for Tool's */
  Tool *selectedTool;
  XYCoord toolPos;
  int toolActive;

  /* Game logic */
  Goal *goal;    /* results of testing this Goal are discarded; use PseudoGoal's to drive game state */

  /* entrance */
  EntrancePortal theEntrance;

  /* exit portal */
  ExitPortal theExit;

  /* power-ups */
  List *charger;  /* all ToolCharger's */

  /* dummy CellWatcher for write protects */
  CellWatcher *writeProtectWatcher;

} Game;

/* Game methods */
Game* newGame();
void deleteGame (Game *game);
void gameLoop (Game *game, double targetUpdatesPerCell, double maxFractionOfTimeInterval, double *actualUpdatesPerCell, int *actualUpdates, double *evolveTime);

#define gameRunning(GAME_PTR) ((GAME_PTR)->gameState == GameOn || (GAME_PTR)->gameState == GameWon || (GAME_PTR)->gameState == GameLost)
#define quitGame(GAME_PTR) { (GAME_PTR)->gameState = GameQuit; }

/* helpers */
void makeEntrances (Game *game);
void useTools (Game *game, double duration);  /* duration is measured in board time, i.e. updates per cell */
void updateGameState (Game *game);  /* tests win/lose conditions */

/* Types of CellWatcher: ExitPortal, ToolCharger and WriteProtect */
State exitPortalIntercept (CellWatcher *watcher, Board *board, int x, int y, State state);
State toolChargerIntercept (CellWatcher *watcher, Board *board, int x, int y, State state);
State writeProtectIntercept (CellWatcher *watcher, Board *board, int x, int y, State state);

struct ToolCharger {
  CellWatcher *watcher;
  Type overwriteType;  /* cell must be overwritten with this Type to get Tool bonus */
  Tool *tool;
};

ToolCharger* newToolCharger();
void deleteToolCharger (void* charger);

typedef Game* ExitPortalContext;
typedef ToolCharger* ToolChargerContext;

/*
  Game threads (all on timers):
  Judge thread: test win/lose conditions, end game or sleep
  Evolve thread: if not paused, use current selected tool (if active), recharge inactive tools, evolve board, recalculate overload, sleep
  Redraw thread: redraw board, sleep

  UI events:
  Key press (or toolbar touch): select current tool
  Mouse down (or board touch): set current tool active flag
  Mouse move (or board touch): set current tool x, y
  Mouse up (or board touch release): clear tool active flag
*/


#endif /* GAME_INCLUDED */
