  TCP Header Format


    0                   1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |          Source Port          |       Destination Port        |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                        Sequence Number                        |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                    Acknowledgment Number                      |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |  Data |           |U|A|P|R|S|F|                               |
   | Offset| Reserved  |R|C|S|S|Y|I|            Window             |
   |       |           |G|K|H|T|N|N|                               |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |           Checksum            |         Urgent Pointer        |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                    Options                    |    Padding    |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                             data                              |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

                            TCP Header Format

  Source Port:  16 bits

    The source port number.

  Destination Port:  16 bits

    The destination port number.
  
  Sequence Number:  32 bits

    The sequence number of the first data octet in this segment (except
    when SYN is present). If SYN is present the sequence number is the
    initial sequence number (ISN) and the first data octet is ISN+1.

  Acknowledgment Number:  32 bits

    If the ACK control bit is set this field contains the value of the
    next sequence number the sender of the segment is expecting to
    receive.  Once a connection is established this is always sent.

  Data Offset:  4 bits

    The number of 32 bit words in the TCP Header.  This indicates where
    the data begins.  The TCP header (even one including options) is an
    integral number of 32 bits long.

  Control Bits:  6 bits (from left to right):

    URG:  Urgent Pointer field significant
    ACK:  Acknowledgment field significant
    PSH:  Push Function
    RST:  Reset the connection
    SYN:  Synchronize sequence numbers
    FIN:  No more data from sender

  Window:  16 bits

    The number of data octets beginning with the one indicated in the
    acknowledgment field which the sender of this segment is willing to
    accept.

  Checksum:  16 bits

    The checksum field is the 16 bit one's complement of the one's
    complement sum of all 16 bit words in the header and text.  If a
    segment contains an odd number of header and text octets to be
    checksummed, the last octet is padded on the right with zeros to
    form a 16 bit word for checksum purposes.  The pad is not
    transmitted as part of the segment.  While computing the checksum,
    the checksum field itself is replaced with zeros.

    The checksum also covers a 96 bit pseudo header conceptually
    prefixed to the TCP header.  This pseudo header contains the Source
    Address, the Destination Address, the Protocol, and TCP length.
    This gives the TCP protection against misrouted segments.  This
    information is carried in the Internet Protocol and is transferred
    across the TCP/Network interface in the arguments or results of
    calls by the TCP on the IP.

                     +--------+--------+--------+--------+
                     |           Source Address          |
                     +--------+--------+--------+--------+
                     |         Destination Address       |
                     +--------+--------+--------+--------+
                     |  zero  |  PTCL  |    TCP Length   |
                     +--------+--------+--------+--------+

      The TCP Length is the TCP header length plus the data length in
      octets (this is not an explicitly transmitted quantity, but is
      computed), and it does not count the 12 octets of the pseudo
      header.

  Options:  variable

    Options may occupy space at the end of the TCP header and are a
    multiple of 8 bits in length.  All options are included in the
    checksum.

      Maximum Segment Size

        +--------+--------+---------+--------+
        |00000010|00000100|   max seg size   |
        +--------+--------+---------+--------+
         Kind=2   Length=4

        Maximum Segment Size Option Data:  16 bits

          If this option is present, then it communicates the maximum
          receive segment size at the TCP which sends this segment.
          This field must only be sent in the initial connection request
          (i.e., in segments with the SYN control bit set).  If this
          option is not used, any segment size is allowed.

  Padding:  variable

    The TCP header padding is used to ensure that the TCP header ends
    and data begins on a 32 bit boundary.  The padding is composed of
    zeros.


