//
//  PZAppDelegate.m
//  PixelZoo
//
//  Created by Ian Holmes on 10/4/13.
//  Copyright (c) 2013 Holmesian Software. All rights reserved.
//

#import "PZAppDelegate.h"
#import "PZDefs.h"
#import "GDataXMLNode.h"
#import "PZWorldDescriptor.h"

@implementation PZAppDelegate

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
    // Override point for customization after application launch.

    UINavigationController *navigationController = (UINavigationController*) self.window.rootViewController;
    PZWorldsViewController* worldsViewController = (PZWorldsViewController*) [[navigationController viewControllers] objectAtIndex:0];
    
    // get world list
    
    // Send a synchronous request
    NSURLRequest * urlRequest = [NSURLRequest requestWithURL:[NSURL URLWithString:[NSString stringWithFormat:@"%@/world/list",@SERVER_URL_PREFIX]]];
    NSURLResponse * response = nil;
    NSError * error = nil;
    NSData * data = [NSURLConnection sendSynchronousRequest:urlRequest
                                          returningResponse:&response
                                                      error:&error];
    
    if (error == nil)
    {
        // parse data using GDataXMLDocument, use xpath to extract list of <world>...</world> elements as NSArray
        
        GDataXMLDocument *doc = [[GDataXMLDocument alloc] initWithData:data
                                                               options:0 error:&error];
        
        worldsViewController.doc = doc;

        NSArray *worldNodes = [doc nodesForXPath:@"//world-list/world" error:nil];
        NSMutableArray *worldDescriptors = [[NSMutableArray alloc] init];
        for (int i = 0; i < [worldNodes count]; ++i)
            [worldDescriptors addObject:[[PZWorldDescriptor alloc] initWithNode:[worldNodes objectAtIndex:i]]];
        
        // put worlds NSArray in PZWorldTableViewController
        worldsViewController.worlds = worldDescriptors;
    }
    
    return YES;
}
							
- (void)applicationWillResignActive:(UIApplication *)application
{
    // Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
    // Use this method to pause ongoing tasks, disable timers, and throttle down OpenGL ES frame rates. Games should use this method to pause the game.
}

- (void)applicationDidEnterBackground:(UIApplication *)application
{
    // Use this method to release shared resources, save user data, invalidate timers, and store enough application state information to restore your application to its current state in case it is terminated later. 
    // If your application supports background execution, this method is called instead of applicationWillTerminate: when the user quits.
}

- (void)applicationWillEnterForeground:(UIApplication *)application
{
    // Called as part of the transition from the background to the inactive state; here you can undo many of the changes made on entering the background.
}

- (void)applicationDidBecomeActive:(UIApplication *)application
{
    // Restart any tasks that were paused (or not yet started) while the application was inactive. If the application was previously in the background, optionally refresh the user interface.
}

- (void)applicationWillTerminate:(UIApplication *)application
{
    // Called when the application is about to terminate. Save data if appropriate. See also applicationDidEnterBackground:.
}

@end
