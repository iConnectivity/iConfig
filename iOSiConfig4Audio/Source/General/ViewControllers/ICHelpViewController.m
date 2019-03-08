/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import "ICHelpViewController.h"

@interface ICHelpViewController ()

@property(nonatomic, strong) NSURLRequest *request;

@end

@implementation ICHelpViewController

- (id)initWithNibName:(NSString *)nibNameOrNil
               bundle:(NSBundle *)nibBundleOrNil
              request:(NSURLRequest *)request {
  self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
  if (self) {
    // Custom initialization
    self.request = request;
  }
  return self;
}

- (void)viewDidLoad {
  [super viewDidLoad];
  // Do any additional setup after loading the view from its nib.
}

- (void)didReceiveMemoryWarning {
  [super didReceiveMemoryWarning];
  // Dispose of any resources that can be recreated.
}

- (void)viewDidUnload {
  [self setWebView:nil];
  [super viewDidUnload];
}

- (void)viewWillAppear:(BOOL)animated {
  [super viewWillAppear:animated];

  auto *const leftArrowImage = [UIImage imageNamed:@"LeftArrow"];
  auto *const rightArrowImage = [UIImage imageNamed:@"RightArrow"];

  auto *const leftArrowBBI =
      [[UIBarButtonItem alloc] initWithImage:leftArrowImage
                                       style:UIBarButtonItemStylePlain
                                      target:self.webView
                                      action:@selector(goBack)];
  auto *const rightArrowBBI =
      [[UIBarButtonItem alloc] initWithImage:rightArrowImage
                                       style:UIBarButtonItemStylePlain
                                      target:self.webView
                                      action:@selector(goForward)];

  [leftArrowBBI setImageInsets:UIEdgeInsetsMake(0, -2, 0, 0)];
  [rightArrowBBI setImageInsets:UIEdgeInsetsMake(0, +2, 0, 0)];

  [self.navigationItem setRightBarButtonItems:@[ rightArrowBBI, leftArrowBBI ]];

  [self.navigationController setNavigationBarHidden:NO animated:YES];

  [self.webView loadHTMLString:@"Please wait..." baseURL:nil];
}

- (void)viewDidAppear:(BOOL)animated {
  [super viewDidAppear:animated];
  [self.webView loadRequest:self.request];
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
