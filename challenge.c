#include "challenge.h"

Challenge* newChallenge() {
  Challenge* chal;
  chal = SafeMalloc (sizeof (Challenge));
  chal->goal = NULL;
  chal->stage = newStringSet();
  chal->nextStage = chal->tool = chal->achievement = chal->rewardText = chal->npcSleep = chal->npcWake = NULL;
  chal->scoreDelta = NULL;
  chal->goalTestRate = 1.;
  chal->delayBetweenAwards = 0.;
  chal->timesAwarded = 0;
  chal->maxTimesAwarded = 1;
  return chal;
}

void deleteChallenge (Challenge *challenge) {
  SafeFreeOrNull (challenge->stage);
  SafeFreeOrNull (challenge->rewardText);
  SafeFreeOrNull (challenge->tool);
  SafeFreeOrNull (challenge->nextStage);
  if (challenge->scoreDelta)
    deleteStringMap (challenge->scoreDelta);
  if (challenge->goal)
    deleteGoal (challenge->goal);
  deleteStringSet (challenge->stage);
  SafeFree (challenge);
}
