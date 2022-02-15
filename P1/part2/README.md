Reliable Communication
Reason about the following,

A. Performance:

Overhead of sending a message.
File - client.c (line 50-54) time taken by sendto() call 

Total round trip time of sending a message and receiving an ack
File - client.c (line 50-61) time taken before sending mssg and after receiving ack
Calculated by method timespec_diff (line 17-25)

Bandwidth achieved when sending a large number of max-sized packets between sender and receiver.
File - client.c (line 176) avg rtt is calculated in (line 164-170) 

Above three running on a single machine vs two.
We have all readings in 1 machine vs 2.

Explain what limits your bandwidth and how this can be done better.
Explained in presentation

B. Reliability:

Demonstrate your library works by controlled message drops.
Timeout set by setsockopt() and checked by err code (line 66) 
Retries are implemented in client.c (line 146-153)

The receiver code must be equipped to drop a percentage of packets randomly.
Message drops in serverbasic.c (line 22-24)

Measure round-trip time for various drop percentages while sending a stream of packets.
RTT calculation in client.c (line 154, 165, 169)

Inference:
- We observe that with the increase in server drop percentage, the probability of packet being dropped increases which increases the average Round Trip Time
- Since the probability of drop increases, there are more chances of one packet being dropped multiple times before being accepted by server
- We send 1k packets from client to server and keep on retrying untill they are acknowledged. We set the timeout to be 1sec for easy differentiation of multiple dropped packets

C. Compiler Optimization:
For the above experiments, what’s the observation you made for compiler optimization’s influence on performance? 
changes made to Makefile compilation command with optimization flags O1, O2, O3, Os, Ofast
results mentioned in presentation



