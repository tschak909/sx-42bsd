# sx (Send XMODEM for ancient 4.2BSD UNIX)

## Abstract 

This is an implementation of the send portion of the XMODEM protocol, for ancient UNIX implementations, such as 4.2BSD.

As such, it is an example of:

* K&R C
* Ancient IOCTLs to set TTY discipline, before termios.
* pre ANSI functions for buffers (bzero instead of memset)
* Uses 8-bit Checksum instead of 16-bit CRC.

Because XMODEM is entirely receiver driven, a mechanism for timing out the sender is not needed, and thus, calls to select() (or doing an NREAD ioctl()) is not required.

## Author's Caveat:

While this is completely debugged, I wound up not using it, due to the fact that my configuration of 4.2BSD and SIMH do not play well together when an 8-bit line discipline is used on either the console or DZ controller. So I am merely leaving this here as a historical example, while I go write a Kermit implementation.

## Author

Thom Cherryhomes <thom dot cherryhomes at gmail dot com>

## License

GPL v. 3. See LICENSE for details.
