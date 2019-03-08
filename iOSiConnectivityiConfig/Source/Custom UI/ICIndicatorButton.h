/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import <UIKit/UIKit.h>

@interface ICIndicatorButton : UIView {
  UIButton *button;
  UIImageView *indicatorImageView;
}

@property(strong, nonatomic) UIButton *button;
@property(strong, nonatomic) UIImageView *indicatorImageView;

@property(nonatomic, setter=setTag :) NSInteger tag;

@property(nonatomic, setter=setSelected :, getter=isSelected) BOOL selected;

+ (UIImage *)imageFromColor:(UIColor *)color;

- (id)initWithFrame:(CGRect)frame
     indicatorFrame:(CGRect)indicatorFrame
              title:(NSString *)title;

// default is nil. title is assumed to be single line
- (void)setTitle:(NSString *)title forState:(UIControlState)state;

// default if nil. use opaque white
- (void)setTitleColor:(UIColor *)color forState:(UIControlState)state;

// default is nil. use 50% black
- (void)setTitleShadowColor:(UIColor *)color forState:(UIControlState)state;

// default is nil. should be same size if different for different states
- (void)setImage:(UIImage *)image forState:(UIControlState)state;

- (NSString *)title;

- (void)setBackgroundColor:(UIColor *)color forState:(UIControlState)state;

// default is nil.
- (void)setBackgroundImage:(UIImage *)image forState:(UIControlState)state;

- (void)addTarget:(id)target
              action:(SEL)action
    forControlEvents:(UIControlEvents)controlEvents;

- (BOOL)isSelected;
- (void)setSelected:(BOOL)selected;

- (BOOL)isEnabled;
- (void)setEnabled:(BOOL)enabled;

- (void)setTag:(NSInteger)tag;

- (void)setIndicatorImage:(UIImage *)image;

@end
