//
//  SPMasterViewController.h
//  BigPlanets
//
//  Created by Quinn McHenry on 1/11/13.
//  Copyright (c) 2013 Small Planet Digital. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "Galaxy_XMLLoader.h"

@class SPDetailViewController;

@interface SPMasterViewController : UITableViewController
{
    Galaxy_StarSystem *starSystem;
}

@property (strong, nonatomic) SPDetailViewController *detailViewController;
@property (nonatomic, retain) Galaxy_StarSystem *starSystem;

@end
