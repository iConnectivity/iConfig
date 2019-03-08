/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import "NSString+RotatedDraw.h"

@implementation NSString (RotatedDraw)

- (void)drawAtPoint:(CGPoint)point
           forWidth:(CGFloat)width
           withFont:(UIFont *)font
              angle:(CGFloat)angle
      lineBreakMode:(NSLineBreakMode)lineBreakMode {

  CGAffineTransform translate =
      CGAffineTransformMakeTranslation(point.x, point.y);
  CGAffineTransform rotate = CGAffineTransformMakeRotation(angle);

  CGContextConcatCTM(UIGraphicsGetCurrentContext(), translate);
  CGContextConcatCTM(UIGraphicsGetCurrentContext(), rotate);

  //  [self draw]
  CGSize textSize =
      [self sizeWithFont:font forWidth:width lineBreakMode:lineBreakMode];

  [self drawAtPoint:CGPointMake(-0.5f * textSize.width, -0.5f * textSize.height)
           forWidth:width
           withFont:font
      lineBreakMode:lineBreakMode];

  CGContextConcatCTM(UIGraphicsGetCurrentContext(),
                     CGAffineTransformInvert(rotate));
  CGContextConcatCTM(UIGraphicsGetCurrentContext(),
                     CGAffineTransformInvert(translate));
}

- (void)drawInRect:(CGRect)rect
          withFont:(UIFont *)font
             angle:(CGFloat)angle
     lineBreakMode:(NSLineBreakMode)lineBreakMode {

  CGAffineTransform translate =
      CGAffineTransformMakeTranslation(rect.origin.x, rect.origin.y);
  CGAffineTransform rotate = CGAffineTransformMakeRotation(angle);

  CGContextConcatCTM(UIGraphicsGetCurrentContext(), translate);
  CGContextConcatCTM(UIGraphicsGetCurrentContext(), rotate);

  //  [self draw]
  CGSize textSize = [self sizeWithFont:font
                     constrainedToSize:rect.size
                         lineBreakMode:lineBreakMode];

  [self drawInRect:CGRectMake(0, textSize.height * -0.5f, rect.size.width,
                              textSize.height)
           withFont:font
      lineBreakMode:lineBreakMode
          alignment:NSTextAlignmentCenter];

  CGContextConcatCTM(UIGraphicsGetCurrentContext(),
                     CGAffineTransformInvert(rotate));
  CGContextConcatCTM(UIGraphicsGetCurrentContext(),
                     CGAffineTransformInvert(translate));
}

@end
