racetroute

racetroute is a traceroute-like program for finding parallelism in internet
infrastructure. It does this by sending back-to-back packet trains and then
observing the order of ICMP "time exceeded" messages that come back.

racetroute can perform several different tests. The default is to send UDP
test packets with fixed source and destination ports. This test finds
per-packet load balancing. However, this is rare: most ECMP (equal cost
multipath, RFC 2992) implementations try to make sure that packets belonging
to the same session or flow are sent over the same path, to avoid reordering
and the associated potential performance issues. Other tests vary the source
and/or destination port numbers. Test options are specified by a set of bits
combined into a decimal number as follows:

UDP: 1
TCP: 2
ICMP: 4
No UDP checksum: 8
Vary TCP/UDP source port / ICMP seq (+1): 16
Vary TCP/UDP destination port / ICMP id (-1): 32
Vary TCP/UDP destination port / ICMP id (+6): 64
Vary data: 128
ToS: bits 8 - 15, multiply ToS by 256 or DSCP by 1024

One or more tests can be specified with the -e option. For instance:

iljitsch@minerva racetroute % sudo ./racetroute -e 1 17 www.ripe.net      
 4:  *  *  *  *  *  *  *  *  *  * 
 5:  0  1  2  3  4  5  6  7  8  9  172.71.92.2
 6:  0  1  2  3  4  5  !  !  !  !  104.18.21.44
 7:  0  !  !  !  !  !  !  !  !  !  104.18.21.44
Test 1: reached destination at hop 7 for www.ripe.net
 4:  *  *  *  *  *  *  *  *  *  * 
 5:  4  1                          172.71.100.2
           5  7        9           172.70.44.2
                 2  8              172.71.96.2
                          0        172.71.92.2
                             3  6  141.101.65.2
 6:  6  3  4  0  8  5  1  2  9  7  104.18.21.44
 7:  6  3  4  0  5  8  2  1  7  9  104.18.21.44
Test 17: found reordering at hop 5 for www.ripe.net

Test 1 doesn't find reordering. However, test 17 sees return packets from four
different IP addresses as well as reordering. For the most part, but not
entirely, packets from the same return address are not reordered. So this
shows parallelism at layer 3.

Hops 6 and 7 are the destination. For test 17 packets from that return address
are also reordered. It is possible that this is a reflection of the reordering
at hop 5, or there may be layer 2 based ECMP between hops 5 and 6.

Please note that racetroute was written in 2010, with no real documentation.
There have been some recent changes, but for some options their usage and even
purpose are unclear at this time.

The racetroute program supports the following options:

-a colorpalette

By default racetroute will display packet reordering (defined as a packet with
a sequence number smaller than a previous packet for a certain hop) in red
and reordering between different addresses for a hop, but not within that same
address, in blue. This is color palette 1. Color palette 0 turns off these
ANSI color codes.

-m modulus offset
-p <process>

These options make it possible to run multiple racetroute instances simultaneously.

-v verbosity

Output level. 0 is silent, 1 is the default, 2 also reports reordering on the
return path specifically if this can be detected from the IPv4 ID field in the
returned ICMP messages. (Note that a good number of routers set this field to
0 for ICMP messages.) Values higher than 2 increasingly show more detail.

-i interface

The output interface.

-t start ttl

Specify the first hop to test.

-s size

Size of test packets.

-c packetcount

Number of packets in a packet train. The default is 10.

-e test type(s)

See discussion above.

-u unreachable-timeout

After how many hops with no return packets the program will give up.

-o [-f] <destination(s) (file)

Work with input and/or output files.

LICENSE

This software will be made available under an open source license. Currently,
no license has been selected yet, so at this time:

copyright (c) 2010, 2023 Iljitsch van Beijnum