The maintenance of a TCP connection requires the remembering of several 
variables.  We conceive of these variables being stored in a connection 
record called a Transmission Control Block or TCB.

    Send Sequence Variables

      SND.UNA - send unacknowledged
      SND.NXT - send next
      SND.WND - send window
      SND.UP  - send urgent pointer
      SND.WL1 - segment sequence number used for last window update
      SND.WL2 - segment acknowledgment number used for last window
                update
      ISS     - initial send sequence number

    Receive Sequence Variables

      RCV.NXT - receive next
      RCV.WND - receive window
      RCV.UP  - receive urgent pointer
      IRS     - initial receive sequence number

  The following diagrams may help to relate some of these variables to
  the sequence space.

  Send Sequence Space

                   1         2          3          4
              ----------|----------|----------|----------
                     SND.UNA    SND.NXT    SND.UNA
                                          +SND.WND

        1 - old sequence numbers which have been acknowledged
        2 - sequence numbers of unacknowledged data
        3 - sequence numbers allowed for new data transmission
        4 - future sequence numbers which are not yet allowed

                          Send Sequence Space

                               Figure 4.



  The send window is the portion of the sequence space labeled 3 in
  figure 4.

  Receive Sequence Space

                       1          2          3
                   ----------|----------|----------
                          RCV.NXT    RCV.NXT
                                    +RCV.WND

        1 - old sequence numbers which have been acknowledged
        2 - sequence numbers allowed for new reception
        3 - future sequence numbers which are not yet allowed

                         Receive Sequence Space

                               Figure 5.

    Current Segment Variables

      SEG.SEQ - segment sequence number
      SEG.ACK - segment acknowledgment number
      SEG.LEN - segment length
      SEG.WND - segment window
      SEG.UP  - segment urgent pointer
      SEG.PRC - segment precedence value

  A connection progresses through a series of states during its
  lifetime.  The states are:  LISTEN, SYN-SENT, SYN-RECEIVED,
  ESTABLISHED, FIN-WAIT-1, FIN-WAIT-2, CLOSE-WAIT, CLOSING, LAST-ACK,
  TIME-WAIT, and the fictional state CLOSED.  CLOSED is fictional
  because it represents the state when there is no TCB, and therefore,
  no connection.  Briefly the meanings of the states are:

    LISTEN - represents waiting for a connection request from any remote
    TCP and port.

    SYN-SENT - represents waiting for a matching connection request
    after having sent a connection request.

    SYN-RECEIVED - represents waiting for a confirming connection
    request acknowledgment after having both received and sent a
    connection request.

    ESTABLISHED - represents an open connection, data received can be
    delivered to the user.  The normal state for the data transfer phase
    of the connection.

    FIN-WAIT-1 - represents waiting for a connection termination request
    from the remote TCP, or an acknowledgment of the connection
    termination request previously sent.

    FIN-WAIT-2 - represents waiting for a connection termination request
    from the remote TCP.

    CLOSE-WAIT - represents waiting for a connection termination request
    from the local user.

    CLOSING - represents waiting for a connection termination request
    acknowledgment from the remote TCP.

    LAST-ACK - represents waiting for an acknowledgment of the
    connection termination request previously sent to the remote TCP
    (which includes an acknowledgment of its connection termination
    request).

    TIME-WAIT - represents waiting for enough time to pass to be sure
    the remote TCP received the acknowledgment of its connection
    termination request.

    CLOSED - represents no connection state at all.

  A TCP connection progresses from one state to another in response to
  events.  The events are the user calls, OPEN, SEND, RECEIVE, CLOSE,
  ABORT, and STATUS; the incoming segments, particularly those
  containing the SYN, ACK, RST and FIN flags; and timeouts.

  The state diagram in figure 6 illustrates only state changes, together
  with the causing events and resulting actions, but addresses neither
  error conditions nor actions which are not connected with state
  changes.


                              +---------+ ---------\      active OPEN
                              |  CLOSED |            \    -----------
                              +---------+<---------\   \   create TCB
                                |     ^              \   \  snd SYN
                   passive OPEN |     |   CLOSE        \   \
                   ------------ |     | ----------       \   \
                    create TCB  |     | delete TCB         \   \
                                V     |                      \   \
                              +---------+            CLOSE    |    \
                              |  LISTEN |          ---------- |     |
                              +---------+          delete TCB |     |
                   rcv SYN      |     |     SEND              |     |
                  -----------   |     |    -------            |     V
 +---------+      snd SYN,ACK  /       \   snd SYN          +---------+
 |         |<-----------------           ------------------>|         |
 |   SYN   |                    rcv SYN                     |   SYN   |
 |   RCVD  |<-----------------------------------------------|   SENT  |
 |         |                    snd ACK                     |         |
 |         |------------------           -------------------|         |
 +---------+   rcv ACK of SYN  \       /  rcv SYN,ACK       +---------+
   |           --------------   |     |   -----------
   |                  x         |     |     snd ACK
   |                            V     V
   |  CLOSE                   +---------+
   | -------                  |  ESTAB  |
   | snd FIN                  +---------+
   |                   CLOSE    |     |    rcv FIN
   V                  -------   |     |    -------
 +---------+          snd FIN  /       \   snd ACK          +---------+
 |  FIN    |<-----------------           ------------------>|  CLOSE  |
 | WAIT-1  |------------------                              |   WAIT  |
 +---------+          rcv FIN  \                            +---------+
   | rcv ACK of FIN   -------   |                            CLOSE  |
   | --------------   snd ACK   |                           ------- |
   V        x                   V                           snd FIN V
 +---------+                  +---------+                   +---------+
 |FINWAIT-2|                  | CLOSING |                   | LAST-ACK|
 +---------+                  +---------+                   +---------+
   |                rcv ACK of FIN |                 rcv ACK of FIN |
   |  rcv FIN       -------------- |    Timeout=2MSL -------------- |
   |  -------              x       V    ------------        x       V
    \ snd ACK                 +---------+delete TCB         +---------+
     ------------------------>|TIME WAIT|------------------>| CLOSED  |
                              +---------+                   +---------+

                      TCP Connection State Diagram
                               Figure 6.

