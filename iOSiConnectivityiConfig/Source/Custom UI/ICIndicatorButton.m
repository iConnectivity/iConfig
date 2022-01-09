/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import "ICIndicatorButton.h"

@implementation ICIndicatorButton

@synthesize button;
@synthesize indicatorImageView;
@synthesize selected;
@synthesize tag;

+ (UIImage *)imageFromColor:(UIColor *)color {
  CGRect rect = CGRectMake(0, 0, 1, 1);
  UIGraphicsBeginImageContext(rect.size);
  CGContextRef context = UIGraphicsGetCurrentContext();
  CGContextSetFillColorWithColor(
      context, [[color colorWithAlphaComponent:0.99f] CGColor]);
  CGContextFillRect(context, rect);
  UIImage *img = UIGraphicsGetImageFromCurrentImageContext();
  UIGraphicsEndImageContext();
  return img;
}

- (id)initWithFrame:(CGRect)frame
     indicatorFrame:(CGRect)indicatorFrame
              title:(NSString *)title {
  self = [super initWithFrame:frame];
  if (self) {
    // Initialization code
    self.button = [UIButton buttonWithType:UIButtonTypeCustom];
    [self.button
        setFrame:CGRectMake(0, 0, frame.size.width, frame.size.height)];

    [self.button setTitle:title forState:UIControlStateNormal];
    [self.button setAutoresizingMask:UIViewAutoresizingFlexibleWidth |
                                     UIViewAutoresizingFlexibleHeight |
                                     UIViewAutoresizingFlexibleRightMargin |
                                     UIViewAutoresizingFlexibleLeftMargin |
                                     UIViewAutoresizingFlexibleTopMargin |
                                     UIViewAutoresizingFlexibleBottomMargin];

    if ([[UIDevice currentDevice] userInterfaceIdiom] ==
        UIUserInterfaceIdiomPhone) {
      [self.button.titleLabel setFont:[UIFont boldSystemFontOfSize:15.0]];

      [self.button.titleLabel setShadowColor:[UIColor darkGrayColor]];
      [self.button.titleLabel setShadowOffset:CGSizeMake(0.5, 0.5)];
    } else {
      [self.button.titleLabel setFont:[UIFont boldSystemFontOfSize:22.0]];

      [self.button.titleLabel setShadowColor:[UIColor darkGrayColor]];
      [self.button.titleLabel setShadowOffset:CGSizeMake(1.0, 1.0)];
    }
    [self.button.titleLabel setNumberOfLines:0];
    [self.button.titleLabel setLineBreakMode:NSLineBreakByWordWrapping];
    [self.button.titleLabel setTextAlignment:NSTextAlignmentCenter];

    // lower the text and push it left to center it
    self.button.titleEdgeInsets = UIEdgeInsetsMake(10.0, 10.0, 10.0, 10.0);

    [self addSubview:self.button];

    self.indicatorImageView =
        [[UIImageView alloc] initWithFrame:indicatorFrame];
    [self.indicatorImageView
        setAutoresizingMask:UIViewAutoresizingFlexibleRightMargin |
                            UIViewAutoresizingFlexibleLeftMargin |
                            UIViewAutoresizingFlexibleTopMargin |
                            UIViewAutoresizingFlexibleBottomMargin];

    [self.inputAccessoryView setContentMode:UIViewContentModeScaleAspectFit];

    [self addSubview:self.indicatorImageView];

    [self setAutoresizingMask:UIViewAutoresizingFlexibleWidth |
                              UIViewAutoresizingFlexibleHeight |
                              UIViewAutoresizingFlexibleRightMargin |
                              UIViewAutoresizingFlexibleLeftMargin |
                              UIViewAutoresizingFlexibleTopMargin |
                              UIViewAutoresizingFlexibleBottomMargin];
  }
  return self;
}

- (void)setTitle:(NSString *)title forState:(UIControlState)state {
  [self.button setTitle:title forState:state];
}

- (NSString *)title {
  return self.button.titleLabel.text;
}

- (void)setTitleColor:(UIColor *)color forState:(UIControlState)state {
  [self.button setTitleColor:color forState:state];
}

- (void)setTitleShadowColor:(UIColor *)color forState:(UIControlState)state {
  [self.button setTitleShadowColor:color forState:state];
}

- (void)setImage:(UIImage *)image forState:(UIControlState)state {
  [self.button setImage:image forState:state];

  // the space between the image and text
  CGFloat spacing = -2.0;

  // get the size of the elements here for readability
  CGSize imageSize = self.button.imageView.frame.size;
  CGSize titleSize;

  // lower the text and push it left to center it
  self.button.titleEdgeInsets = UIEdgeInsetsMake(
      0.0, -imageSize.width, -(imageSize.height + spacing), 0.0);

  // the text width might have changed (in case it was shortened before due to
  // lack of space and isn't anymore now), so we get the frame size again
  titleSize = button.titleLabel.frame.size;

  // raise the image and push it right to center it
  self.button.imageEdgeInsets = UIEdgeInsetsMake(-(titleSize.height + spacing),
                                                 0.0, 0.0, -titleSize.width);
}

- (void)setBackgroundColor:(UIColor *)color forState:(UIControlState)state {
  [self.button setBackgroundImage:[ICIndicatorButton imageFromColor:color]
                         forState:state];
}

- (void)setBackgroundImage:(UIImage *)image forState:(UIControlState)state {
  [self.button setBackgroundImage:image forState:state];
}

- (void)addTarget:(id)target
              action:(SEL)action
    forControlEvents:(UIControlEvents)controlEvents {
  [self.button addTarget:target action:action forControlEvents:controlEvents];
}

- (BOOL)isSelected {
  return [self.button isSelected];
}

- (void)setSelected:(BOOL)_selected {
  [self.button setSelected:_selected];
}

- (BOOL)isEnabled {
  return self.button.enabled;
}

- (void)setEnabled:(BOOL)enabled {
  [self.button setEnabled:enabled];
}

- (void)setTag:(NSInteger)tagValue {
  super.tag = tagValue;
  self.button.tag = tagValue;
}

- (void)setIndicatorImage:(UIImage *)image {
  [self.indicatorImageView setImage:image];
}

@end
