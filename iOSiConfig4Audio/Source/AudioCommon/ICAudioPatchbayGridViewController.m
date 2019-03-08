/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import "ICAudioPatchbayGridViewController.h"
#import <QuartzCore/QuartzCore.h>
#import "NSString+RotatedDraw.h"

#define MoveToPoint(pt) \
  CGContextMoveToPoint(UIGraphicsGetCurrentContext(), pt.x, pt.y)

#define AddPoint(pt) \
  CGContextAddLineToPoint(UIGraphicsGetCurrentContext(), pt.x, pt.y)

#define kRefreshRate (30.0f / 1000.0f)

static const CGFloat gutterSize = 35.0f;
static const CGFloat arrowSize = 5.0f;
static const CGFloat textHeight = 20.0f;
static const CGFloat arrowWidth = 5.0f;
static const CGFloat arrowHeight = 9.0f;
static const CGFloat arrowXOffset = 5.0f;
static const CGFloat arrowYOffset = 2.0f;
static const CGFloat circleRadius = 6.0f;
static const CGFloat crossWidth = 6.0f;
static const CGFloat horizontalHeaderHeight = 50.0f;
static const CGFloat verticalHeaderWidth = 50.0f;

static const CGFloat sectionColorRed = 0.705888f;
static const CGFloat sectionColorGreen = 0.8627f;
static const CGFloat sectionColorBlue = 1.0f;
static const CGFloat sectionColorAlpha = 0.5f;

static const CGFloat additionColorRed = 0.0f;
static const CGFloat additionColorGreen = 0.0f;
static const CGFloat additionColorBlue = 1.0f;
static const CGFloat additionColorAlpha = 1.0f;

static const CGFloat numberFontSize = 14;
static const CGFloat fullAlpha = 1.0f;
static const CGFloat blendedAlpha = 0.75f;
static const CGFloat headerAlpha = 0.2f;
static const CGFloat gridBackAlpha = 0.07f;
static const CGFloat verticalHeaderAngle = 1.57079632679f;

typedef unsigned int (^PortCount)(unsigned int);

@interface ICAudioPatchbayGridViewController () {
  UIImage *staticImage;
  UIImage *dynamicImage;
  NSPredicate *filterTouchPredicate;
  NSTimer *redrawTimer;
  NSSet *currentProposed;

  CAShapeLayer *pathLayer;
  CAShapeLayer *bluePathLayer;
  CAShapeLayer *proposedLayer;
}

@property(nonatomic, strong) NSSet *touches;

- (CGFloat)horizontalHeaderHeight;
- (CGFloat)horizontalHeaderWidth:(CGSize)viewSize;
- (CGFloat)verticalHeaderWidth;
- (CGFloat)verticalHeaderHeight:(CGSize)viewSize;
- (CGFloat)gridWidth:(CGSize)viewSize;
- (CGFloat)gridHeight:(CGSize)viewSize;
- (CGFloat)gutterSize;
- (CGFloat)arrowSize;
- (CGFloat)horizontalChannelNumOffset:(CGSize)viewSize;
- (CGFloat)horizontalArrowOffset:(CGSize)viewSize;
- (CGFloat)verticalChannelNumOffset;
- (CGFloat)verticalArrowOffset;
- (CGRect)horizontalHeaderRect:(CGPoint)offset size:(CGSize)viewSize;
- (CGRect)verticalHeaderRect:(CGPoint)offset size:(CGSize)viewSize;
- (CGRect)horizontalGutterRect:(CGPoint)offset size:(CGSize)viewSize;
- (CGRect)verticalGutterRect:(CGPoint)offset size:(CGSize)viewSize;
- (CGRect)topRightRect:(CGPoint)offset size:(CGSize)viewSize;
- (CGRect)gridRect:(CGPoint)offset size:(CGSize)viewSize;

- (UIImage *)generateStaticImage;
- (void)updateDynamicPaths:(NSSet *)touches;

- (void)inContextDrawTriangle:(CGPoint)a b:(CGPoint)b c:(CGPoint)c;
- (void)inContextDrawDownTriangle:(CGPoint)point;
- (void)inContextDrawRightTriangle:(CGPoint)point;

- (void)inPathDrawTriangle:(CGMutablePathRef *)path
                         a:(CGPoint)a
                         b:(CGPoint)b
                         c:(CGPoint)c;
- (void)inPathDrawDownTriangle:(CGMutablePathRef *)path point:(CGPoint)point;
- (void)inPathDrawRightTriangle:(CGMutablePathRef *)path point:(CGPoint)point;

- (void)inContextDrawLine:(CGPoint)a to:(CGPoint)b;
- (void)inContextDrawHorizontalHeaderBackground:(CGSize)viewSize;
- (void)inContextDrawVerticalGutterBackground:(CGSize)viewSize;

- (void)redrawTimerTick:(id)sender;

@end

@implementation ICAudioPatchbayGridViewController

- (id)initWithDelegate:(id<ICPatchbayDelegate>)patchbayDelegate {
  self = [super initWithNibName:@"ICAudioPatchbayGrid_iPad" bundle:nil];

  if (self) {
    NSParameterAssert(patchbayDelegate);
    self.patchbayDelegate = patchbayDelegate;
  }

  return self;
}

- (void)viewDidLoad {
  [super viewDidLoad];
  // Do any additional setup after loading the view, typically from a nib.

  filterTouchPredicate = [NSPredicate
      predicateWithFormat:@"(phase = %d) OR (phase = %d) OR (phase = %d)",
                          UITouchPhaseBegan, UITouchPhaseMoved,
                          UITouchPhaseStationary];
  redrawTimer = nil;

  pathLayer = [[CAShapeLayer alloc] init];
  bluePathLayer = [[CAShapeLayer alloc] init];
  proposedLayer = [[CAShapeLayer alloc] init];

  [imageView.layer addSublayer:pathLayer];
  [imageView.layer addSublayer:bluePathLayer];
  [imageView.layer addSublayer:proposedLayer];

  [self.view setContentScaleFactor:[[UIScreen mainScreen] scale]];
  [imageView setContentScaleFactor:[[UIScreen mainScreen] scale]];

  self.view.multipleTouchEnabled = NO;
  self.view.exclusiveTouch = YES;
}

- (void)viewWillAppear:(BOOL)animated {
  [super viewWillAppear:animated];
  [self.navigationController setNavigationBarHidden:NO animated:YES];
}

