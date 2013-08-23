//
//  pixelzooAppDelegate.h
//  pixelzoo
//
//  Created by Ian Holmes on 12/28/10.
//

#import <UIKit/UIKit.h>

@class pixelzooViewController;

@interface pixelzooAppDelegate : NSObject <UIApplicationDelegate> {
    UIWindow *window;
    pixelzooViewController *viewController;
}

@property (nonatomic, retain) IBOutlet UIWindow *window;
@property (nonatomic, retain) IBOutlet pixelzooViewController *viewController;

@end

