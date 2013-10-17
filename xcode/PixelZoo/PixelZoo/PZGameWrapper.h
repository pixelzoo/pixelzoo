//
//  PZGameWrapper.h
//  pixelzoo
//
//  Created by Ian Holmes on 10/10/13.
//  Copyright (c) 2013 Holmesian Software. All rights reserved.
//

#import <Foundation/Foundation.h>

#import "pixelzoo.h"
#import "GDataXMLNode.h"
#import "PZWorldDescriptor.h"
#import "PZLockDescriptor.h"


@interface PZGameWrapper : NSObject <NSURLConnectionDelegate> {
    NSURLConnection* turnConnection;
    bool deleteLockWhenTurnPosted;
}

@property (strong, nonatomic) PZWorldDescriptor *worldDescriptor;
@property (strong, nonatomic) PZLockDescriptor *lockDescriptor;

@property (nonatomic) pzGame *game;
@property (nonatomic) long long lastSavedBoardClock;

// initialization

-(void)initGameFromXMLString:(NSString*)xmlString;
-(void)initGameFromLock:(PZLockDescriptor*)lock;
-(bool)isInitialized;

// saving turns

-(void)postTurn;
-(void)postTurnAndDeleteLock;

-(bool)turnSaved;

// pixelzoo.h wrapper functions

-(void)updateGame;
-(int)boardSize;
-(long long)boardClock;

-(int)numberOfTools;
-(void)selectTool:(int)n;
-(void)unselectTool;
-(int)selectedToolNumber;

-(void)untouchCell;
-(void)touchCellAtX:(int)x y:(int)y;

-(int)cellRgbAtX:(int)x y:(int)y;
-(const char*)cellNameAtX:(int)x y:(int)y;
-(int)cellNameRgbAtX:(int)x y:(int)y;

-(int)toolRgbByNumber:(int)n;
-(const char*)toolNameByNumber:(int)n;
-(CGFloat)toolReserveByNumber:(int)n;

-(int)numberOfConsoleLines;
-(const char*)consoleText:(int)line;

-(int)numberOfBalloons;
-(pzBalloon)balloonNumber:(int)n;
-(int)textRgbForBalloon:(pzBalloon)b;


@end