require("rjit")

############ Single brackets ############

f <- jit.compile(function() {
	a <- matrix(c(2:5),2,2)
	a[2,2]
})
stopifnot(5 == f())

f <- jit.compile(function() {
	a <- matrix(c(2:5),2,2)
	a[1,1]
})
stopifnot(2 == f())

f <- jit.compile(function() {
	a <- matrix(c(2:5),2,2)
	a[1+1,1+1]
})
stopifnot(5 == f())

f <- jit.compile(function() {
	a <- matrix(c(2:5),2,2)
	a[,]
})
stopifnot(matrix(c(2:5),2,2) == f())

f <- jit.compile(function() {
	a <- matrix(c(2:5),2,2)
	b <- a[1,1]
	b
})
stopifnot(2 == f())

f <- jit.compile(function() {
	a <- matrix(c(2:5),2,2)
	a[TRUE,TRUE]
})
stopifnot(matrix(c(2:5),2,2) == f())

f <- jit.compile(function() {
	a <- matrix(c(2:5),2,2)
	g <- function(x){x}
	g(a)[g(1),g(1)]
})
stopifnot(2 == f())

f <- jit.compile(function() {
	a <- matrix(c(2:5),2,2)
	a[NULL,NULL]
})
stopifnot(matrix(0,0,0) == f())

f <- jit.compile(function() {
	a <- matrix(c(2:5),2,2)
	g <- function(){a[1,1] <- 10}
	g()
	a
})
stopifnot(matrix(c(2:5),2,2) == f())

f <- jit.compile(function() {
	a <- matrix(c(2:5),2,2)
	g <- function(a){a[1,1] <- 10}
	g(a)
	a
})
stopifnot(matrix(c(2:5),2,2) == f())

############ Double brackets ############

f <- jit.compile(function() {
	a <- matrix(c(2:5),2,2)
	a[[2,2]]
})
stopifnot(5 == f())

f <- jit.compile(function() {
	a <- matrix(c(2:5),2,2)
	a[[1,1]]
})
stopifnot(2 == f())

f <- jit.compile(function() {
	a <- matrix(c(2:5),2,2)
	a[[1+1,1+1]]
})
stopifnot(5 == f())

# Gives an error
f <- jit.compile(function() {
	a <- matrix(c(2:5),2,2)
	a[[,]]
})
stopifnot("invalid subscript type 'symbol'" == tryCatch(f(), error = function(e) e$message))

# Gives an error
f <- jit.compile(function() {
	a <- matrix(c(2:5),2,2)
	a[[NULL,NULL]]
})
stopifnot("attempt to select less than one element" == tryCatch(f(), error = function(e) e$message))


f <- jit.compile(function() {
	a <- matrix(c(2:5),2,2)
	b <- a[[1,1]]
	b
})
stopifnot(2 == f())

f <- jit.compile(function() {
	a <- matrix(c(2:5),2,2)
	a[[TRUE,TRUE]]
})
f()
stopifnot(2 == f())

f <- jit.compile(function() {
	a <- matrix(c(2:5),2,2)
	g <- function(x){x}
	g(a)[[g(1),g(1)]]
})
stopifnot(2 == f())

f <- jit.compile(function() {
	a <- matrix(c(2:5),2,2)
	g <- function(){a[[1,1]] <-10}
	g()
	a
})
stopifnot(matrix(c(2:5),2,2) == f())

f <- jit.compile(function() {
	a <- matrix(c(2:5),2,2)
	g <- function(a){a[[1,1]] <- 10}
	g(a)
	a
})
stopifnot(matrix(c(2:5),2,2) == f())

######################### ASSIGNMENT #########################

############ Single brackets ############

f <- jit.compile(function() {
	a <- matrix(c(2:5),2,2)
	g <- function(){a[1,1] <-10}
	g()
	a
})
stopifnot(matrix(c(2:5),2,2) == f())


f <- jit.compile(function() {
	a <- matrix(c(2:5),2,2)
	a[2,2] <- 10
	a
})
stopifnot(c(2,3,4,10) == f())

f <- jit.compile(function() {
	a <- matrix(c(2:5),2,2)
	a[1,1] <- 20
	a
})
stopifnot(c(20,3,4,5) == f())

f <- jit.compile(function() {
	a <- matrix(c(2:5),2,2)
	a[1+1,1+1]<- 2+2+2+2+2
	a
})
stopifnot(c(2,3,4,10) == f())

f <- jit.compile(function(){
	x <- matrix(c(1:4),2,2)
	y <- x
	for(i in 1:2){
		x[i,1] <- x[i+1]
	}
	x
})
f()
stopifnot(c(2,3,3,4) == f())

f <- jit.compile(function(){
	x <- matrix(c(1:4),2,2)
	y <- x
	for(i in 1:2){
		x[i,1] <- x[i+1]
	}
	y
})
stopifnot(matrix(c(1:4),2,2) == f())

