f <- jit.compile(function(){
  1:4
})
stopifnot(f() == 1:4)

f <- jit.compile(function(){
  4:1 
})
stopifnot(f() == 4:1)

f <- jit.compile(function(){
  1 : 2 - 1
})
stopifnot(f() == 0:1)

f <- jit.compile(function(){
  4 + 1 : 2 
})
stopifnot(f() == 5:6)

f <- jit.compile(function(){
  4 + 1 : 2 - 1
})
stopifnot(f() == 4:5)

f <- jit.compile(function(){
  (4 + 1) : (2 - 1)
})
stopifnot(f() == 5:1)
