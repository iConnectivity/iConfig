/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifdef DEBUG

#import <Foundation/Foundation.h>
#import <sys/socket.h>
#import <netinet/in.h>
#import "LibTypes.h"

@interface ICServer : NSObject {
  /* master file descriptor list */
  fd_set master;

  /* temp file descriptor list for select() */
  fd_set read_fds;

  /* server address */
  struct sockaddr_in serveraddr;

  /* client address */
  struct sockaddr_in clientaddr;

  /* maximum file descriptor number */
  int fdmax;

  /* listening socket descriptor */
  int listener;

  /* newly accept()ed socket descriptor */
  int newfd;

  /* buffer for client data */
  char buf[1024];
  int nbytes;

  bool stopServer;
}

- (void)start;
- (void)stop;

- (void)send:(int)port string:(NSString *)str;
- (void)send:(int)port data:(NSData *)data;

- (void)broadcastStr:(NSString *)string;
- (void)broadcast:(NSData *)data;

- (NSString *)getIPAddress;

@end
#endif  // DEBUG
