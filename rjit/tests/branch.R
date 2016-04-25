require("rjit")

f <- jit.compile(function(){
  if (1) 3
})
stopifnot(f() == 3)
stopifnot(f() == 3)


f <- jit.compile(function(){
  if (2L) 3
})
stopifnot(f() == 3)
stopifnot(f() == 3)

f <- jit.compile(function(){
  if (!1) 0 else 3
})
stopifnot(f() == 3)
stopifnot(f() == 3)


f <- jit.compile(function(){
  if (!2L) 1 else 3
})
stopifnot(f() == 3)
stopifnot(f() == 3)


f <- jit.compile(function(){
    lf <- length(1:2)
    if(lf) 1L:lf
})
stopifnot(length(f()) == 2)

f <- jit.compile(function(){
  TRUE && TRUE
})
stopifnot(f() == TRUE)

f <- jit.compile(function(){
  FALSE && TRUE
})
stopifnot(f() == FALSE)

f <- jit.compile(function(){
  TRUE && FALSE
})
stopifnot(f() == FALSE)

f <- jit.compile(function(){
  FALSE && FALSE
})
stopifnot(f() == FALSE)

f <- jit.compile(function(){
  x <- 1
  FALSE && (x <- 10)
  x
})
stopifnot(f() == 1)

f <- jit.compile(function(){
  TRUE|| TRUE
})
stopifnot(f() == TRUE)

f <- jit.compile(function(){
  FALSE|| TRUE
})
stopifnot(f() == TRUE)

f <- jit.compile(function(){
  TRUE || FALSE
})
stopifnot(f() == TRUE)

f <- jit.compile(function(){
  FALSE || FALSE
})
stopifnot(f() == FALSE)

f <- jit.compile(function(){
  x <- 1
  TRUE || (x <- 10)
  x
})
stopifnot(f() == 1)

f <- jit.compile(function(){
  if (TRUE && TRUE) 3
})
stopifnot(f() == 3)

f <- jit.compile(function(){
  if (FALSE && TRUE) 3 else 1
})
stopifnot(f() == 1)

f <- jit.compile(function(){
  if (FALSE && FALSE) 3 else 1
})
stopifnot(f() == 1)

f <- jit.compile(function(){
  if (TRUE && FALSE) 3 else 1
})
stopifnot(f() == 1)

f <- jit.compile(function(){
  if (FALSE && FALSE) 3 else 1
})
stopifnot(f() == 1)

f <- jit.compile(function(){
  TRUE || NA
})
stopifnot(f() == TRUE)

f <- jit.compile(function(){
  if (TRUE || NA) 3 
})
stopifnot(f() == 3)

f <- jit.compile(function(){
  if (FALSE || FALSE) 3 else 1
})
stopifnot(f() == 1)

seq.default <- jit.compile(seq.default)
stopifnot(length(seq(1:2)) == 2)
stopifnot(length(seq(1:2)) == 2)
stopifnot(length(seq(11:13)) == 3)
stopifnot(length(seq(seq(1:2))) == 2)

