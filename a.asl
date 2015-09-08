count(0).

!wait.

+!wait : not called & count(X) <-
    NextX = X + 1;
    -+count(NextX);
    !print(NextX);
    !wait.

+!wait : called <-
    !print("A was called.").

+!print(X) <-
    .print("\x1B[47;32m", X, "\x1B[m").