- (void)inContextSetSectionColor {
  CGContextSetRGBStrokeColor(UIGraphicsGetCurrentContext(), sectionColorRed,
                             sectionColorGreen, sectionColorBlue,
                             sectionColorAlpha);
  CGContextSetRGBFillColor(UIGraphicsGetCurrentContext(), sectionColorRed,
                           sectionColorGreen, sectionColorBlue,
                           sectionColorAlpha);
}

- (void)inContextSetBlackColor:(CGFloat)alpha {
  CGContextSetRGBStrokeColor(UIGraphicsGetCurrentContext(), 0, 0, 0, alpha);
  CGContextSetRGBFillColor(UIGraphicsGetCurrentContext(), 0, 0, 0, alpha);
}

- (void)inContextSetAdditionColor {
  CGContextSetRGBStrokeColor(UIGraphicsGetCurrentContext(), additionColorRed,
                             additionColorGreen, additionColorBlue,
                             additionColorAlpha);
  CGContextSetRGBFillColor(UIGraphicsGetCurrentContext(), additionColorRed,
                           additionColorGreen, additionColorBlue,
                           additionColorAlpha);
}

- (void)inContextSetSolidLine {
  CGContextSetLineDash(UIGraphicsGetCurrentContext(), 0, NULL, 0);
}

- (void)inContextSetDottedLine {
  CGFloat pattern[] = {1, 5};
  CGContextSetLineDash(UIGraphicsGetCurrentContext(), 0, pattern,
                       sizeof(pattern) / sizeof(CGFloat));
}

- (void)inContextDrawHorizontalHeaderBackground:(CGSize)viewSize {
  CGContextSaveGState(UIGraphicsGetCurrentContext());
  [self inContextSetBlackColor:headerAlpha];
  CGContextFillRect(
      UIGraphicsGetCurrentContext(),
      [self horizontalHeaderRect:CGPointMake(0, 0) size:viewSize]);

  CGContextRestoreGState(UIGraphicsGetCurrentContext());
}

- (void)inContextDrawVerticalGutterBackground:(CGSize)viewSize {
  [self inContextSetBlackColor:blendedAlpha];

  CGContextSaveGState(UIGraphicsGetCurrentContext());
  [self inContextSetBlackColor:headerAlpha];

  CGPoint topRightGrid =
      CGPointMake([self gridWidth:viewSize],
                  [self horizontalHeaderHeight] + [self gutterSize]);
  CGPoint bottomRightGrid =
      CGPointMake([self gridWidth:viewSize], viewSize.height);
  CGPoint topRightView = CGPointMake(viewSize.width, 0);

  CGPoint bottomLeftVerticalGutter = CGPointMake(
      [self gridWidth:viewSize] + [self gutterSize], viewSize.height);
  CGPoint topLeftVerticalGutter =
      CGPointMake([self gridWidth:viewSize] + [self gutterSize],
                  [self horizontalHeaderHeight] + [self gutterSize]);
  CGPoint topRightVerticalGutter = CGPointMake(
      viewSize.width, [self horizontalHeaderHeight] + [self gutterSize]);

  MoveToPoint(topRightGrid);
  AddPoint(topRightView);
  AddPoint(topRightVerticalGutter);
  AddPoint(topLeftVerticalGutter);
  AddPoint(bottomLeftVerticalGutter);
  AddPoint(bottomRightGrid);
  CGContextClosePath(UIGraphicsGetCurrentContext());

  CGContextDrawPath(UIGraphicsGetCurrentContext(), kCGPathFillStroke);

  CGContextRestoreGState(UIGraphicsGetCurrentContext());
}

- (void)inContextDrawVericalSections:(CGSize)viewSize {
  [self inContextSetSectionColor];

  CGFloat totalSources = (CGFloat)[self.patchbayDelegate totalSources];
  CGFloat pinDX = [self gridWidth:viewSize] / totalSources;
  CGFloat xOffset = 0.0f;
  CGFloat yOffset = 0;
  CGFloat height = viewSize.height;

  bool drawFlipFlop = false;
  for (int section = 1; section <= [self.patchbayDelegate numSourceSections];
       ++section) {
    CGFloat numInputs =
        (CGFloat)[self.patchbayDelegate numSourcesPerSection : section];
    if (numInputs) {
      CGFloat width = pinDX * numInputs;

      if (drawFlipFlop) {
        CGContextFillRect(UIGraphicsGetCurrentContext(),
                          CGRectMake(xOffset, yOffset, width, height));
      }

      drawFlipFlop = !drawFlipFlop;
      xOffset += width;
    }
  }
}

- (void)inContextDrawHorizontalSections:(CGSize)viewSize {
  [self inContextSetSectionColor];

  CGFloat totalDestinations =
      (CGFloat)[self.patchbayDelegate totalDestinations];
  CGFloat pinDY = [self gridHeight:viewSize] / totalDestinations;
  CGFloat xOffset = 0;
  CGFloat yOffset = [self horizontalHeaderHeight] + [self gutterSize];
  CGFloat width = viewSize.width;

  bool drawFlipFlop = false;
  for (int section = 1;
       section <= [self.patchbayDelegate numDestinationSections]; ++section) {
    CGFloat numOutputs =
        (CGFloat)[self.patchbayDelegate numDestinationsPerSection : section];
    if (numOutputs) {
      CGFloat height = pinDY * numOutputs;

      if (drawFlipFlop) {
        CGContextFillRect(UIGraphicsGetCurrentContext(),
                          CGRectMake(xOffset, yOffset, width, height));
      }

      drawFlipFlop = !drawFlipFlop;
      yOffset += height;
    }
  }
}

