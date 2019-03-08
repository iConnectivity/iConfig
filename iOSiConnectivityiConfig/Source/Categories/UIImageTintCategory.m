/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import "UIImageTintCategory.h"

@implementation UIImage (TintAndTile)

- (UIImage *)tintedImageUsingColor:(UIColor *)tintColor {
  UIGraphicsBeginImageContext(self.size);
  CGRect drawRect = CGRectMake(0, 0, self.size.width, self.size.height);
  [self drawInRect:drawRect];
  [tintColor set];
  UIRectFillUsingBlendMode(drawRect, kCGBlendModeMultiply);
  UIImage *tintedImage = UIGraphicsGetImageFromCurrentImageContext();
  UIGraphicsEndImageContext();
  return tintedImage;
}

- (UIImage *)tiledImageOfSize:(CGSize)size {
  UIGraphicsBeginImageContext(size);
  CGContextRef imageContext = UIGraphicsGetCurrentContext();
  CGContextDrawTiledImage(imageContext, (CGRect) {CGPointZero, size},
                          self.CGImage);
  UIImage *finishedImage = UIGraphicsGetImageFromCurrentImageContext();
  UIGraphicsEndImageContext();

  return finishedImage;
}

@end
