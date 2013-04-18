//
//  SPMasterViewController.h
//  BigPlanets
//
//  Created by Quinn McHenry on 1/11/13.
//  Copyright (c) 2013 Small Planet Digital. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "Planets_XMLLoader.h"

@class SPDetailViewController;

@interface SPMasterViewController : UITableViewController
{
    Planets_SolarSystem *solarSystem;
}

@property (strong, nonatomic) SPDetailViewController *detailViewController;
@property (nonatomic, retain) Planets_SolarSystem *solarSystem;

@end