- (void)inContextDrawBorder:(CGSize)viewSize {
  [self inContextSetBlackColor:fullAlpha];
  [self inContextSetSolidLine];
  CGContextSetLineWidth(UIGraphicsGetCurrentContext(), 1.0f);

  /* Draw the lines that seperate the sections */

  CGPoint topLeftGrid =
      CGPointMake(0, [self horizontalHeaderHeight] + [self gutterSize]);
  CGPoint topRightGrid =
      CGPointMake([self gridWidth:viewSize],
                  [self horizontalHeaderHeight] + [self gutterSize]);
  CGPoint bottomRightGrid =
      CGPointMake([self gridWidth:viewSize], viewSize.height);
  CGPoint topRightView = CGPointMake(viewSize.width, 0);

  CGPoint bottomLeftHorizontalGutter =
      CGPointMake(0, [self horizontalHeaderHeight]);
  CGPoint bottomRightHorizontalGutter =
      CGPointMake([self gridWidth:viewSize], [self horizontalHeaderHeight]);
  CGPoint topRightHorizontalGutter = CGPointMake([self gridWidth:viewSize], 0);

  CGPoint bottomLeftVerticalGutter = CGPointMake(
      [self gridWidth:viewSize] + [self gutterSize], viewSize.height);
  CGPoint topLeftVerticalGutter =
      CGPointMake([self gridWidth:viewSize] + [self gutterSize],
                  [self horizontalHeaderHeight] + [self gutterSize]);
  CGPoint topRightVerticalGutter = CGPointMake(
      viewSize.width, [self horizontalHeaderHeight] + [self gutterSize]);

  [self inContextDrawLine:topLeftGrid to:topRightGrid];
  [self inContextDrawLine:topRightGrid to:bottomRightGrid];
  [self inContextDrawLine:topRightGrid to:topRightView];
  [self inContextDrawLine:bottomLeftHorizontalGutter
                       to:bottomRightHorizontalGutter];
  [self inContextDrawLine:bottomRightHorizontalGutter
                       to:topRightHorizontalGutter];
  [self inContextDrawLine:bottomLeftVerticalGutter to:topLeftVerticalGutter];
  [self inContextDrawLine:topLeftVerticalGutter to:topRightVerticalGutter];
  CGContextStrokePath(UIGraphicsGetCurrentContext());
}

- (void)inContextDrawDottedGrid:(CGSize)viewSize {
  CGFloat totalSources = (CGFloat)[self.patchbayDelegate totalSources];
  CGFloat pinDX = [self gridWidth:viewSize] / totalSources;
  CGFloat totalDestinations =
      (CGFloat)[self.patchbayDelegate totalDestinations];
  CGFloat pinDY = [self gridHeight:viewSize] / totalDestinations;

  CGFloat xOffset, yOffset;

  xOffset = pinDX * 0.5f;
  yOffset = [self horizontalHeaderHeight] + [self gutterSize] - arrowYOffset;

  CGContextSetLineWidth(UIGraphicsGetCurrentContext(), 0.5);
  CGContextSetLineCap(UIGraphicsGetCurrentContext(), kCGLineCapRound);
  [self inContextSetDottedLine];
  [self inContextSetBlackColor:blendedAlpha];

  for (int i = 0; i < (int)totalSources; ++i) {
    [self inContextDrawLine:CGPointMake(xOffset, yOffset)
                         to:CGPointMake(xOffset, viewSize.height)];
    xOffset += pinDX;
  }

  xOffset = 0.0f;
  yOffset = [self horizontalHeaderHeight] + [self gutterSize] + pinDY * 0.5f;
  for (int i = 0; i < (int)totalDestinations; ++i) {
    [self
        inContextDrawLine:CGPointMake(xOffset, yOffset)
                       to:CGPointMake([self gridWidth:viewSize] + arrowXOffset,
                                      yOffset)];
    yOffset += pinDY;
  }

  CGContextStrokePath(UIGraphicsGetCurrentContext());

  [self inContextSetBlackColor:gridBackAlpha];
  CGContextFillRect(UIGraphicsGetCurrentContext(),
                    [self gridRect:CGPointMake(0, 0) size:viewSize]);
}

- (void)inContextDrawHorizontalPortNumbers:(CGSize)viewSize {
  [self inContextSetBlackColor:fullAlpha];

  int numSections = [self.patchbayDelegate numSourceSections];
  CGFloat totalSources = (CGFloat)[self.patchbayDelegate totalSources];
  CGFloat pinDX = [self gridWidth:viewSize] / totalSources;

  CGFloat xOffset, yOffset;

  xOffset = 0.0f;
  yOffset = [self horizontalHeaderHeight];

  for (int i = 1; i <= numSections; ++i) {
    int pinsPerSection = [self.patchbayDelegate numSourcesPerSection:i];
    for (int pin = 1; pin <= pinsPerSection; ++pin) {
      NSString *text = [NSString stringWithFormat:@"%d", pin];
      if (![self.patchbayDelegate isCollapsedInput:i]) {
        [text drawInRect:CGRectMake(xOffset, yOffset, pinDX, [self gutterSize])
                withFont:[UIFont systemFontOfSize:numberFontSize]
           lineBreakMode:NSLineBreakByCharWrapping
               alignment:NSTextAlignmentCenter];
      }

      xOffset += pinDX;
    }
  }
}

- (void)inContextDrawHorizontalPortNames:(CGSize)viewSize {
  [self inContextSetBlackColor:fullAlpha];

  int numSections = [self.patchbayDelegate numSourceSections];
  CGFloat totalSources = (CGFloat)[self.patchbayDelegate totalSources];
  CGFloat pinDX = [self gridWidth:viewSize] / totalSources;

  CGFloat xOffset, yOffset;

  xOffset = 0.0f;
  yOffset = [self horizontalHeaderHeight] * 0.5f;

  for (int i = 1; i <= numSections; ++i) {
    int pinsPerSection = [self.patchbayDelegate numSourcesPerSection:i];
    NSString *text = [self.patchbayDelegate nameForSourceSection:i];
    if (pinsPerSection)
    {
      [text drawInRect:CGRectMake(xOffset, yOffset, pinsPerSection * pinDX,
                                  [self horizontalHeaderHeight])
              withFont:[UIFont systemFontOfSize:19]
                 angle:0
         lineBreakMode:NSLineBreakByWordWrapping];
      xOffset += pinsPerSection * pinDX;
    }
  }
}

- (void)inContextDrawVerticalPortNames:(CGSize)viewSize {
  [self inContextSetBlackColor:fullAlpha];

  int numSections = [self.patchbayDelegate numSourceSections];
  CGFloat totalDestinations =
      (CGFloat)[self.patchbayDelegate totalDestinations];
  CGFloat pinDY = [self gridHeight:viewSize] / totalDestinations;

  CGFloat xOffset, yOffset;

  xOffset = [self gridWidth:viewSize] + [self gutterSize] +
            [self verticalHeaderWidth] * 0.5f;
  yOffset = [self horizontalHeaderHeight] + [self gutterSize];

  for (int i = 1; i <= numSections; ++i) {
    int pinsPerSection = [self.patchbayDelegate numDestinationsPerSection:i];
    NSString *text = [self.patchbayDelegate nameForDestinationSection:i];
    if (pinsPerSection) {
      [text drawInRect:CGRectMake(xOffset, yOffset, pinDY * pinsPerSection,
                                  [self verticalHeaderWidth])
              withFont:[UIFont systemFontOfSize:19]
                 angle:verticalHeaderAngle
         lineBreakMode:NSLineBreakByWordWrapping];
      yOffset += pinsPerSection * pinDY;
    }
  }
}

