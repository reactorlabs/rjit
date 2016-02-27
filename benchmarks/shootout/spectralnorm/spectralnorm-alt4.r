# ------------------------------------------------------------------
# The Computer Language Shootout
# http://shootout.alioth.debian.org/
#
# Contributed by Leo Osvald
# ------------------------------------------------------------------

spectralnorm_alt4 <- function(args) {
    n = if (length(args)) as.integer(args[[1]]) else 100L
    options(digits=10)

    eval_A <- function(i, j)
        return(if (eval_A_cache[[i, j]] != 0) eval_A_cache[[i, j]] else
            eval_A_cache[[i, j]] <<- 1 / ((i + j - 2) * (i + j - 1) / 2 + i))
    eval_A_times_u <- function(u) {
        #    eval_A_mat <- outer(seq(n), seq(n), FUN=eval_A)
        eval_A_mat <- matrix(0, n, n)
        for (i in 1:n)
            for (j in 1:n)
                eval_A_mat[[i, j]] <- eval_A(i, j)
        return(u %*% eval_A_mat)
    }
    eval_At_times_u <- function(u) {
        # eval_A_mat <- t(outer(seq(n), seq(n), FUN=eval_A))
        eval_A_mat <- matrix(0, n, n)
        for (i in 1:n)
            for (j in 1:n)
                eval_A_mat[[i, j]] <- eval_A(i, j)
        return(u %*% t(eval_A_mat))
    }
    eval_AtA_times_u <- function(u)
    eval_At_times_u(eval_A_times_u(u))

    eval_A_cache <- matrix(0, n, n)
    u <- rep(1, n)
    v <- rep(0, n)
    for (itr in seq(10)) {
        v <- eval_AtA_times_u(u)
        u <- eval_AtA_times_u(v)
    }

    cat(sqrt(sum(u * v) / sum(v * v)), "\n")
}

execute <- function(n = 250L) {
    spectralnorm_alt4(n)
}
execute(250L)
jit.disableOSR()
for(i in 1:10) {
    jit.startChrono()
    execute(250L)
    jit.endChrono()
}