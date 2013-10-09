//
//  PZAppDelegate.h
//  PixelZoo
//
//  Created by Ian Holmes on 10/4/13.
//  Copyright (c) 2013 Holmesian Software. All rights reserved.
//

#import <UIKit/UIKit.h>

#import "PZWorldsViewController.h"

@interface PZAppDelegate : UIResponder <UIApplicationDelegate, NSURLConnectionDelegate>
{
    PZWorldsViewController* worldsViewController;
    NSURLConnection *worldListConnection;
    NSMutableData *worldListResponseData;
}

@property (strong, nonatomic) UIWindow *window;

@end
