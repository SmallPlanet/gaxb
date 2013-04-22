//
//  SPDetailViewController.h
//  BigPlanets
//
//  Created by Quinn McHenry on 1/11/13.
//  Copyright (c) 2013 Small Planet Digital. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "Galaxy_XMLLoader.h"

@interface SPDetailViewController : UIViewController <UISplitViewControllerDelegate>

@property (strong, nonatomic) Galaxy_Planet *detailItem;

@property (strong, nonatomic) IBOutlet UILabel *detailDescriptionLabel;
@property (retain, nonatomic) IBOutlet UILabel *detailMassLabel;
@property (retain, nonatomic) IBOutlet UILabel *detailRadiusLabel;
@property (retain, nonatomic) IBOutlet UILabel *detailVolumeLabel;
@property (retain, nonatomic) IBOutlet UILabel *detailRingsLabel;
@property (retain, nonatomic) IBOutlet UILabel *detailMoonsLabel;


@end
