//
//  ViewController.m
//  RayTraceBenchmark_iOS
//
//  Created by Andrew Witte on 4/5/14.
//  Copyright (c) 2014 Reign-Studios. All rights reserved.
//

#import "ViewController.h"
#include "RayTraceBenchmark.h"

@implementation ViewController

-  (void)myButtonClick:(id)sender
{
    double sec = runTest();
    _secText.text = [NSNumber numberWithDouble:sec].stringValue;
}

- (void)viewDidLoad
{
    [super viewDidLoad];
	// Do any additional setup after loading the view, typically from a nib.
    [_runButton addTarget:self action:@selector(myButtonClick:) forControlEvents:(UIControlEvents)UIControlEventTouchDown];
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

@end