- (void)inContextDrawVerticalPortNumbers:(CGSize)viewSize {
  [self inContextSetBlackColor:fullAlpha];

  int numSections = [self.patchbayDelegate numDestinationSections];
  CGFloat totalDestinations =
      (CGFloat)[self.patchbayDelegate totalDestinations];
  CGFloat pinDY = [self gridHeight:viewSize] / totalDestinations;

  CGFloat xOffset, yOffset;

  xOffset = [self gridWidth:viewSize] + arrowXOffset + arrowHeight;
  yOffset = [self horizontalHeaderHeight] + [self gutterSize] + pinDY * 0.5f;

  for (int i = 1; i <= numSections; ++i) {
    int pinsPerSection = [self.patchbayDelegate numDestinationsPerSection:i];
    for (int pin = 1; pin <= pinsPerSection; ++pin) {
      NSString *text = [NSString stringWithFormat:@"%d", pin];

      if (![self.patchbayDelegate isCollapsedOutput:i]) {
        [text drawInRect:CGRectMake(
                                    xOffset, yOffset,
                                    [self gutterSize] - arrowHeight - arrowXOffset,
                                    pinDY)
                withFont:[UIFont systemFontOfSize:numberFontSize]
                   angle:0
           lineBreakMode:NSLineBreakByClipping];
      }

      yOffset += pinDY;
    }
  }
}

- (void)inContextDrawOutArrows:(CGSize)viewSize {
  [self inContextSetBlackColor:fullAlpha];

  CGFloat totalSources = (CGFloat)[self.patchbayDelegate totalSources];
  CGFloat pinDX = [self gridWidth:viewSize] / totalSources;

  CGFloat xOffset, yOffset;

  xOffset = pinDX * 0.5f;
  yOffset = [self horizontalHeaderHeight] + [self gutterSize] - arrowYOffset;

  for (int i = 1; i <= (int)totalSources; ++i) {
    [self inContextDrawDownTriangle:CGPointMake(xOffset, yOffset)];
    xOffset += pinDX;
  }
}

- (void)inContextDrawInArrows:(CGSize)viewSize {
  CGFloat totalDestinations =
      (CGFloat)[self.patchbayDelegate totalDestinations];
  CGFloat pinDY = [self gridHeight:viewSize] / totalDestinations;

  CGFloat xOffset, yOffset;

  xOffset = [self gridWidth:viewSize] + arrowXOffset;
  yOffset = [self horizontalHeaderHeight] + [self gutterSize] + pinDY * 0.5f;
  for (int i = 1; i <= (int)totalDestinations; ++i) {
    [self inContextDrawRightTriangle:CGPointMake(xOffset, yOffset)];
    yOffset += pinDY;
  }
}

- (void)inContextDrawLegend:(CGSize)viewSize {
  CGFloat xOffset, yOffset;

  CGFloat angle;

  CGPoint topRightGrid =
      CGPointMake([self gridWidth:viewSize],
                  [self horizontalHeaderHeight] + [self gutterSize]);

  CGPoint topRightView = CGPointMake(viewSize.width, 0);

  angle =
      atan2f(topRightView.y - topRightGrid.y, topRightView.x - topRightGrid.x);

  CGPoint p2 = topRightView;
  CGPoint p1 = topRightGrid;
  // Assign the coord of p2 and p1...
  // End Assign...
  CGFloat xDist = (p2.x - p1.x);                               //[2]
  CGFloat yDist = (p2.y - p1.y);                               //[3]
  CGFloat distance = sqrt((xDist * xDist) + (yDist * yDist));  //[4]

  NSString *outText = @"Sources";
  xOffset = topRightGrid.x + abs(topRightView.x - topRightGrid.x) * 0.4f;
  yOffset = abs(topRightView.y - topRightGrid.y) * 0.4f;

  [outText drawAtPoint:CGPointMake(xOffset, yOffset)
              forWidth:distance
              withFont:[UIFont boldSystemFontOfSize:14]
                 angle:angle
         lineBreakMode:NSLineBreakByCharWrapping];

  NSString *inText = @"Destinations";
  xOffset = topRightGrid.x + abs(topRightView.x - topRightGrid.x) * 0.6f;
  yOffset = abs(topRightView.y - topRightGrid.y) * 0.6f;
  [inText drawAtPoint:CGPointMake(xOffset, yOffset)
             forWidth:distance
             withFont:[UIFont boldSystemFontOfSize:14]
                angle:angle
        lineBreakMode:NSLineBreakByCharWrapping];
}

- (void)inContextDrawBackgroundWithViewSize:(CGSize)viewSize
                           rightGutterWidth:(CGFloat)rightGutterWidth
                            topGutterHeight:(CGFloat)topGutterHeight
                          numSourceSections:(unsigned int)numSourceSections
                     numDestinationSections:
                         (unsigned int)numDestinationSections {
  assert(self.patchbayDelegate);

  [self inContextSetSectionColor];

  [self inContextDrawHorizontalHeaderBackground:viewSize];

  [self inContextDrawVerticalGutterBackground:viewSize];

  [self inContextDrawVericalSections:viewSize];

  [self inContextDrawHorizontalSections:viewSize];

  [self inContextDrawDottedGrid:viewSize];

  [self inContextDrawHorizontalPortNumbers:viewSize];

  [self inContextDrawHorizontalPortNames:viewSize];

  [self inContextDrawVerticalPortNames:viewSize];

  [self inContextDrawVerticalPortNumbers:viewSize];

  [self inContextDrawBorder:viewSize];

  [self inContextDrawOutArrows:viewSize];

  [self inContextDrawInArrows:viewSize];

  [self inContextDrawLegend:viewSize];
}

