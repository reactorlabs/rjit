# ------------------------------------------------------------------
# The Computer Language Shootout
# http://shootout.alioth.debian.org/
#
# Contributed by Leo Osvald
# ------------------------------------------------------------------

lim <- 2
iter <- 50

mandelbrot_naive <- function(args) {
    n = if (length(args)) as.integer(args[[1]]) else 200L
    cat("P4\n")
    cat(n, n, "\n")
    bin_con <- pipe("cat", "wb")
    for (y in 0:(n-1)) {
        bits <- 0L
        x <- 0L
        while (x < n) {
            c <- 2 * x / n - 1.5 + 1i * (2 * y / n - 1)
            z <- 0+0i
            i <- 0L
            while (i < iter && abs(z) <= lim) {
                z <- z * z + c
                i <- i + 1L
            }
            bits <- 2L * bits + as.integer(abs(z) <= lim)
            if ((x <- x + 1L) %% 8L == 0) {
                writeBin(as.raw(bits), bin_con)
                bits <- 0L
            }
        }
        xmod <- x %% 8L
        if (xmod)
            writeBin(as.raw(bits * as.integer(2^(8L - xmod))), bin_con)
        flush(bin_con)
    }
}

execute <- function(n = 400L) {
    mandelbrot_naive(n)
}

jit.startChrono()
execute(400L)
jit.endChrono()
