/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifdef DEBUG

#import <ifaddrs.h>
#import <arpa/inet.h>
#import <stdio.h>
#import "ICServer.h"
#import "NSString+NSStringExtension.h"

@implementation ICServer

// listen on port 2020
#define PORT 2020

- (void)start {
  FD_ZERO(&master);
  FD_ZERO(&read_fds);

  if ((listener = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
    // ERROR
    perror("Server-socket() error.");
    exit(1);
  }

  NSLog(@"Server-socket() is OK...");

  /* bind */
  struct sockaddr_in sin;
  memset(&sin, 0, sizeof(sin));
  sin.sin_len = sizeof(sin);
  sin.sin_family = AF_INET;  // or AF_INET6 (address family)
  sin.sin_port = htons(PORT);
  sin.sin_addr.s_addr = INADDR_ANY;

  if (bind(listener, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
    // Handle the error.
    perror("Server-bind() error");
    exit(1);
  }
  NSLog(@"Server-bind() OK...");
  NSLog(@"Server @%@", [self getIPAddress]);

  // Start worker thread
	dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
		@autoreleasepool {
			NSLog(@"Worker thread dispatched");
			if (listen(listener, 10) == 0) {
    NSLog(@"Socket-listen() is OK...");

    /* add the listener to the master set */
    FD_SET(listener, &master);
    /* keep track of the biggest file descriptor */
    fdmax = listener; /* so far, it's this one */

    // loop
    while (!stopServer) {
      /* copy it */
      read_fds = master;

      if (select(fdmax + 1, &read_fds, NULL, NULL, NULL) == -1) {
        perror("Server-select() error");
        exit(1);
      }

      /* run through the existing connections looking for data to be read */
      for (int i = 0; i <= fdmax; i++) {
        if (FD_ISSET(i, &read_fds)) {
          /* we got one... */
          if (i == listener) {
            /* handle new connections */

            socklen_t addrlen;

            addrlen = sizeof(clientaddr);

            if ((newfd = accept(listener, (struct sockaddr *)&clientaddr,
                                &addrlen)) == -1) {
              perror("Server-accept() error");
            } else {
              NSLog(@"Server-accept() ok");

              // redirect stdout to new fd
              dup2(newfd, STDOUT_FILENO);
              dup2(newfd, STDERR_FILENO);

              FD_SET(newfd, &master);  // Add to master set
              if (newfd > fdmax) {
                // keep track of the maximum
                fdmax = newfd;
              }

              NSLog(@"New connection socket %d", newfd);
            }
          } else {
            /* handle data from a client */
            memset(buf, 0, sizeof(buf));
            if ((nbytes = recv(i, buf, sizeof(buf), 0)) <= 0) {
              /* go error or connection closed by client */
              if (nbytes == 0) {
                // connection closed
                NSLog(@"Socket %d hung up", i);
              } else {
                perror("recv() error");
              }

              /* close it */
              close(i);

              /* remove from master set */
              FD_CLR(i, &master);
            }
          }
        }
      }
    }
			}
			else {
    perror("Server-listen() error");
			}
			
			for (int i = 0; i < fdmax; i++) {
    if (FD_ISSET(i, &master)) {
      close(i);
    }
			}
}
});
}

- (void)stop {
  stopServer = true;
}

- (void)send:(int)port string:(NSString *)str {
  return [self
      send:port
      data:[NSData
               dataWithBytes:[str cStringUsingEncoding:NSASCIIStringEncoding]
                      length:[str length]]];
}

- (void)send:(int)port data:(NSData *)data {
  send(port, [data bytes], [data length], 0);
}

- (void)broadcastStr:(NSString *)string {
  NSData *data =
      [NSData dataWithBytes:[string cStringUsingEncoding:NSASCIIStringEncoding]
                     length:[string length]];
  return [self broadcast:data];
}

- (void)broadcast:(NSData *)data {
  for (u_int i = 0; i <= fdmax; i++) {
    if (FD_ISSET(i, &master) && i != listener) {
      send(i, [data bytes], [data length], 0);
    }
  }
}

- (NSString *)getIPAddress {
  NSString *address = @"error";
  struct ifaddrs *interfaces = NULL;
  struct ifaddrs *temp_addr = NULL;
  int success = 0;
  // retrieve the current interfaces - returns 0 on success
  success = getifaddrs(&interfaces);
  if (success == 0) {
    // Loop through linked list of interfaces
    temp_addr = interfaces;
    while (temp_addr != NULL) {
      if (temp_addr->ifa_addr->sa_family == AF_INET) {
        // Check if interface is en0 which is the wifi connection on the iPhone
        if ([[NSString stringWithUTF8String:temp_addr->ifa_name]
                isEqualToString:@"en0"]) {
          // Get NSString from C String
          address = [NSString
              stringWithUTF8String:inet_ntoa(((struct sockaddr_in *)
                                              temp_addr->ifa_addr)->sin_addr)];
        }
      }
      temp_addr = temp_addr->ifa_next;
    }
  }
  // Free memory
  freeifaddrs(interfaces);
  return address;
}

@end

#endif  // DEBUG
