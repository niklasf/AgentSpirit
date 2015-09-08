sending_in(20).

!start.

+!start : sending_in(X) & X > 0 <-
    -sending_in(X);
    +sending_in(X - 1);
    .print("B: ", X - 1);
    !start.

+!start : sending_in(X) & X <= 0 <-
    .mpi_send_belief("a.asl1", called);
    .print("B: Sent!").