- (void)inContextDrawPortsWithViewSize:(CGSize)viewSize
                      rightGutterWidth:(CGFloat)rightGutterWidth
                       topGutterHeight:(CGFloat)topGutterHeight
                     numSourceSections:(unsigned int)numSourceSections
                     portsPerInSection:(PortCount)portsPerInSection
                numDestinationSections:(unsigned int)numDestinationSections
                    portsPerOutSection:(PortCount)portsPerOutSection {
  CGFloat width = viewSize.width;
  CGFloat gridWidth = width - rightGutterWidth;
  CGFloat yOffset = topGutterHeight;

  CGContextSetLineCap(UIGraphicsGetCurrentContext(), kCGLineCapRound);
  CGContextSetLineWidth(UIGraphicsGetCurrentContext(), 1.0);
  CGContextSetRGBStrokeColor(UIGraphicsGetCurrentContext(), 0, 0, 0, 0.8);

  CGContextSetBlendMode(UIGraphicsGetCurrentContext(), kCGBlendModeNormal);
  for (unsigned int i = 1; i <= numSourceSections; ++i) {
    unsigned int inPortCount = portsPerInSection(i);
    for (unsigned int j = 1; j <= inPortCount; ++j) {
      CGContextMoveToPoint(UIGraphicsGetCurrentContext(), 0, yOffset);
      CGContextAddLineToPoint(UIGraphicsGetCurrentContext(), gridWidth,
                              yOffset);
    }
  }

  CGContextStrokePath(UIGraphicsGetCurrentContext());
}

- (void)inContextDrawPatches {
  CGMutablePathRef patchPath = CGPathCreateMutable();
  CGMutablePathRef bluePatchPath = CGPathCreateMutable();

  CGFloat totalSources = (CGFloat)[self.patchbayDelegate totalSources];
  CGFloat pinDX = [self gridWidth:imageView.frame.size] / totalSources;
  CGFloat totalDestinations =
      (CGFloat)[self.patchbayDelegate totalDestinations];
  CGFloat pinDY = [self gridHeight:imageView.frame.size] / totalDestinations;

  if (self.patchbayDelegate) {
    [self.patchbayDelegate foreach:^(device_pair_t *pair) {
        if ((pair) &&
            ([self.patchbayDelegate isSourceDeviceInBounds:pair->source]) &&
            ([self.patchbayDelegate
                isDestinationDeviceInBounds:pair->destination])) {
          CGPoint a, b, c;

          int totalSrcs =
              [self.patchbayDelegate sourceIndexToTotal:pair->source];
          int totalDests =
              [self.patchbayDelegate destinationIndexToTotal:pair->destination];

          CGFloat horX = (CGFloat)(totalSrcs) * pinDX + pinDX * 0.5f;
          CGFloat horY = (CGFloat)(totalDests) * pinDY + pinDY * 0.5f;

          CGRect grid =
              [self gridRect:imageView.frame.origin size:imageView.frame.size];

          a = CGPointMake(grid.origin.x + horX, [self verticalArrowOffset]);
          b = CGPointMake(grid.origin.x + horX, grid.origin.y + horY);
          c = CGPointMake([self horizontalArrowOffset:imageView.frame.size],
                          grid.origin.y + horY);

          if ([self.patchbayDelegate isCollapsedInput:pair->source.port] ||
              [self.patchbayDelegate isCollapsedOutput:pair->destination.port]) {
            CGPathMoveToPoint(bluePatchPath, NULL, a.x, a.y);
            CGPathAddLineToPoint(bluePatchPath, NULL, b.x, b.y);
            CGPathAddLineToPoint(bluePatchPath, NULL, c.x, c.y);
            CGPathAddLineToPoint(bluePatchPath, NULL, b.x, b.y);
            CGPathAddLineToPoint(bluePatchPath, NULL, a.x, a.y);
            CGPathAddEllipseInRect(
                                   bluePatchPath, NULL,
                                   CGRectMake(b.x - circleRadius, b.y - circleRadius,
                                              circleRadius * 2.0f, circleRadius * 2.0f));
          }
          else {
            CGPathMoveToPoint(patchPath, NULL, a.x, a.y);
            CGPathAddLineToPoint(patchPath, NULL, b.x, b.y);
            CGPathAddLineToPoint(patchPath, NULL, c.x, c.y);
            CGPathAddLineToPoint(patchPath, NULL, b.x, b.y);
            CGPathAddLineToPoint(patchPath, NULL, a.x, a.y);
            CGPathAddEllipseInRect(
                                   patchPath, NULL,
                                   CGRectMake(b.x - circleRadius, b.y - circleRadius,
                                              circleRadius * 2.0f, circleRadius * 2.0f));
          }
        }
    }];

    [self.patchbayDelegate foreachMixer:^(device_pair_t *pair) {
      if ((pair) &&
          ([self.patchbayDelegate isSourceDeviceInBounds:pair->source]) &&
          ([self.patchbayDelegate
            isDestinationDeviceInBounds:pair->destination])) {
        CGPoint a, b, c;

        int totalSrcs =
        [self.patchbayDelegate sourceIndexToTotal:pair->source];
        int totalDests =
        [self.patchbayDelegate destinationIndexToTotal:pair->destination];

        CGFloat horX = (CGFloat)(totalSrcs) * pinDX + pinDX * 0.5f;
        CGFloat horY = (CGFloat)(totalDests) * pinDY + pinDY * 0.5f;

        CGRect grid =
        [self gridRect:imageView.frame.origin size:imageView.frame.size];

        a = CGPointMake(grid.origin.x + horX, [self verticalArrowOffset]);
        b = CGPointMake(grid.origin.x + horX, grid.origin.y + horY);
        c = CGPointMake([self horizontalArrowOffset:imageView.frame.size],
                        grid.origin.y + horY);

        if ([self.patchbayDelegate isCollapsedInput:pair->source.port] ||
            [self.patchbayDelegate isCollapsedOutput:pair->destination.port]) {
          CGPathMoveToPoint(bluePatchPath, NULL, a.x, a.y);
          CGPathAddLineToPoint(bluePatchPath, NULL, b.x, b.y);
          CGPathAddLineToPoint(bluePatchPath, NULL, c.x, c.y);
          CGPathAddLineToPoint(bluePatchPath, NULL, b.x, b.y);
          CGPathAddLineToPoint(bluePatchPath, NULL, a.x, a.y);
          CGPathAddEllipseInRect(
                                 bluePatchPath, NULL,
                                 CGRectMake(b.x - circleRadius, b.y - circleRadius,
                                            circleRadius * 2.0f, circleRadius * 2.0f));
        }
        else {
          CGPathMoveToPoint(patchPath, NULL, a.x, a.y);
          CGPathAddLineToPoint(patchPath, NULL, b.x, b.y);
          CGPathAddLineToPoint(patchPath, NULL, c.x, c.y);
          CGPathAddLineToPoint(patchPath, NULL, b.x, b.y);
          CGPathAddLineToPoint(patchPath, NULL, a.x, a.y);
          CGPathAddEllipseInRect(
                                 patchPath, NULL,
                                 CGRectMake(b.x - circleRadius, b.y - circleRadius,
                                            circleRadius * 2.0f, circleRadius * 2.0f));
        }
      }
    }];

  }

  pathLayer.allowsEdgeAntialiasing = NO;
  pathLayer.lineWidth = 1.2f;
  pathLayer.fillColor = [[UIColor blackColor] CGColor];
  pathLayer.strokeColor = [[UIColor blackColor] CGColor];
  pathLayer.path = patchPath;

  bluePathLayer.allowsEdgeAntialiasing = NO;
  bluePathLayer.lineWidth = 1.2f;
  bluePathLayer.fillColor = [[UIColor blueColor] CGColor];
  bluePathLayer.strokeColor = [[UIColor blueColor] CGColor];
  bluePathLayer.path = bluePatchPath;

  CGPathRelease(bluePatchPath);
  CGPathRelease(patchPath);
}

