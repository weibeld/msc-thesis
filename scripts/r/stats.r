# R functions for analyising the results of Büchi complementation experiments
# with GOAL on the test automata from the paper "State of Büchi Complementation"
# 
# Test automata available on: http://goal.im.ntu.edu.tw/wiki/lib/exe/fetch.php?
#                             media=goal:ciaa2010_automata.tar.gz
#
# Daniel Weibel <daniel.weibel@unifr.ch> Sep. 2014 - May 2015
#------------------------------------------------------------------------------#

Read <- function(file) {
  # Read a CSV file to a data frame
  #----------------------------------------------------------------------------#
  read.csv(file=file, comment.char="#")
}

OutT <- function(df) {
  # Return number of timeouts
  #----------------------------------------------------------------------------#
  nrow(df[df$t_out == "Y",])
}

OutM <- function(df) {
  # Return number of memory outs
  #----------------------------------------------------------------------------#
  nrow(df[df$m_out == "Y",])
}

Out <- function(df) {
  # Return sum of timeouts and memory outs
  #----------------------------------------------------------------------------#
  OutsTime(df) + OutsMem(df)
}

Stats1 <- function(df, label="", col="states") {
  # Statistics aggregated over all automata
  #----------------------------------------------------------------------------#
  s <- df[!is.na(df$states), col]
  data.frame(mean=mean(s),
             min=min(s),
             p25=as.numeric(quantile(s, 0.25)),
             median=median(s),
             p75=as.numeric(quantile(s, 0.75)),
             max=max(s),
             n=as.integer(length(s)),
             label=label)
}

Stats2 <- function(df, label="", col="states") {
  # Statistics for the 110 transition density/acceptance density classes
  # Calculate statistics for the 110 classes of acceptance densitiy and trans-
  # ition density combinations from the result of a single complementation run
  # over the 11.000 automata. Return a data frame with one line for each class.
  #----------------------------------------------------------------------------#
  # Transition densities (11). Don't use seq(1, 3, 0.2), or '==' will break!
  dt <- c(1.0, 1.2, 1.4, 1.6, 1.8, 2.0, 2.2, 2.4, 2.6, 2.8, 3.0)
  # Acceptance densities (10). Don't use seq(0.1, 1, 0.1), or '==' will break!
  da <- c(0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0)
  i <- 0
  for (t in dt) {
    for (a in da) {
      s <- with(df, df[dt == t & da == a & !is.na(states), col])
      row <- data.frame(mean=mean(s),
                        min=min(s),
                        p25=as.numeric(quantile(s, 0.25)),
                        median=median(s),
                        p75=as.numeric(quantile(s, 0.75)),
                        max=max(s),
                        dt=t,
                        da=a,
                        n=length(s),
                        label=label)
      if (i == 1) stats <- row               else
                  stats <- rbind(stats, row)
      i <- i + 1
    }
  }
  stats
}


EffectiveSamples <- function(...) {
  # Get effective samples of a set of data frames.
  # Args:
  #   ...: One or more data frames
  # Returns:
  #   A list with copies of the passed data frames containing only the effective
  #   samples. The order of the data frames in the list corresponds to the order
  #   in which they were passed to this function.
  #----------------------------------------------------------------------------#
  df <- list(...)
  mask <- logical(nrow(df[[1]]))  # Logical vector initialised with FALSE
  for (frame in df)
    mask <- mask | is.na(frame$states)
  df.reduced <- list()
  for (frame in df)
    df.reduced[[length(df.reduced)+1]] <- frame[!mask,]
  df.reduced
}



Complexity <- function(n, m) {
  # Write two numbers n and m in the form m = (xn)^n
  #----------------------------------------------------------------------------#
  x <- m^(1/n)/n  # x = nth_root(m)/n
  x <- round(x, digits=3)
  write(paste0("(",x,"n)^n"), file="")
}