3.3.  Sequence Numbers

  A fundamental notion in the design is that every octet of data sent
  over a TCP connection has a sequence number.  Since every octet is
  sequenced, each of them can be acknowledged.  The acknowledgment
  mechanism employed is cumulative so that an acknowledgment of sequence
  number X indicates that all octets up to but not including X have been
  received.  This mechanism allows for straight-forward duplicate
  detection in the presence of retransmission.  Numbering of octets
  within a segment is that the first data octet immediately following
  the header is the lowest numbered, and the following octets are
  numbered consecutively.

  It is essential to remember that the actual sequence number space is
  finite, though very large.  This space ranges from 0 to 2**32 - 1.
  Since the space is finite, all arithmetic dealing with sequence
  numbers must be performed modulo 2**32.  This unsigned arithmetic
  preserves the relationship of sequence numbers as they cycle from
  2**32 - 1 to 0 again.  There are some subtleties to computer modulo
  arithmetic, so great care should be taken in programming the
  comparison of such values.  The symbol "=<" means "less than or equal"
  (modulo 2**32).

  The typical kinds of sequence number comparisons which the TCP must
  perform include:

    (a)  Determining that an acknowledgment refers to some sequence
         number sent but not yet acknowledged.

    (b)  Determining that all sequence numbers occupied by a segment
         have been acknowledged (e.g., to remove the segment from a
         retransmission queue).

    (c)  Determining that an incoming segment contains sequence numbers
         which are expected (i.e., that the segment "overlaps" the
         receive window).

  In response to sending data the TCP will receive acknowledgments.  The
  following comparisons are needed to process the acknowledgments.

    SND.UNA = oldest unacknowledged sequence number

    SND.NXT = next sequence number to be sent

    SEG.ACK = acknowledgment from the receiving TCP (next sequence
              number expected by the receiving TCP)

    SEG.SEQ = first sequence number of a segment

    SEG.LEN = the number of octets occupied by the data in the segment
              (counting SYN and FIN)

    SEG.SEQ+SEG.LEN-1 = last sequence number of a segment

  A new acknowledgment (called an "acceptable ack"), is one for which
  the inequality below holds:

    SND.UNA < SEG.ACK =< SND.NXT

  A segment on the retransmission queue is fully acknowledged if the sum
  of its sequence number and length is less or equal than the
  acknowledgment value in the incoming segment.

  When data is received the following comparisons are needed:

    RCV.NXT = next sequence number expected on an incoming segments, and
        is the left or lower edge of the receive window

    RCV.NXT+RCV.WND-1 = last sequence number expected on an incoming
        segment, and is the right or upper edge of the receive window

    SEG.SEQ = first sequence number occupied by the incoming segment

    SEG.SEQ+SEG.LEN-1 = last sequence number occupied by the incoming
        segment

  A segment is judged to occupy a portion of valid receive sequence
  space if

    RCV.NXT =< SEG.SEQ < RCV.NXT+RCV.WND

  or

    RCV.NXT =< SEG.SEQ+SEG.LEN-1 < RCV.NXT+RCV.WND

  The first part of this test checks to see if the beginning of the
  segment falls in the window, the second part of the test checks to see
  if the end of the segment falls in the window; if the segment passes
  either part of the test it contains data in the window.

  Actually, it is a little more complicated than this.  Due to zero
  windows and zero length segments, we have four cases for the
  acceptability of an incoming segment:

    Segment Receive  Test
    Length  Window
    ------- -------  -------------------------------------------

       0       0     SEG.SEQ = RCV.NXT

       0      >0     RCV.NXT =< SEG.SEQ < RCV.NXT+RCV.WND

      >0       0     not acceptable

      >0      >0     RCV.NXT =< SEG.SEQ < RCV.NXT+RCV.WND
                  or RCV.NXT =< SEG.SEQ+SEG.LEN-1 < RCV.NXT+RCV.WND

  Note that when the receive window is zero no segments should be
  acceptable except ACK segments.  Thus, it is be possible for a TCP to
  maintain a zero receive window while transmitting data and receiving
  ACKs.
  When a SYN is present then SEG.SEQ is the sequence number of the SYN.

  Initial Sequence Number Selection

  For each connection there is a send sequence number and a receive
  sequence number.  The initial send sequence number (ISS) is chosen by
  the data sending TCP, and the initial receive sequence number (IRS) is
  learned during the connection establishing procedure.

  For a connection to be established or initialized, the two TCPs must
  synchronize on each other's initial sequence numbers.  This is done in
  an exchange of connection establishing segments carrying a control bit
  called "SYN" (for synchronize) and the initial sequence numbers.  As a
  shorthand, segments carrying the SYN bit are also called "SYNs".
  Hence, the solution requires a suitable mechanism for picking an
  initial sequence number and a slightly involved handshake to exchange
  the ISN's.

  The synchronization requires each side to send it's own initial
  sequence number and to receive a confirmation of it in acknowledgment
  from the other side.  Each side must also receive the other side's
  initial sequence number and send a confirming acknowledgment.

    1) A --> B  SYN my sequence number is X
    2) A <-- B  ACK your sequence number is X
    3) A <-- B  SYN my sequence number is Y
    4) A --> B  ACK your sequence number is Y

  Because steps 2 and 3 can be combined in a single message this is
  called the three way (or three message) handshake.

  A three way handshake is necessary because sequence numbers are not
  tied to a global clock in the network, and TCPs may have different
  mechanisms for picking the ISN's.  The receiver of the first SYN has
  no way of knowing whether the segment was an old delayed one or not,
  unless it remembers the last sequence number used on the connection
  (which is not always possible), and so it must ask the sender to
  verify this SYN.  The three way handshake and the advantages of a
  clock-driven scheme are discussed in [3].

