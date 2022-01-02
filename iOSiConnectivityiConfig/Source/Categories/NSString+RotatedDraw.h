/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

@interface NSString (RotatedDraw)

- (void)drawAtPoint:(CGPoint)point
           forWidth:(CGFloat)width
           withFont:(UIFont *)font
              angle:(CGFloat)angle
      lineBreakMode:(NSLineBreakMode)lineBreakMode;

- (void)drawInRect:(CGRect)rect
          withFont:(UIFont *)font
             angle:(CGFloat)angle
     lineBreakMode:(NSLineBreakMode)lineBreakMode;

@end