f <- jit.compile(function() {
	a <- matrix(c(2:5),2,2)
	a[TRUE,TRUE] <- 10
	a
})
stopifnot(c(10,10,10,10) == f())

f <- jit.compile(function() {
	a <- matrix(c(2:5),2,2)
	g <- function(x){x}
	a[g(1),g(1)] <- 10
	a
})
stopifnot(c(10,3,4,5)== f())

f <- jit.compile(function() {
	a <- matrix(c(2:5),2,2)
	a[NULL,NULL] <- 10
	a
})
stopifnot(matrix(c(2:5),2,2) == f())

f <- jit.compile(function() {
	a <- matrix(c(2:5),2,2)
	g <- function(){a[1,1] <- 10}
	g()
	a
})
stopifnot(matrix(c(2:5),2,2) == f())

f <- jit.compile(function() {
	a <- matrix(c(2:5),2,2)
	g <- function(a){a[1,1] <- 10}
	g(a)
	a
})
stopifnot(matrix(c(2:5),2,2) == f())

############ Double brackets ############

f <- jit.compile(function() {
	a <- matrix(c(2:5),2,2)
	a[2,2] <- 10
	a
})
stopifnot(c(2,3,4,10) == f())

f <- jit.compile(function() {
	a <- matrix(c(2:5),2,2)
	a[[1,1]] <- 20
	a
})
stopifnot(c(20,3,4,5) == f())

f <- jit.compile(function() {
	a <- matrix(c(2:5),2,2)
	a[[1+1,1+1]]<- 2+2+2+2+2
	a
})
stopifnot(c(2,3,4,10) == f())

f <- jit.compile(function(){
	x <- matrix(c(1:4),2,2)
	y <- x
	for(i in 1:2){
		x[[i,1]] <- x[[i+1]]
	}
	x
})
f()
stopifnot(c(2,3,3,4) == f())

f <- jit.compile(function(){
	x <- matrix(c(1:4),2,2)
	y <- x
	for(i in 1:2){
		x[[i,1]] <- x[[i+1]]
	}
	y
})
stopifnot(matrix(c(1:4),2,2) == f())

f <- jit.compile(function() {
	a <- matrix(c(2:5),2,2)
	a[[TRUE,TRUE]] <- 10
	a
})
stopifnot(c(10,3,4,5) == f())

f <- jit.compile(function() {
	a <- matrix(c(2:5),2,2)
	g <- function(x){x}
	a[[g(1),g(1)]] <- 10
	a
})
stopifnot(c(10,3,4,5)== f())

f <- jit.compile(function() {
	a <- matrix(c(2:5),2,2)
	a[[NULL,NULL]] <- 10
	a
})
stopifnot("attempt to select less than one element" == tryCatch(f(), error = function(e) e$message))

# Gives an error
f <- jit.compile(function() {
	a <- matrix(c(2:5),2,2)
	a[[,]] <- 10
})
stopifnot("[[ ]] with missing subscript" == tryCatch(f(), error = function(e) e$message))

f <- jit.compile(function() {
	a <- matrix(c(2:5),2,2)
	g <- function(){a[[1,1]] <- 10}
	g()
	a
})
stopifnot(matrix(c(2:5),2,2) == f())

f <- jit.compile(function() {
	a <- matrix(c(2:5),2,2)
	g <- function(a){a[[1,1]] <- 10}
	g(a)
	a
})
stopifnot(matrix(c(2:5),2,2) == f())

######################### SUPER ASSIGNMENT #########################

############ Single brackets ############

f <- jit.compile(function(){
	x <- matrix(c(1:6),2,3)
	x[1,1] <<- 5
	x
})
stopifnot("object 'x' not found" == tryCatch(f(), error = function(e) e$message))


f <- jit.compile(function(){
	x <- matrix(c(1:6),2,3)
	y <- x
	g <- function(){
		y[1,1] <<- 5
	}
	g()
	y
})
stopifnot(c(5,2,3,4,5,6) == f())

f <- jit.compile(function(){
	x <- matrix(c(1:6),2,3)
	y <- x
	g <- function(){
		x[1,1] <<- 5
	}
	g()
	y
})
stopifnot(matrix(c(1:6),2,3) == f())


f <- jit.compile(function(){
	x <- matrix(c(1:6),2,3)
	y <- x
	h <- function(){
		g <- function(){
			y[1,1] <<- 5
		}
		g()
	}
	h()
	y
})
stopifnot(c(5,2,3,4,5,6) == f())

f <- jit.compile(function(){
	x <- matrix(c(1:6),2,3)
	y <- x
	h <- function(){
		g <- function(){
			x[1,1] <<- 5
		}
		g()
	}
	h()
	y
})
stopifnot(matrix(c(1:6),2,3) == f())