3.4.  Establishing a connection

  The "three-way handshake" is the procedure used to establish a
  connection.  This procedure normally is initiated by one TCP and
  responded to by another TCP.  The procedure also works if two TCP
  simultaneously initiate the procedure.  When simultaneous attempt
  occurs, each TCP receives a "SYN" segment which carries no
  acknowledgment after it has sent a "SYN".  Of course, the arrival of
  an old duplicate "SYN" segment can potentially make it appear, to the
  recipient, that a simultaneous connection initiation is in progress.
  Proper use of "reset" segments can disambiguate these cases.

  The simplest three-way handshake is shown in figure 7 below.  The
  figures should be interpreted in the following way.  Each line is
  numbered for reference purposes.  Right arrows (-->) indicate
  departure of a TCP segment from TCP A to TCP B, or arrival of a
  segment at B from A.  Left arrows (<--), indicate the reverse.
  Ellipsis (...) indicates a segment which is still in the network
  (delayed).  An "XXX" indicates a segment which is lost or rejected.
  Comments appear in parentheses.  TCP states represent the state AFTER
  the departure or arrival of the segment (whose contents are shown in
  the center of each line).  Segment contents are shown in abbreviated
  form, with sequence number, control flags, and ACK field.  Other
  fields such as window, addresses, lengths, and text have been left out
  in the interest of clarity.

      TCP A                                                TCP B

  1.  CLOSED                                               LISTEN

  2.  SYN-SENT    --> <SEQ=100><CTL=SYN>               --> SYN-RECEIVED

  3.  ESTABLISHED <-- <SEQ=300><ACK=101><CTL=SYN,ACK>  <-- SYN-RECEIVED

  4.  ESTABLISHED --> <SEQ=101><ACK=301><CTL=ACK>       --> ESTABLISHED

  5.  ESTABLISHED --> <SEQ=101><ACK=301><CTL=ACK><DATA> --> ESTABLISHED

          Basic 3-Way Handshake for Connection Synchronization

                                Figure 7.

  In line 2 of figure 7, TCP A begins by sending a SYN segment
  indicating that it will use sequence numbers starting with sequence
  number 100.  In line 3, TCP B sends a SYN and acknowledges the SYN it
  received from TCP A.  Note that the acknowledgment field indicates TCP
  B is now expecting to hear sequence 101, acknowledging the SYN which
  occupied sequence 100.

  At line 4, TCP A responds with an empty segment containing an ACK for
  TCP B's SYN; and in line 5, TCP A sends some data.  Note that the
  sequence number of the segment in line 5 is the same as in line 4
  because the ACK does not occupy sequence number space (if it did, we
  would wind up ACKing ACK's!).


      TCP A                                            TCP B

  1.  CLOSED                                           CLOSED

  2.  SYN-SENT     --> <SEQ=100><CTL=SYN>              ...

  3.  SYN-RECEIVED <-- <SEQ=300><CTL=SYN>              <-- SYN-SENT

  4.               ... <SEQ=100><CTL=SYN>              --> SYN-RECEIVED

  5.  SYN-RECEIVED --> <SEQ=100><ACK=301><CTL=SYN,ACK> ...

  6.  ESTABLISHED  <-- <SEQ=300><ACK=101><CTL=SYN,ACK> <-- SYN-RECEIVED

  7.               ... <SEQ=101><ACK=301><CTL=ACK>     --> ESTABLISHED

                Simultaneous Connection Synchronization
                               Figure 8.

      TCP A                                                TCP B

  1.  CLOSED                                               LISTEN

  2.  SYN-SENT    --> <SEQ=100><CTL=SYN>               ...

  3.  (duplicate) ... <SEQ=90><CTL=SYN>               --> SYN-RECEIVED

  4.  SYN-SENT    <-- <SEQ=300><ACK=91><CTL=SYN,ACK>  <-- SYN-RECEIVED

  5.  SYN-SENT    --> <SEQ=91><CTL=RST>               --> LISTEN


  6.              ... <SEQ=100><CTL=SYN>               --> SYN-RECEIVED

  7.  SYN-SENT    <-- <SEQ=400><ACK=101><CTL=SYN,ACK>  <-- SYN-RECEIVED

  8.  ESTABLISHED --> <SEQ=101><ACK=401><CTL=ACK>      --> ESTABLISHED

                    Recovery from Old Duplicate SYN
                               Figure 9.

      TCP A                                           TCP B

  1.  (CRASH)                               (send 300,receive 100)

  2.  CLOSED                                           ESTABLISHED

  3.  SYN-SENT --> <SEQ=400><CTL=SYN>              --> (??)

  4.  (!!)     <-- <SEQ=300><ACK=100><CTL=ACK>     <-- ESTABLISHED

  5.  SYN-SENT --> <SEQ=100><CTL=RST>              --> (Abort!!)

  6.  SYN-SENT                                         CLOSED

  7.  SYN-SENT --> <SEQ=400><CTL=SYN>              -->

                     Half-Open Connection Discovery
                               Figure 10.


        TCP A                                              TCP B

  1.  (CRASH)                                   (send 300,receive 100)

  2.  (??)    <-- <SEQ=300><ACK=100><DATA=10><CTL=ACK> <-- ESTABLISHED

  3.          --> <SEQ=100><CTL=RST>                   --> (ABORT!!)

           Active Side Causes Half-Open Connection Discovery
                               Figure 11.


      TCP A                                         TCP B

  1.  LISTEN                                        LISTEN

  2.       ... <SEQ=Z><CTL=SYN>                -->  SYN-RECEIVED

  3.  (??) <-- <SEQ=X><ACK=Z+1><CTL=SYN,ACK>   <--  SYN-RECEIVED

  4.       --> <SEQ=Z+1><CTL=RST>              -->  (return to LISTEN!)

  5.  LISTEN                                        LISTEN

       Old Duplicate SYN Initiates a Reset on two Passive Sockets
                               Figure 12.


      TCP A                                                TCP B

  1.  ESTABLISHED                                          ESTABLISHED

  2.  (Close)
      FIN-WAIT-1  --> <SEQ=100><ACK=300><CTL=FIN,ACK>  --> CLOSE-WAIT

  3.  FIN-WAIT-2  <-- <SEQ=300><ACK=101><CTL=ACK>      <-- CLOSE-WAIT

  4.                                                       (Close)
      TIME-WAIT   <-- <SEQ=300><ACK=101><CTL=FIN,ACK>  <-- LAST-ACK

  5.  TIME-WAIT   --> <SEQ=101><ACK=301><CTL=ACK>      --> CLOSED

  6.  (2 MSL)
      CLOSED

                         Normal Close Sequence
                               Figure 13.


      TCP A                                                TCP B

  1.  ESTABLISHED                                          ESTABLISHED

  2.  (Close)                                              (Close)
      FIN-WAIT-1  --> <SEQ=100><ACK=300><CTL=FIN,ACK>  ... FIN-WAIT-1
                  <-- <SEQ=300><ACK=100><CTL=FIN,ACK>  <--
                  ... <SEQ=100><ACK=300><CTL=FIN,ACK>  -->

  3.  CLOSING     --> <SEQ=101><ACK=301><CTL=ACK>      ... CLOSING
                  <-- <SEQ=301><ACK=101><CTL=ACK>      <--
                  ... <SEQ=101><ACK=301><CTL=ACK>      -->

  4.  TIME-WAIT                                            TIME-WAIT
      (2 MSL)                                              (2 MSL)
      CLOSED                                               CLOSED

                      Simultaneous Close Sequence
                               Figure 14.


