//
//  SPDetailViewController.m
//  BigPlanets
//
//  Created by Quinn McHenry on 1/11/13.
//  Copyright (c) 2013 Small Planet Digital. All rights reserved.
//

#import "SPDetailViewController.h"

@interface SPDetailViewController ()
@property (strong, nonatomic) UIPopoverController *masterPopoverController;
- (void)configureView;
@end

@implementation SPDetailViewController

- (void)dealloc
{
    [_detailItem release];
    [_detailDescriptionLabel release];
    [_masterPopoverController release];
    [_detailRadiusLabel release];
    [_detailMassLabel release];
    [_detailVolumeLabel release];
    [_detailRingsLabel release];
    [_detailMoonsLabel release];
    [super dealloc];
}

#pragma mark - Managing the detail item

- (void)setDetailItem:(id)newDetailItem
{
    if (_detailItem != newDetailItem) {
        [_detailItem release];
        _detailItem = [newDetailItem retain];

        // Update the view.
        [self configureView];
    }

    if (self.masterPopoverController != nil) {
        [self.masterPopoverController dismissPopoverAnimated:YES];
    }        
}

- (void)configureView
{
    // Update the user interface for the detail item.

    if (self.detailItem) {
        self.detailDescriptionLabel.text = [self.detailItem name];
        self.detailMassLabel.text = [self.detailItem massAsString];
        self.detailRadiusLabel.text = [self.detailItem equatorialRadiusAsString];
        self.detailVolumeLabel.text = [self.detailItem estimatedVolumeAsString];
        self.detailRingsLabel.text = [NSString stringWithFormat:@"%@\n \n ",[self.detailItem hasRingsAnswerString]];

        // list the moon names
        if([[self.detailItem Moons] count] > 0) {
            NSString* moonList = @"";
            for(Galaxy_Moon* moon in [self.detailItem Moons])
                moonList = [moonList stringByAppendingString:[NSString stringWithFormat:@"%@, ",[moon name]]];

            self.detailMoonsLabel.text = [NSString stringWithFormat:@"%@",[moonList stringByReplacingCharactersInRange:NSMakeRange([moonList length]-2, 2) withString:@""]];
        }
        else
            self.detailMoonsLabel.text = @"No moons.";
    }
}

- (void)viewDidLoad
{
    [super viewDidLoad];
	// Do any additional setup after loading the view, typically from a nib.
    [self configureView];
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

#pragma mark - Split view

- (void)splitViewController:(UISplitViewController *)splitController willHideViewController:(UIViewController *)viewController withBarButtonItem:(UIBarButtonItem *)barButtonItem forPopoverController:(UIPopoverController *)popoverController
{
    barButtonItem.title = NSLocalizedString(@"Master", @"Master");
    [self.navigationItem setLeftBarButtonItem:barButtonItem animated:YES];
    self.masterPopoverController = popoverController;
}

- (void)splitViewController:(UISplitViewController *)splitController willShowViewController:(UIViewController *)viewController invalidatingBarButtonItem:(UIBarButtonItem *)barButtonItem
{
    // Called when the view is shown again in the split view, invalidating the button and popover controller.
    [self.navigationItem setLeftBarButtonItem:nil animated:YES];
    self.masterPopoverController = nil;
}

@end
