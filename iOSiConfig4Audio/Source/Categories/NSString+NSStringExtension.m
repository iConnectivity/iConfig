/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifdef DEBUG
#import "NSString+NSStringExtension.h"

#import <stdio.h>
#import <stdlib.h>
#import <string.h>

unsigned char strToChar(char a, char b);
unsigned char strToChar(char a, char b) {
  char encoder[3] = {'\0', '\0', '\0'};
  encoder[0] = a;
  encoder[1] = b;
  return (char)strtol(encoder, NULL, 16);
}

@implementation NSString (NSStringExtension)

- (NSData *)decodeFromHexidecimal;
{
  NSString *temp = [[[self uppercaseString]
      componentsSeparatedByCharactersInSet:
          [[NSCharacterSet characterSetWithCharactersInString:
                               @"0123456789ABCDEF"] invertedSet]]
      componentsJoinedByString:@""];
  const char *bytes = [temp cStringUsingEncoding:NSUTF8StringEncoding];
  NSUInteger length = strlen(bytes);
  unsigned char *r = (unsigned char *)malloc(length / 2 + 1);
  unsigned char *index = r;

  while ((*bytes) && (*(bytes + 1))) {
    *index = strToChar(*bytes, *(bytes + 1));
    index++;
    bytes += 2;
  }
  *index = '\0';

  NSData *result = [NSData dataWithBytes:r length:length / 2];
  free(r);

  return result;
}

@end

#endif  // DEBUG