- (void)inPathDrawProposedPatches:(NSSet *)touches ofSize:(CGSize)viewSize {
  CGMutablePathRef path = CGPathCreateMutable();
  BOOL isAdd = FALSE;

  CGFloat totalSources = (CGFloat)[self.patchbayDelegate totalSources];
  CGFloat pinDX = [self gridWidth:imageView.frame.size] / totalSources;
  CGFloat totalDestinations =
      (CGFloat)[self.patchbayDelegate totalDestinations];
  CGFloat pinDY = [self gridHeight:imageView.frame.size] / totalDestinations;

  // loop through all touchs
  for (UITouch *touch in touches) {
    CGPoint location = [touch locationInView:imageView];
    CGRect grid = [self gridRect:CGPointMake(0, 0) size:viewSize];

    // filter out all non begin and move phases
    if (CGRectContainsPoint(grid, location)) {
      // modify the location to inside the grid
      location.x -= grid.origin.x;
      location.y -= grid.origin.y;

      // determine the corresponding row and column
      int col = (int)floor(location.x / pinDX) + 1;
      int row = (int)floor(location.y / pinDY) + 1;

      auto srcIndex = [self.patchbayDelegate sourceTotalToIndex:col];
      auto dstIndex = [self.patchbayDelegate destinationTotalToIndex:row];

      // check the bounds
      if (([self.patchbayDelegate isSourceDeviceInBounds:srcIndex]) &&
          ([self.patchbayDelegate isDestinationDeviceInBounds:dstIndex])) {
        isAdd = ![self.patchbayDelegate isPatchedFrom:srcIndex to:dstIndex];
        CGFloat horX = (CGFloat)(col - 1) * pinDX + pinDX * 0.5f;
        CGFloat horY = (CGFloat)(row - 1) * pinDY + pinDY * 0.5f;

        CGPoint a = CGPointMake(grid.origin.x + horX,
                                [self verticalArrowOffset] + arrowHeight);
        CGPoint b = CGPointMake(grid.origin.x + horX, grid.origin.y + horY);
        CGPoint c = CGPointMake([self gridWidth:viewSize] + arrowXOffset,
                                grid.origin.y + horY);

        CGPathMoveToPoint(path, NULL, a.x, a.y);
        CGPathAddLineToPoint(path, NULL, b.x, b.y);
        CGPathAddLineToPoint(path, NULL, c.x, c.y);
        CGPathAddLineToPoint(path, NULL, b.x, b.y);
        CGPathAddLineToPoint(path, NULL, a.x, a.y);
        [self inPathDrawDownTriangle:&path point:a];
        [self inPathDrawRightTriangle:&path point:c];

        if (isAdd) {
          CGPathAddEllipseInRect(
              path, NULL,
              CGRectMake(b.x - circleRadius * 2, b.y - circleRadius * 2,
                         circleRadius * 4.0f, circleRadius * 4.0f));
        } else {
          CGPoint ab, bc;
          ab.x = (a.x + b.x) * 0.5f;
          ab.y = (a.y + b.y) * 0.5f;
          bc.x = (b.x + c.x) * 0.5f;
          bc.y = (b.y + c.y) * 0.5f;

          CGPathMoveToPoint(path, NULL, ab.x - crossWidth, ab.y - crossWidth);
          CGPathAddLineToPoint(path, NULL, ab.x + crossWidth,
                               ab.y + crossWidth);
          CGPathMoveToPoint(path, NULL, ab.x + crossWidth, ab.y - crossWidth);
          CGPathAddLineToPoint(path, NULL, ab.x - crossWidth,
                               ab.y + crossWidth);

          CGPathMoveToPoint(path, NULL, b.x - crossWidth, b.y - crossWidth);
          CGPathAddLineToPoint(path, NULL, b.x + crossWidth, b.y + crossWidth);
          CGPathMoveToPoint(path, NULL, b.x + crossWidth, b.y - crossWidth);
          CGPathAddLineToPoint(path, NULL, b.x - crossWidth, b.y + crossWidth);

          CGPathMoveToPoint(path, NULL, bc.x - crossWidth, bc.y - crossWidth);
          CGPathAddLineToPoint(path, NULL, bc.x + crossWidth,
                               bc.y + crossWidth);
          CGPathMoveToPoint(path, NULL, bc.x + crossWidth, bc.y - crossWidth);
          CGPathAddLineToPoint(path, NULL, bc.x - crossWidth,
                               bc.y + crossWidth);
        }
      }
    }


  }
  proposedLayer.lineWidth = 3.0f;
  proposedLayer.fillColor =
      (isAdd ? [[UIColor blueColor] CGColor] : [[UIColor redColor] CGColor]);
  proposedLayer.strokeColor =
      (isAdd ? [[UIColor blueColor] CGColor] : [[UIColor redColor] CGColor]);
  proposedLayer.path = path;
  proposedLayer.lineJoin = kCALineJoinMiter;
  proposedLayer.miterLimit = 1000.0f;
  proposedLayer.lineCap = kCALineCapSquare;
  CGPathRelease(path);
}