f <- jit.compile(function(){
	x <- matrix(c(1:6),2,3)
	y <- x
	h <- function(){
		g <- function(){
			y[1,1] <<- 5
		}
		g()
	}
	h()
	x
})
stopifnot(matrix(c(1:6),2,3) == f())


f <- jit.compile(function(){
	x <- matrix(c(1:6),2,3)
	y <- x
	g <- function(){
		y[1,1] <<- 5
	}
	g()
	x
})
stopifnot(matrix(c(1:6),2,3) == f())


f <- jit.compile(function(){
	x <- matrix(c(1:6),2,3)
	y <- x
	h <- function(){
		g <- function(){
			x[1,1] <<- 5
		}
		g()
	}
	h()
	x
})
stopifnot(c(5,2,3,4,5,6) == f())

f <- jit.compile(function(){
	x <- matrix(c(1:6),2,3)
	x[TRUE, TRUE] <<- 5
	x
})
stopifnot("object 'x' not found" == tryCatch(f(), error = function(e) e$message))

f <- jit.compile(function(){
	x <- matrix(c(1:6),2,3)
	x[NULL, NULL] <<- 5
	x
})
stopifnot("object 'x' not found" == tryCatch(f(), error = function(e) e$message))

f <- jit.compile(function(){
	x <- matrix(c(1:6),2,3)
	x[] <<- 5
	x
})
stopifnot("object 'x' not found" == tryCatch(f(), error = function(e) e$message))

f <- jit.compile(function(){
	x <- c(1:5)
	g <- function(){
		x[,] <<- 5
	}
	g()
	x
})
# This is wrong!!!!!!!!!!!!!
stopifnot("incorrect number of subscripts on matrix" == tryCatch(f(), error = function(e) e$message))
# The correct behaviour should be:
# stopifnot(c(5,5,5,5,5) == f())

############ Double brackets ############

f <- jit.compile(function(){
	x <- matrix(c(1:6),2,3)
	x[[1,1]] <<- 5
	x
})
stopifnot("object 'x' not found" == tryCatch(f(), error = function(e) e$message))


f <- jit.compile(function(){
	x <- matrix(c(1:6),2,3)
	y <- x
	g <- function(){
		y[[1,1]] <<- 5
	}
	g()
	y
})
stopifnot(c(5,2,3,4,5,6) == f())

f <- jit.compile(function(){
	x <- matrix(c(1:6),2,3)
	y <- x
	g <- function(){
		x[[1,1]] <<- 5
	}
	g()
	y
})
stopifnot(matrix(c(1:6),2,3) == f())


f <- jit.compile(function(){
	x <- matrix(c(1:6),2,3)
	y <- x
	h <- function(){
		g <- function(){
			y[[1,1]] <<- 5
		}
		g()
	}
	h()
	y
})
stopifnot(c(5,2,3,4,5,6) == f())

f <- jit.compile(function(){
	x <- matrix(c(1:6),2,3)
	y <- x
	h <- function(){
		g <- function(){
			x[[1,1]] <<- 5
		}
		g()
	}
	h()
	y
})
stopifnot(matrix(c(1:6),2,3) == f())


f <- jit.compile(function(){
	x <- matrix(c(1:6),2,3)
	y <- x
	h <- function(){
		g <- function(){
			y[[1,1]] <<- 5
		}
		g()
	}
	h()
	x
})
stopifnot(matrix(c(1:6),2,3) == f())


f <- jit.compile(function(){
	x <- matrix(c(1:6),2,3)
	y <- x
	g <- function(){
		y[[1,1]] <<- 5
	}
	g()
	x
})
stopifnot(matrix(c(1:6),2,3) == f())


f <- jit.compile(function(){
	x <- matrix(c(1:6),2,3)
	y <- x
	h <- function(){
		g <- function(){
			x[[1,1]] <<- 5
		}
		g()
	}
	h()
	x
})
stopifnot(c(5,2,3,4,5,6) == f())

f <- jit.compile(function(){
	x <- matrix(c(1:6),2,3)
	x[[TRUE, TRUE]] <<- 5
	x
})
stopifnot("object 'x' not found" == tryCatch(f(), error = function(e) e$message))

f <- jit.compile(function(){
	x <- matrix(c(1:6),2,3)
	x[[NULL, NULL]] <<- 5
	x
})
stopifnot("object 'x' not found" == tryCatch(f(), error = function(e) e$message))

f <- jit.compile(function(){
	x <- matrix(c(1:6),2,3)
	x[[]] <<- 5
	x
})
stopifnot("object 'x' not found" == tryCatch(f(), error = function(e) e$message))

f <- jit.compile(function(){
	x <- c(1:5)
	g <- function(){
		x[[,]] <<- 5
	}
	g()
	x
})
# This is wrong!!!!!!!!!!!!!
stopifnot("[[ ]] with missing subscript" == tryCatch(f(), error = function(e) e$message))