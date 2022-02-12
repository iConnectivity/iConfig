/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import "ICFileBrowserViewController.h"
#import "DevicePID.h"

using namespace GeneSysLib;

@interface ICFileBrowserViewController ()

@end

@implementation ICFileBrowserViewController

+ (ICFileBrowserViewController *)fileBrowserWithDevice:(DeviceInfoPtr)dev {
  return [[ICFileBrowserViewController alloc] initWithDevice:dev];
}

- (id)initWithDevice:(DeviceInfoPtr)dev {
  self = [super initWithNibName:@"ICFileBrowserViewController" bundle:nil];

  if (self) {
    NSParameterAssert(dev);
    self.device = dev;
  }

  return self;
}

- (void)viewWillAppear:(BOOL)animated {
  [super viewWillAppear:animated];

  [self.navigationController setNavigationBarHidden:NO animated:YES];
  [self.navigationItem setTitle:@"Load Configuration"];

  auto *const bbi =
      [[UIBarButtonItem alloc] initWithTitle:@"Edit"
                                       style:UIBarButtonItemStylePlain
                                      target:self
                                      action:@selector(editButton)];

  [self.navigationItem setRightBarButtonItem:bbi];

  NSString *const documentsPath = [NSSearchPathForDirectoriesInDomains(
      NSDocumentDirectory, NSUserDomainMask, YES) objectAtIndex:0];

  NSError *error;
  NSArray *const listing =
      [[NSFileManager defaultManager] contentsOfDirectoryAtPath:documentsPath
                                                          error:&error];

  auto *const tempArray = [NSMutableArray array];

  NSString *supportedExtension = nil;

  if (self.device) {
    const auto &deviceID = self.device->getDeviceID();
    const auto &pid = deviceID.pid();

    switch (pid) {
      case DevicePID::iConnect2Plus:
        supportedExtension = @"\\.icm2$";
        break;

      case DevicePID::iConnect4Plus:
        supportedExtension = @"\\.icm4$";
        break;
    }
  }

  if (supportedExtension != nil) {
    for (NSString *const file in listing) {
      if ([file rangeOfString:supportedExtension
                      options:(NSCaseInsensitiveSearch |
                               NSRegularExpressionSearch)].location !=
          NSNotFound) {
        auto *const attributes = [[NSFileManager defaultManager]
            attributesOfItemAtPath:[documentsPath
                                       stringByAppendingFormat:@"/%@", file]
                             error:&error];

        if ((attributes) &&
            ([attributes objectForKey:NSFileModificationDate])) {
          [tempArray addObject:@{
                                 @"FileName" : file,
                                 NSFileModificationDate :
                                 attributes[NSFileModificationDate]
                               }];
        }
      }
    }
  }

  self.directoryListing = tempArray;

  /*
  if (error) {
    //NSLog(@"Error %@", [error localizedDescription]);
  }
   */
}

- (void)viewWillDisappear:(BOOL)animated {
  [super viewWillDisappear:animated];
  self.navigationItem.rightBarButtonItem = nil;
}

// UITableViewDataSource
- (NSInteger)tableView:(UITableView *)tableView
    numberOfRowsInSection:(NSInteger)section {
  return [self.directoryListing count];
}

- (UITableViewCell *)tableView:(UITableView *)tableView
         cellForRowAtIndexPath:(NSIndexPath *)indexPath {
  static NSString *const cellIdentifier = @"CellIdentifier";

  UITableViewCell *cell =
      [tableView dequeueReusableCellWithIdentifier:cellIdentifier];
  if (cell == nil) {
    cell = [[UITableViewCell alloc] initWithStyle:UITableViewCellStyleValue1
                                  reuseIdentifier:cellIdentifier];
  }

  [cell.textLabel setText:self.directoryListing[indexPath.row][@"FileName"]];

  NSDate *const date =
      self.directoryListing[indexPath.row][NSFileModificationDate];
  if (date) {
    auto *const dateString =
        [NSDateFormatter localizedStringFromDate:date
                                       dateStyle:NSDateFormatterLongStyle
                                       timeStyle:NSDateFormatterShortStyle];
    [cell.detailTextLabel setText:dateString];
  } else {
    [cell.detailTextLabel setText:@""];
  }

  return cell;
}