- (void)viewDidAppear:(BOOL)animated {
  [super viewDidAppear:animated];
  imageView.layer.contents = (id)[[self generateStaticImage] CGImage];
  [self updateDynamicPaths:[NSSet set]];
  [self becomeFirstResponder];

  if (!redrawTimer) {
      //zx, 2017-06-16
/*
      redrawTimer = [NSTimer timerWithTimeInterval:kRefreshRate
                                          target:self
                                        selector:@selector(redrawTimerTick:)
                                        userInfo:nil
                                         repeats:YES];

    [[NSRunLoop currentRunLoop] addTimer:redrawTimer
                                 forMode:NSDefaultRunLoopMode];
*/
      redrawTimer = [NSTimer scheduledTimerWithTimeInterval:kRefreshRate
                                            target:self
                                          selector:@selector(redrawTimerTick:)
                                          userInfo:nil
                                           repeats:YES];
      
  }
}

- (void)viewDidDisappear:(BOOL)animated {
  [super viewDidDisappear:animated];
  if (redrawTimer) {
    [redrawTimer invalidate];
    redrawTimer = nil;
  }
}

/// AGH FIXME TODO
- (void)viewWillTransitionToSize:(CGSize)size withTransitionCoordinator:(id<UIViewControllerTransitionCoordinator>)coordinator
{
  //NSLog(@"viewWillTransitionToSize");
  [super viewWillTransitionToSize:size withTransitionCoordinator:coordinator];
  imageView.layer.contents = (id)[[self generateStaticImage] CGImage];
  [self updateDynamicPaths:[NSSet set]];
}

- (UIImage *)generateStaticImage {
  CGFloat scale = [[UIScreen mainScreen] scale];

  UIGraphicsBeginImageContextWithOptions(imageView.frame.size, NO, scale);
  CGContextClearRect(UIGraphicsGetCurrentContext(), imageView.frame);

  CGContextSetLineCap(UIGraphicsGetCurrentContext(), kCGLineCapRound);

  CGContextSetLineWidth(UIGraphicsGetCurrentContext(), 1.0f);

  CGSize viewSize = imageView.frame.size;
  CGFloat topGutterHeight = 100.0f;
  CGFloat rightGutterWidth = 100.0f;
  unsigned int numSourceSections = [self.patchbayDelegate numSourceSections];
  unsigned int numDestinationSections =
      [self.patchbayDelegate numDestinationSections];

  [self inContextDrawBackgroundWithViewSize:viewSize
                           rightGutterWidth:rightGutterWidth
                            topGutterHeight:topGutterHeight
                          numSourceSections:numSourceSections
                     numDestinationSections:numDestinationSections];

  staticImage = UIGraphicsGetImageFromCurrentImageContext();

  UIGraphicsEndImageContext();

  return staticImage;
}

- (void)updateDynamicPaths:(NSSet *)touches {
  [self inContextDrawPatches];

  [self inPathDrawProposedPatches:touches ofSize:imageView.frame.size];
}

- (CGFloat)horizontalHeaderHeight {
  return horizontalHeaderHeight;
}

- (CGFloat)horizontalHeaderWidth:(CGSize)viewSize {
  return viewSize.width - [self verticalHeaderWidth] - [self gutterSize];
}

- (CGFloat)verticalHeaderWidth {
  return verticalHeaderWidth;
}

- (CGFloat)verticalHeaderHeight:(CGSize)viewSize {
  return viewSize.height - [self horizontalHeaderHeight] - [self gutterSize];
}

- (CGFloat)gridWidth:(CGSize)viewSize {
  return [self horizontalHeaderWidth:viewSize];
}

- (CGFloat)gridHeight:(CGSize)viewSize {
  return [self verticalHeaderHeight:viewSize];
}

- (CGFloat)gutterSize {
  return gutterSize;
}

- (CGFloat)arrowSize {
  return arrowSize;
}

- (CGFloat)horizontalChannelNumOffset:(CGSize)viewSize {
  return [self horizontalArrowOffset:viewSize];
}

- (CGFloat)horizontalArrowOffset:(CGSize)viewSize {
  return [self gridWidth:viewSize] + [self arrowSize] + arrowXOffset;
}

- (CGFloat)verticalChannelNumOffset {
  return [self verticalArrowOffset] - [self arrowSize] - textHeight;
}

- (CGFloat)verticalArrowOffset {
  return [self horizontalHeaderHeight] + [self gutterSize] - arrowYOffset -
         arrowHeight;
}

- (CGRect)horizontalHeaderRect:(CGPoint)offset size:(CGSize)viewSize {
  return CGRectMake(0, 0, [self horizontalHeaderWidth:viewSize],
                    [self horizontalHeaderHeight]);
}

- (CGRect)verticalHeaderRect:(CGPoint)offset size:(CGSize)viewSize {
  return CGRectMake([self horizontalHeaderWidth:viewSize] + [self gutterSize],
                    [self horizontalHeaderHeight] + [self gutterSize],
                    [self verticalHeaderWidth],
                    [self verticalHeaderHeight:viewSize]);
}

- (CGRect)horizontalGutterRect:(CGPoint)offset size:(CGSize)viewSize {
  return CGRectMake(0, [self horizontalHeaderHeight],
                    [self horizontalHeaderWidth:viewSize], [self gutterSize]);
}

- (CGRect)verticalGutterRect:(CGPoint)offset size:(CGSize)viewSize {
  return CGRectMake([self horizontalHeaderWidth:viewSize],
                    [self horizontalHeaderHeight] + [self gutterSize],
                    [self gutterSize], [self verticalHeaderHeight:viewSize]);
}

- (CGRect)topRightRect:(CGPoint)offset size:(CGSize)viewSize {
  return CGRectMake([self horizontalHeaderWidth:viewSize], 0,
                    [self verticalHeaderWidth] + [self gutterSize],
                    [self horizontalHeaderHeight] + [self gutterSize]);
}

- (CGRect)gridRect:(CGPoint)offset size:(CGSize)viewSize {
  return CGRectMake(0, [self horizontalHeaderHeight] + [self gutterSize],
                    [self gridWidth:viewSize], [self gridHeight:viewSize]);
}

- (void)inContextDrawLine:(CGPoint)a to:(CGPoint)b {
  CGContextMoveToPoint(UIGraphicsGetCurrentContext(), a.x, a.y);
  CGContextAddLineToPoint(UIGraphicsGetCurrentContext(), b.x, b.y);
}

- (void)inContextDrawTriangle:(CGPoint)a b:(CGPoint)b c:(CGPoint)c {
  CGContextMoveToPoint(UIGraphicsGetCurrentContext(), a.x, a.y);
  CGContextAddLineToPoint(UIGraphicsGetCurrentContext(), b.x, b.y);
  CGContextAddLineToPoint(UIGraphicsGetCurrentContext(), c.x, c.y);
  CGContextAddLineToPoint(UIGraphicsGetCurrentContext(), a.x, a.y);
  CGContextFillPath(UIGraphicsGetCurrentContext());
}

