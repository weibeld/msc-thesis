# Compare the results from Fribourg and Fribourg+R2C+C from Christian Göttel's
# experiments and my experiments. Göttel provides only the mean number of states
# for each dt/da class, thus we can just compare the means of the two experi-
# ments.
#
# Dependencies: toolbox.r from the evaluation of the main results
#
# Daniel Weibel <daniel.weibel@unifr.ch> 8 June 2015
#------------------------------------------------------------------------------#

# Generate persp plots of states:
# DoPersp(c(MatrixGoettel(), MatrixWeibel()), prefix="s.")

# Generate persp plots of times:
# DoPersp(c(MatrixGoettel("cpu_time"), MatrixWeibel("cpu_time")), prefix="t.")

# Calculate means of states:
# Mean(ReadGoettel())
# Mean(ReadWeibel())

# Calculate means of times:
# Mean(ReadGoettel(), dat="cpu_time")
# Mean(ReadWeibel(), dat="cpu_time")

ReadGoettel <- function() {
  l <- list()
  col.names <- c("dt", "da", "sets", "states", "cpu_time")
  l$efa.Fribourg <- read.table(sep="\t", file="Fribourg.txt", col.names=col.names) 
  l$`efa.Fribourg+R2C+C` <- read.table(sep="\t", file="Fribourg+R2C+C.txt", col.names=col.names)
  l
}

ReadWeibel <- function() {
  l <- Eff(ReadSubdirs())
  names(l) <- c("goal.Fribourg", "goal.Fribourg+R2C+C")
  l
}

MatrixGoettel <- function(dat="states") {
  l <- ReadGoettel()
  lapply(l, function(df) {
              m <- matrix(df[[dat]], nrow=11, ncol=10, byrow=TRUE)
              rownames(m) <- Float(Dt())
              colnames(m) <- Float(Da())
              m    
            })
}

MatrixWeibel <- function(dat="states") {
  l <- ReadWeibel()
  MatrixGoal(l, dat=dat, stat="mean")
}

DoPersp <- function(list.m, ...) {
  PerspBatch(list.m, pdf=TRUE, theta=150, phi=25, shade=0.5, ltheta=30, lphi=20,
  ...)
}

Mean <- function(list.df, dat="states") {
  sapply(list.df, function(df) { mean(df[[dat]]) })
}
