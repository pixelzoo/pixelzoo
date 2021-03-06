#include <string.h>
#include "notify.h"
#include "xmlgame.h"
#include "xmlboard.h"
#include "xmlutil.h"

/* method defs */
Game* newGameFromXmlString (const char* string) {
  xmlDoc* doc;
  Game* game = NULL;
  doc = xmlTreeFromString (string);
  Assert (doc != NULL, "XML string not parsed");
  game = newGameFromXmlDocument (doc);
  return game;
}

Game* newGameFromXmlDocument (xmlDoc *doc) {
  return newGameFromXmlRoot (xmlDocGetRootElement (doc));
}

Game* newGameFromXmlRoot (xmlNode *gameNode) {
  return newGameFromXmlRootWithSeparateBoard (gameNode, gameNode);
}

Game* newGameFromXmlDocumentWithSeparateBoard (xmlDoc *gameDoc, xmlDoc *separateBoardDoc) {
  return newGameFromXmlRootWithSeparateBoard (xmlDocGetRootElement (gameDoc),
					      xmlDocGetRootElement (separateBoardDoc));
}

Game* newGameFromXmlStringWithSeparateBoard (const char* gameString, const char* boardString) {
  xmlDoc *doc, *boardDoc;
  Game* game = NULL;
  doc = xmlTreeFromString (gameString);
  Assert (doc != NULL, "Game XML string not parsed");
  boardDoc = boardString==gameString ? doc : xmlTreeFromString (boardString);
  Assert (boardDoc != NULL, "Board XML string not parsed");
  game = newGameFromXmlDocumentWithSeparateBoard (doc, boardDoc);
  deleteXmlTree (doc);
  if (boardDoc != doc)
    deleteXmlTree (boardDoc);
  return game;
}

Game* newGameFromXmlRootWithSeparateBoard (xmlNode *gameNode, xmlNode *separateBoardRoot) {
  Game *game;
  xmlNode *node, *globalOverwriteNode, *protectNode, *schemeNode;
  Tool *tool, *selectedTool;

  game = newGame();
  game->board = newBoardFromXmlRoot (separateBoardRoot);

  game->ticksPerSecond = OPTCHILDFLOAT (gameNode, RATE, DefaultTicksPerSecond);

  globalOverwriteNode = CHILD(gameNode,TOOLSET);
  if (globalOverwriteNode && (schemeNode = CHILD(globalOverwriteNode,SCHEME)))
    globalOverwriteNode = protoTableExpandSchemeNode (game->board->protoTable, schemeNode, globalOverwriteNode, gameNode);

  selectedTool = NULL;
  for (node = gameNode->children; node; node = node->next)
  if (MATCHES(node,TOOL)) {
    selectedTool = tool = newToolFromXmlNode (node, game->board->depth - 1, globalOverwriteNode, game->board->protoTable);
    addToolToGame (game, tool);
  }

  if ( (protectNode = CHILD(gameNode,PROTECT)) ) {
    if ( (schemeNode = CHILD(protectNode,SCHEME)) )
      protectNode = protoTableExpandSchemeNode (game->board->protoTable, schemeNode, protectNode, gameNode);
    for (node = protectNode->children; node; node = node->next)
      if (MATCHES(node,CELL))
	registerCellWatcher (game->board, CHILDINT(node,X), CHILDINT(node,Y), CHILDINT(node,Z), game->writeProtectWatcher);
  }

  return game;
}

Tool* newToolFromXmlNode (xmlNode* toolNode, int defaultZ, xmlNode* globalOverwriteNode, ProtoTable *protoTable) {
  Tool *tool;
  xmlNode *brushNode, *intensityNode, *patternNode, *overwriteNode, *node, *schemeNode, *iconNode;
  int x, y, size;
  const char *evalResult;
  xmlNode *evalNode;

  if ( (schemeNode = CHILD (toolNode, SCHEME)) ) {  /* assignment intentional */

    evalResult = protoTableEvalSxml (protoTable, (const char*) getNodeContent(schemeNode));
    evalNode = xmlTreeFromString (evalResult);

    tool = newToolFromXmlNode (evalNode, defaultZ, globalOverwriteNode, protoTable);

    deleteXmlTree (evalNode);
    StringDelete ((void*) evalResult);

  } else {
    size = CHILDINT(toolNode,SIZE);
    tool = newTool ((char*) CHILDSTRING(toolNode,NAME), size);
    if ((iconNode = CHILD(toolNode,ICON)))
      tool->icon = StringNew((const char*)NODESTRINGVAL(iconNode));
    tool->z = OPTCHILDINT(toolNode,Z,defaultZ);
    if ((brushNode = CHILD(toolNode,BRUSH))) {
      tool->isStamp = OPTCHILDINT(brushNode,STAMP,0);
      if ((node = CHILD(brushNode,CENTER))) {
	tool->brushCenter.x = CHILDINT(node,X);
	tool->brushCenter.y = CHILDINT(node,Y);
      }
      if ((intensityNode = CHILD(brushNode,INTENSITY))) {
	for (x = 0; x < size; ++x)
	  for (y = 0; y < size; ++y)
	    updateQuadTree (tool->brushIntensity, x, y, 0.);
	for (node = intensityNode->children; node; node = node->next)
	  if (MATCHES(node,SPOT))
	    updateQuadTree (tool->brushIntensity, CHILDINT(node,X), CHILDINT(node,Y), OPTCHILDINT(node,RATE,1.));
      }
      if ((patternNode = CHILD(brushNode,PATTERN))) {
	tool->brushState = newXYMap (copyState, deleteState, printState);
	for (node = patternNode->children; node; node = node->next)
	  if (MATCHES(node,PIXEL))
	    (void) XYMapInsert (tool->brushState, CHILDINT(node,X), CHILDINT(node,Y), newState(getStateFromNode(node,protoTable,0)));
      }
    }
    if (tool->brushState == NULL)
      tool->defaultBrushState = getStateFromNode(toolNode,protoTable,0);
    overwriteNode = CHILD(toolNode,OVERWRITE);
    if ((overwriteNode && CHILD(overwriteNode,DISALLOW))
	 || (globalOverwriteNode && CHILD(globalOverwriteNode,DISALLOW)))
      tool->overwriteDisallowLoc = newXYSet();
    if (globalOverwriteNode)
      for (node = globalOverwriteNode->children; node; node = node->next)
	if (MATCHES(node,DISALLOW))
	  (void) XYSetInsert (tool->overwriteDisallowLoc, CHILDINT(node,X), CHILDINT(node,Y));
    if (overwriteNode) {
      for (node = overwriteNode->children; node; node = node->next)
	if (MATCHES(node,DISALLOW))
	  (void) XYSetInsert (tool->overwriteDisallowLoc, CHILDINT(node,X), CHILDINT(node,Y));
      if (testNodeHasState(overwriteNode)) {
	tool->overwriteStates = newStateSet();
	for (node = nextNodeWithState(overwriteNode->children); node; node = nextNodeWithState(node->next))
	  (void) StateSetInsert (tool->overwriteStates, getStateFromChild (node, protoTable, 0));
	tool->overwriteMask = getMaskFromNode(overwriteNode,protoTable,TypeMask);
      }
    }
    tool->sprayRate = OPTCHILDFLOAT(toolNode,SPRAY,1.);
    tool->reserve = OPTCHILDFLOAT(toolNode,RESERVE,1.);
    tool->rechargeRate = OPTCHILDFLOAT(toolNode,RECHARGE,0.);
    tool->maxReserve = OPTCHILDFLOAT(toolNode,MAXRESERVE,tool->reserve);
  }

  return tool;
}