- (void)inContextDrawDownTriangle:(CGPoint)point {
  CGPoint mid_p, top_l, top_r;
  mid_p = point;
  top_l.x = mid_p.x + arrowWidth;
  top_l.y = mid_p.y - arrowHeight;

  top_r.x = mid_p.x - arrowWidth;
  top_r.y = mid_p.y - arrowHeight;

  [self inContextDrawTriangle:mid_p b:top_l c:top_r];
}

- (void)inContextDrawRightTriangle:(CGPoint)point {
  CGPoint mid_p, top_l, top_r;
  mid_p.x = point.x + arrowHeight;
  mid_p.y = point.y;

  top_l.x = mid_p.x - arrowHeight;
  top_l.y = mid_p.y + arrowWidth;

  top_r.x = mid_p.x - arrowHeight;
  top_r.y = mid_p.y - arrowWidth;
  [self inContextDrawTriangle:mid_p b:top_l c:top_r];
}

- (void)inPathDrawTriangle:(CGMutablePathRef *)path
                         a:(CGPoint)a
                         b:(CGPoint)b
                         c:(CGPoint)c {
  CGPathMoveToPoint(*path, NULL, a.x, a.y);
  CGPathAddLineToPoint(*path, NULL, b.x, b.y);
  CGPathAddLineToPoint(*path, NULL, c.x, c.y);
  CGPathAddLineToPoint(*path, NULL, a.x, a.y);
}

- (void)inPathDrawDownTriangle:(CGMutablePathRef *)path point:(CGPoint)point {
  CGPoint mid_p, top_l, top_r;
  mid_p = point;
  top_l.x = mid_p.x + arrowWidth;
  top_l.y = mid_p.y - arrowHeight;

  top_r.x = mid_p.x - arrowWidth;
  top_r.y = mid_p.y - arrowHeight;

  [self inPathDrawTriangle:path a:mid_p b:top_l c:top_r];
}

- (void)inPathDrawRightTriangle:(CGMutablePathRef *)path point:(CGPoint)point {
  CGPoint mid_p, top_l, top_r;
  mid_p.x = point.x + arrowHeight;
  mid_p.y = point.y;

  top_l.x = mid_p.x - arrowHeight;
  top_l.y = mid_p.y + arrowWidth;

  top_r.x = mid_p.x - arrowHeight;
  top_r.y = mid_p.y - arrowWidth;
  [self inPathDrawTriangle:path a:mid_p b:top_l c:top_r];
}

- (BOOL)canBecomeFirstResponder {
  return YES;
}

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event {
  [super touchesBegan:touches withEvent:event];
  currentProposed = [NSSet setWithObject:[touches anyObject]];
}

- (void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event {
  [super touchesCancelled:touches withEvent:event];
  currentProposed = [NSSet set];
}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event {
  [super touchesMoved:touches withEvent:event];
  currentProposed = [NSSet setWithObject:[touches anyObject]];
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event {
  [super touchesEnded:touches withEvent:event];
  currentProposed = [NSSet set];

  CGFloat totalSources = (CGFloat)[self.patchbayDelegate totalSources];
  CGFloat pinDX = [self gridWidth:imageView.frame.size] / totalSources;

  CGFloat totalDestinations =
      (CGFloat)[self.patchbayDelegate totalDestinations];
  CGFloat pinDY = [self gridHeight:imageView.frame.size] / totalDestinations;

  // loop through all touchs
  for (UITouch *touch in touches) {
    CGPoint location = [touch locationInView:imageView];
    CGRect grid = [self gridRect:CGPointMake(0, 0) size:imageView.frame.size];

    CGRect horizontalHeaderRect = [self horizontalHeaderRect:CGPointMake(0,0) size:imageView.frame.size];
    CGRect verticalHeaderRect = [self verticalHeaderRect:CGPointMake(0,0) size:imageView.frame.size];

    // filter out all non begin and move phases
    if (CGRectContainsPoint(grid, location)) {

      // modify the location to inside the grid
      location.x -= grid.origin.x;
      location.y -= grid.origin.y;

      // determine the corresponding row and column
      int col = (int)floor(location.x / pinDX) + 1;
      int row = (int)floor(location.y / pinDY) + 1;

      auto srcIndex = [self.patchbayDelegate sourceTotalToIndex:col];
      auto dstIndex = [self.patchbayDelegate destinationTotalToIndex:row];

      // check the bounds
      if (([self.patchbayDelegate isSourceDeviceInBounds:srcIndex]) &&
          ([self.patchbayDelegate isDestinationDeviceInBounds:dstIndex])) {
        device_channel_t empty = {0, 0};
        if ([self.patchbayDelegate isPatchedFrom:srcIndex to:dstIndex]) {
          [self.patchbayDelegate setPatchedFrom:empty to:dstIndex andRemove:srcIndex];
        } else {
          [self.patchbayDelegate setPatchedFrom:srcIndex to:dstIndex andRemove:empty];
        }
      }
    }
    else if (CGRectContainsPoint(horizontalHeaderRect, location)) {
      // modify the location to inside the grid
      location.x -= horizontalHeaderRect.origin.x;
      location.y -= horizontalHeaderRect.origin.y;

      // determine the corresponding row and column
      int col = (int)floor(location.x / pinDX) + 1;

      auto srcIndex = [self.patchbayDelegate sourceTotalToIndex:col];
      [self.patchbayDelegate toggleCollapseInput:srcIndex.port];
      imageView.layer.contents = (id)[[self generateStaticImage] CGImage];
      [self updateDynamicPaths:[NSSet set]];

    }
    else if (CGRectContainsPoint(verticalHeaderRect, location)) {
      // modify the location to inside the grid
      location.x -= verticalHeaderRect.origin.x;
      location.y -= verticalHeaderRect.origin.y;

      // determine the corresponding row and column
      int row = (int)floor(location.y / pinDY) + 1;

      auto dstIndex = [self.patchbayDelegate destinationTotalToIndex:row];
      [self.patchbayDelegate toggleCollapseOutput:dstIndex.port];
      imageView.layer.contents = (id)[[self generateStaticImage] CGImage];
      [self updateDynamicPaths:[NSSet set]];

    }
  }
}

- (void)redrawTimerTick:(id)sender {
  [self updateDynamicPaths:currentProposed];
}

@end
