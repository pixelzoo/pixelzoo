//
//  pixelzooDefs.h
//  pixelzoo
//
//  Created by Ian Holmes on 8/22/13.
//

#ifndef pixelzoo_PZDefs_h
#define pixelzoo_PZDefs_h

// PixelZoo server
#ifndef SERVER_URL_PREFIX
#define SERVER_URL_PREFIX [NSString stringWithFormat:@"http://%@:%@", [[[NSBundle mainBundle] infoDictionary] objectForKey:@"SERVER_HOST"], [[[NSBundle mainBundle] infoDictionary] objectForKey:@"SERVER_PORT"]]
#endif


// PixelZoo game constants
// #define GAME_XML_FILENAME "testgame"
#define GAME_XML_FILENAME "simple"
#define MAX_PROPORTION_TIME_EVOLVING  .9
#define REDRAWS_PER_SECOND 30   /* frame rate */
#define GAMELOOP_CALLS_PER_SECOND REDRAWS_PER_SECOND    /* for some reason, increasing this slows updates down; maybe need a separate thread? */

// isometric view
#define TOOL_ICON_WIDTH 64
#define TOOL_ICON_HEIGHT 64
#define TOOL_RESERVE_HEIGHT 4

#define TILE_SPRITE_HEIGHT 32  /* natural height for a (flat) tile sprite (a cube will be twice this height) */
#define MIN_TILE_HEIGHT_FOR_SPRITES 12  /* threshold for showing sprites instead of solid cubes */
#define MAX_NATURAL_TILE_HEIGHT 32 /* max scale at which board image will actually be rendered */
#define MAX_TILE_HEIGHT 32  /* max scale at which board image will be shown */

#define PAN_MARGIN 20
#define PAN_RATE 5

#define TUTORIAL_WORLD_NAME "Tutorial"

#endif
