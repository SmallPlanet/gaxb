//
//  SPMasterViewController.m
//  BigPlanets
//
//  Created by Quinn McHenry on 1/11/13.
//  Copyright (c) 2013 Small Planet Digital. All rights reserved.
//

#import "SPMasterViewController.h"

#import "SPDetailViewController.h"

@interface SPMasterViewController () {
    NSMutableArray *_objects;
}
@end

@implementation SPMasterViewController

@synthesize starSystem;

- (void)awakeFromNib
{
    if ([[UIDevice currentDevice] userInterfaceIdiom] == UIUserInterfaceIdiomPad) {
        self.clearsSelectionOnViewWillAppear = NO;
        self.contentSizeForViewInPopover = CGSizeMake(320.0, 600.0);
    }
    [super awakeFromNib];
}

- (void)dealloc
{
    [_detailViewController release];
    [_objects release];
    [starSystem release]; starSystem = nil;
    [super dealloc];
}

- (void)viewDidLoad
{
    [super viewDidLoad];
	// Do any additional setup after loading the view, typically from a nib.
    self.detailViewController = (SPDetailViewController *)[[self.splitViewController.viewControllers lastObject] topViewController];
    
    // load sol.xml, which defines our solar system and its astronomical objects.
    self.starSystem = [Galaxy_XMLLoader readFromFile:[[NSBundle mainBundle] pathForResource:@"sol" ofType:@".xml"]];
    
    // add the stars, planets, and moons to the collection of astronomical objects.
    self.starSystem.AstronomicalObjects = [NSMutableArray arrayWithArray:self.starSystem.Stars];
    [self.starSystem.AstronomicalObjects addObjectsFromArray:self.starSystem.Planets];
    for(Galaxy_Planet* planet in [self.starSystem Planets]) {
        [self.starSystem.AstronomicalObjects addObjectsFromArray:[planet Moons]];
    }
    
    Galaxy_Planet* defaultObject = [self.starSystem planetWithName:@"Earth"];
    if(defaultObject != nil)
        self.detailViewController.detailItem = defaultObject; // display Earth by default on load
    else
        self.detailViewController.detailItem = [[self.starSystem Planets] objectAtIndex:0];
        
    [self.tableView selectRowAtIndexPath:[NSIndexPath indexPathForRow:2 inSection:0] animated:NO scrollPosition:UITableViewScrollPositionNone];
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

#pragma mark - Table View

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView
{
    return 1;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    return [[starSystem Planets] count];
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:@"Cell" forIndexPath:indexPath];

    Galaxy_Planet *object = starSystem.Planets[indexPath.row];
    cell.textLabel.text = [object name];
    return cell;
}

- (BOOL)tableView:(UITableView *)tableView canEditRowAtIndexPath:(NSIndexPath *)indexPath
{
    // Return NO if you do not want the specified item to be editable.
    return NO;
}

- (void)tableView:(UITableView *)tableView commitEditingStyle:(UITableViewCellEditingStyle)editingStyle forRowAtIndexPath:(NSIndexPath *)indexPath
{
    if (editingStyle == UITableViewCellEditingStyleDelete) {
        [_objects removeObjectAtIndex:indexPath.row];
        [tableView deleteRowsAtIndexPaths:@[indexPath] withRowAnimation:UITableViewRowAnimationFade];
    } else if (editingStyle == UITableViewCellEditingStyleInsert) {
        // Create a new instance of the appropriate class, insert it into the array, and add a new row to the table view.
    }
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
    if ([[UIDevice currentDevice] userInterfaceIdiom] == UIUserInterfaceIdiomPad) {
        self.detailViewController.detailItem = starSystem.Planets[indexPath.row];
    }
}

- (void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender
{
    if ([[segue identifier] isEqualToString:@"showDetail"]) {
        NSIndexPath *indexPath = [self.tableView indexPathForSelectedRow];
        [[segue destinationViewController] setDetailItem:starSystem.Planets[indexPath.row]];
    }
}

@end
