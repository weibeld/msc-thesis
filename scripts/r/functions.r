# Functions for analysing the results of the Büchi complementation experiments.
# Input files are the .out files of the experiments (to be read with function
# read("file.out")).
#
# For creating an R package, the steps are:
# 1. In R: package.skeleton(name="myPackage", path="path/to/pkg/dir")
#    Creates directory path/to/pkg/dir/myPackage
# 2. Edit myPackage/DESCRIPTION
#    Edit content of myPackage/R/ and myPackage/man/
# 3. R CMD check   myPackage (check if package is ok, optional)
# 4. R CMD install myPackage (install package in system's R library)
#    For keeping formatting: R CMD INSTALL --with-keep.source myPackage
# 5. Load package with libary(myPackage)
#
# Daniel Weibel, 04.12.2014

read <- function(file) {
  read.table(file=file, header=TRUE, sep="\t")
}

timeouts <- function(df) {
  table(df$t_out)
}
timeouts.show <- function(df) {
  df[df$t_out == "Y",]
}

memouts <- function(df) {
  table(df$m_out)
}
memouts.show <- function(df) {
  df[df$m_out == "Y",]
}

summary.states <- function(df) {
  summary(df$states, na.rm=TRUE)
}

summary.cputime <- function(df) {
  summary(df$tcpu_t)
}

summary.realtime <- function(df) {
  summary(df$real_t)
}

# Returns the data frames passed as arguments in reduced form, that is, contain-
# ing only the effective samples. What is returned is a list containing the
# reduced data frames in the same order as they were passed as arguments.
effective.samples <- function(...) {
  frames <- list(...)
  mask <- logical(nrow(frames[[1]])) # Logical vector initialised with all FALSE
  for (frame in frames)
    mask <- mask | is.na(frame$states)
  frames.reduced <- list()
  for (frame in frames)
    frames.reduced[[length(frames.reduced)+1]] <- frame[!mask,]
  frames.reduced
}

# Create histogram of number of states of complement automata of an experiment.
histogram <- function(df,xmax=10000,ymax=2000,binsize=100,
    title="Histogram",xlabel="Number of states",ylabel="Complements",
    file="~/Desktop/histogram.pdf") {
  # PDF device driver
  pdf(file)
  # Draw histogram
  binbreaks <- seq(from=0,to=100000,by=binsize)
  hist(df$states,breaks=binbreaks,xlim=c(0,xmax),ylim=c(0,ymax),main=title,
    xlab=xlabel,ylab=ylabel)
  # Add additional lines and text
  median <- median(df$states)
  perc95 <- quantile(df$states,0.95)
  textsize=0.75
  lineheight <- 1750
  lines(x=c(median,median),y=c(0,lineheight))
  text(x=median,y=lineheight+50,paste0("Median (",f(median),")"),cex=textsize)
  lineheight <- 500
  lines(x=c(perc95,perc95),y=c(0,lineheight))
  text(x=perc95,y=lineheight+50,paste0("95th percentile (",f(perc95),")"),cex=textsize)
  # Save to file
  dev.off()
}

# Create a (draft) histogram for any data vector.
histogram.generic <- function(vector,xmax,ymax,binsize,file="~/Desktop/histogram.pdf") {
  # PDF device driver
  pdf(file)
  # Draw histogram
  binbreaks <- seq(from=0,to=100000,by=binsize)
  hist(vector,breaks=binbreaks,xlim=c(0,xmax),ylim=c(0,ymax))
  # Draw median and 95th percentile line
  median <- median(vector)
  perc95 <- quantile(vector,0.95)
  textsize=0.75
  abline(v=median,col="blue")
  text(x=median,y=0,paste0("Median (",f(median),")"),cex=textsize,col="blue",pos=1,offset=0.25)
  abline(v=perc95,col="red")
  text(x=perc95,y=0,paste0("95th percentile (",f(perc95),")"),cex=textsize,col="red",pos=1,offset=0.25)
  # Save to file
  dev.off()
}

# Outputs m = (xn)^n
# m: states of output automaton
# n: states of input automaton
complexity <- function(n, m) {
  x <- m^(1/n)/n  # x = nth_root(m)/n
  x <- round(x, digits=3)
  write(paste0("(",x,"n)^n"), file="")
}

# Format a number to <dec> decimal places. Returns a character value (string).
f <- function(n,dec=1) {
  format(round(n,dec),nsmall=dec)
}