// UITableViewDelegate
- (NSString *)tableView:(UITableView *)tableView
    titleForHeaderInSection:(NSInteger)section {
  return (((self.directoryListing) && (self.directoryListing.count > 0))
              ? (@"")
              : (@"No files found."));
}

- (void)editButton {
  [self.myTableView setEditing:![self.myTableView isEditing] animated:YES];
  [self.navigationItem.rightBarButtonItem
      setTitle:((self.myTableView.editing) ? (@"Done") : (@"Edit"))];
}

- (BOOL)tableView:(UITableView *)tableView
    canEditRowAtIndexPath:(NSIndexPath *)indexPath {
  return ((indexPath.section == 0) &&
          (indexPath.row < [self.directoryListing count]));
}

- (UITableViewCellEditingStyle)tableView:(UITableView *)tableView
           editingStyleForRowAtIndexPath:(NSIndexPath *)indexPath {
  return UITableViewCellEditingStyleDelete;
}

- (void)tableView:(UITableView *)tableView
    commitEditingStyle:(UITableViewCellEditingStyle)editingStyle
     forRowAtIndexPath:(NSIndexPath *)indexPath {
  if ((editingStyle == UITableViewCellEditingStyleDelete) &&
      (indexPath.row < [self.directoryListing count])) {
    NSDictionary *const fileDirectory = self.directoryListing[indexPath.row];
    NSString *const rootPath = [NSSearchPathForDirectoriesInDomains(
        NSDocumentDirectory, NSUserDomainMask, YES) objectAtIndex:0];
    NSString *const fileName =
        [rootPath stringByAppendingPathComponent:fileDirectory[@"FileName"]];
    if ([[NSFileManager defaultManager] fileExistsAtPath:fileName]) {
      NSError *error;
      [[NSFileManager defaultManager] removeItemAtPath:fileName error:&error];

      if (error == nil) {
        [self.directoryListing removeObject:fileDirectory];
        [tableView deleteRowsAtIndexPaths:@[ indexPath ]
                         withRowAnimation:UITableViewRowAnimationRight];

        if ([self.directoryListing count] == 0) {
          [tableView reloadData];
        }
      }
    }
  }
}

- (void)tableView:(UITableView *)tableView
    didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
  if (self.device) {
    NSDictionary *const fileDirectory = self.directoryListing[indexPath.row];
    if (fileDirectory) {
      NSString *const rootPath = [NSSearchPathForDirectoriesInDomains(
          NSDocumentDirectory, NSUserDomainMask, YES) objectAtIndex:0];
      NSString *const fileName =
          [rootPath stringByAppendingPathComponent:fileDirectory[@"FileName"]];

      if (fileName) {
        NSData *const fileData = [NSData dataWithContentsOfFile:fileName];
        auto *const rawData = (Byte *)[fileData bytes];
        auto fileBytes = Bytes();

        fileBytes.insert(fileBytes.begin(), rawData,
                         rawData + [fileData length]);

        self.device->deserialize(fileBytes);

        [self.navigationController popToRootViewControllerAnimated:YES];
      }
    }
  }
}

- (BOOL)shouldAutorotateToInterfaceOrientation:
            (UIInterfaceOrientation)interfaceOrientation {
  const auto &isiPad = ([[UIDevice currentDevice] userInterfaceIdiom] ==
                        UIUserInterfaceIdiomPad);
  const auto &isPortait =
      UIInterfaceOrientationIsPortrait(interfaceOrientation);

  // should only rotate with an iPad or if the orientation is portrait
  return (isiPad || isPortait);
}

@end
