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

# Get number of timeouts, or display them
OutT  <- function(df) { nrow(OutT_(df)) }
OutT_ <- function(df) { df[df$t_out == "Y",] }

# Get number of memory outs, or display them
OutM  <- function(df) { nrow(OutM_(df)) }
OutM_ <- function(df) { df[df$m_out == "Y",] }

# Get total number of timeouts + memory outs, or display them
Out   <- function(df) { nrow(Out_(df)) }
Out_  <- function(df) { df[df$t_out == "Y" | df$m_out == "Y",] }


Stats1 <- function(df, c="", col="states") {
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
             comments=c)
}

Stats2 <- function(df, c="", col="states") {
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
                        comments=c)
      if (i == 1) stats <- row               else
                  stats <- rbind(stats, row)
      i <- i + 1
    }
  }
  stats
}

EffSamples <- function(...) {
  # Determine the effective samples of a set of data frames. The effecive
  # samples are the complementation tasks (rows) that are successful in ALL of
  # the passed data frames (i.e. 'states' is not NA).
  # Args:    ...: One or more data frames
  # Returns: A logical vector with TRUE for each row that is an effective
  #          sample, and FALSE for the others.
  #----------------------------------------------------------------------------#
  dfs <- list(...)
  i <- 1
  for (df in dfs) {
    if (i == 1) v <-     !is.na(df$states) else
                v <- v & !is.na(df$states)
    i <- i + 1
  }
  v
}

Complexity <- function(n, m) {
  # Write two numbers n and m in the form m = (xn)^n
  #----------------------------------------------------------------------------#
  x <- m^(1/n)/n  # x = nth_root(m)/n
  x <- round(x, digits=3)
  write(paste0("(",x,"n)^n"), file="")
}
