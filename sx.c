/**
 * @brief   Send file via XMODEM
 * @author  Thomas Cherryhomes
 * @email   thom dot cherryhomes at gmail dot com
 * @license gpl v. 3, see LICENSE for details.
 * @verbose written for 4.2BSD and similar ancient UNIces
 */

/**
 * Build with:
 * cc -o sx sx.c
 */

#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sgtty.h>

/* Defines for XMODEM control characters borrowed from ASCII */
#define ACK 0x06
#define NAK 0x15
#define SOH 0x01
#define CAN 0x18
#define EOT 0x04

#define BLOCK_SIZE 128

/* current block # */
unsigned char   blknum;

/* are we done with transmission? */
unsigned char   is_eot = 0;

/* current protocol state */
enum _state
{
    INIT,
        ACKNAK,
        NEXTBLOCK,
        BUILDBLOCK,
        SENDBLOCK,
        SENDEOT,
        DONE
} state = INIT;

/* stdio file pointer for sent file. */
FILE * fp;

/**
 * A single XMODEM block, with header and checksum
 */
struct _xmodem_block {
    char    soh;
    unsigned char   blknum;
    unsigned char   notblknum;
    unsigned char   data[128];
    unsigned char   cksum;
}                       block;

struct sgttyb   mode;

cksum (buf, len)
unsigned char  *buf;
int     len;
{
    unsigned char   chk = 0;

    for (chk = 0; len != 0; len--)
        chk += *buf++;

    return (unsigned char) chk;
}

setterm () {
    ioctl (1, TIOCGETP, &mode);
    mode.sg_flags |= RAW | ANYP;
    ioctl (1, TIOCSETP, &mode);
}

resetterm () {
    ioctl (1, TIOCGETP, &mode);
    mode.sg_flags &= ~RAW;
    ioctl (1, TIOCSETP, &mode);
}

nextblock () {
    if (is_eot == 1)
        state = SENDEOT;
    else {
        blknum++;
        state = BUILDBLOCK;
    }
}

buildblock () {
    unsigned char   header_blknum = blknum + 1;
    unsigned char   header_notblknum;

    header_notblknum = 0xFF;
    header_notblknum -= header_blknum;

    block.soh = SOH;
    block.blknum = header_blknum;
    block.notblknum = header_notblknum;

    bzero (block.data, BLOCK_SIZE);

    if (fread (block.data, 1, 128, fp))
        is_eot = 1;

    block.cksum = cksum (block.data, BLOCK_SIZE);
    state = SENDBLOCK;
}

sendblock () {
    fwrite (&block, sizeof (struct _xmodem_block)  , 1, stdout);

    state = ACKNAK;
}

sendeot () {
    char    c = 0;

    putchar (EOT);
    c = getchar ();

    state = DONE;
}

init (fn)
char   *fn;
{
    fp = fopen (fn, "r");

    if (!fp) {
        perror ("Could not open file");
        state = DONE;
        return 0;
    }

    printf ("Sending %s via XMODEM. Start transfer now.\n", fn);

    buildblock ();
    setterm ();

    state = ACKNAK;
}

acknak () {
    char    c = getchar ();

    if (c == CAN)
        state = DONE;
    else
        if (c == ACK)
            state = NEXTBLOCK;
        else
            if (c == NAK)
                state = SENDBLOCK;
}

done () {
    if (fp)
        fclose (fp);

    resetterm ();
    exit (0);
}

sx (fn)
char   *fn;
{
    while (1) {
        switch (state) {
            case INIT: 
                init (fn);
                break;
            case ACKNAK: 
                acknak ();
                break;
            case NEXTBLOCK: 
                nextblock ();
                break;
            case BUILDBLOCK: 
                buildblock ();
                break;
            case SENDBLOCK: 
                sendblock ();
                break;
            case SENDEOT: 
                sendeot ();
                break;
            case DONE: 
                done ();
                break;
        }

    }
}

main (argc, argv)
int     argc;
char  **argv;
{
    if (argc < 2) {
        printf ("%s <fn>\n", argv[0]);
        return 0;
    }

    return sx (argv[1]);
}
