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

ReadSubdirs <- function() {
  # Read all the CSV files in all the subdirectories of the current directory
  # into separate data frames.
  # Returns: a list of data frames, named after the input file names
  #----------------------------------------------------------------------------#
  l <- list()
  for (f in Sys.glob("*/*.csv")) {
    name  <- basename(tools::file_path_sans_ext(f))
    l[[name]] <- Read(f)
  }
  l
}

# Get number of timeouts, or display the corresponding lines
OutT  <- function(df) { nrow(OutT_(df)) }
OutT_ <- function(df) { df[df$timeout == "Y",] }

# Get number of memory outs, or display the corresponding lines
OutM  <- function(df) { nrow(OutM_(df)) }
OutM_ <- function(df) { df[df$memout == "Y",] }

# Get number of timeouts + memory outs, or display tthe corresponding lines
Out   <- function(df) { nrow(Out_(df)) }
Out_  <- function(df) { df[df$timeout == "Y" | df$memout == "Y",] }


StatsAgg <- function(df, col="states", comment="") {
  # Get statistics aggregated over all automata
  #----------------------------------------------------------------------------#
  s <- df[!is.na(df$states), col]
  data.frame(mean=mean(s),
             min=min(s),
             p25=as.numeric(quantile(s, 0.25)),
             median=median(s),
             p75=as.numeric(quantile(s, 0.75)),
             max=max(s),
             n=as.integer(length(s)),
             comments=comment)
}

Stats <- function(df, col="states", comment="") {
  # Get statistics split up by the 110 transition/acceptance density classes
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
                        comments=comment)
      if (i == 0) stats <- row               else
                  stats <- rbind(stats, row)
      i <- i + 1
    }
  }
  stats
}

StatsMatrix <- function(df, col="median") {
  # Arrange one statistics from 'Stats2' as a matrix with 11 transition density
  # rows (dt), and 10 acceptance density columns (da)
  # Args:
  #   df:  output data frame of 'Stats2'
  #   col: column name of one of the columns of 'df'
  #----------------------------------------------------------------------------#
  dt <- Float(seq(1, 3, 0.2), d=1)
  da <- Float(seq(0.1, 1, 0.1), d=1)
  # round(df[[col]], digits=1)
  matrix(df[[col]], nrow=11, ncol=10, dimnames=list(dt, da), byrow=TRUE)
}

Latex <- function(x, digits=NULL, align=NULL, file="") {
  # Transform a R data frame or matrix into a LaTeX table
  #----------------------------------------------------------------------------#
  library(xtable)
  print(xtable(x, digits=digits, align=align), file=file)
}

StatsMatrix2Latex <- function(m, digits=NULL, file="") {
  # Transform the output matrix of 'StatsMatrix' to a LaTeX table. The only
  # difference to 'Latex' is that a vertical line is added after first column.
  #----------------------------------------------------------------------------#
  Latex(m, digits=digits, align="r|rrrrrrrrrr", file=file)
}

EffSamples <- function(list, vector=FALSE) {
  # Get the effective samples of a set of data frames (i.e. the rows with non-NA
  # states in ALL data frames).
  # Args:    list:   a list of data frames
  #          vector: if TRUE, returns only logical vector indicating which rows
  #                  are effective samples
  # Returns: a list of data frames containing only the effective samples of the
  #          input data frames (or a logical vector if 'vector' is TRUE)
  #----------------------------------------------------------------------------#
  i <- 1
  for (df in list) {
    if (i == 1) v <-     !is.na(df$states) else
                v <- v & !is.na(df$states)
    i <- i + 1
  }
  if (vector) v else lapply(list, `[`, v, )
}

Contour <- function(m, grid=TRUE, ...) {
  # Draw a contour plot of a dt/da or da/dt matrix with some default settings
  # Args: m:    dt/da or da/dt matrix, column and row names are important
  #       grid: whether to add a grid (TRUE) or not (FALSE)
  #       ...:  further options for 'contour', or graphical parameters
  #----------------------------------------------------------------------------#
  z <- m                        # z values to plot
  x <- as.numeric(rownames(z))  # Row names are taken as x values
  y <- as.numeric(colnames(z))  # Column names are taken as y values
  xaxp <- c(min(x), max(x), length(x)-1)  # x-axis ticks (low, high, intervals)
  yaxp <- c(min(y), max(y), length(y)-1)  # y-axis "                          "
  xaxs <- "i"                   # Gap at ends of x-axis ("r"), or not ("i")
  yaxs <- "i"                   # "            " y-axis "                 "
  contour(x=x, y=y, z=z, xaxp=xaxp, yaxp=yaxp, xaxs=xaxs, yaxs=yaxs, ...)
  if (grid) {
    v <- x[2:(length(x)-1)]  # x positions for vertical lines
    h <- y[2:(length(y)-1)]  # y positions for horizontal lines
    abline(v=v, h=h, col="lightgray", lty="dotted")
  }
}

Persp <- function(m, theta=0, ...) {
  # Draw a persp plot of a dt/da or da/dt matrix with some default settings
  # Args: m: dt/da or da/dt matrix, column and row names are important
  #       theta: rotation (counter clock-wise)
  #       ...:   further arguments for 'persp', or graphical parameters
  # Returns: the perspective matrix 'pmat' for mapping 3D to 2D points
  #----------------------------------------------------------------------------#
  z <- m                        # z values to plot
  x <- as.numeric(rownames(z))  # Row names are taken as x values
  y <- as.numeric(colnames(z))  # Column names are taken as y values
  ticktype <- "detailed"        # Draw ticks (not just arrows)
  nticks <- 11                  # Number of ticks (approximate)
  phi <- 40                     # Elevation of the viewpoint
  theta <- theta                # Rotation (counter clock-wise)
  expand <- 0.5                 # Stretching of the z-axis
  d <- 4                        # Lessen the "perspective effect" (default is 1)
  shade <- 0.75                 # Shading of surface (higher values are darker)
  # Note: persp plots in Christian's thesis: dt/da matrix and theta=-135
  persp(x=x, y=y, z=z, ticktype=ticktype, nticks=nticks, phi=phi, theta=theta,
        expand=expand, d=d, shade=shade, ...)
}

# Format a vector of numbers as integers or floats, return them as 'character'
Int   <- function(n)      { formatC(n, format="d", big.mark=",") }
Float <- function(n, d=2) { formatC(n, format="f", big.mark=","), digits=d, }


Complexity <- function(n, m) {
  # Write two numbers n and m in the form m = (xn)^n
  #----------------------------------------------------------------------------#
  x <- m^(1/n)/n  # x = nth_root(m)/n
  x <- round(x, digits=3)
  write(paste0("(",x,"n)^n"), file="")
}

# These functions can be used to draw custom axes, if Persp has been called with
# 'axes=FALSE'. The three axes have to be drawn one after another. All functions
# take an argument 'a' whereof from the first three elements exactly one is a
# vector (tick positions), and the others are scalars:
#   a[[1]]: x-axis: tick marks (vector) or x-position of the axis with a vector
#   a[[2]]: y-axis: tick marks (vector) or y-position of the axis with a vector
#   a[[3]]: z-axis: tick marks (vector) or z-position of the axis with a vector
#   a[[4]]: Perspective matrix 'pmat' returned by Persp
#------------------------------------------------------------------------------#
PerspAxis <- function(a, ...) {
  # Add an axis to a persp plot
  # Args: ...: Arguments for lines()
  #----------------------------------------------------------------------------#
  lines(trans3d(a[[1]], a[[2]], a[[3]], a[[4]]), ...)
}

PerspTicks <- function(a, tick.len=strwidth("M"), tick.dir=c(1,0), ...) {
  # Add axis tick marks to one axis of a persp plot
  # Args: tick.len: Length of ticks
  #       tick.dir: Direction of ticks in other two dimensions
  #       ...:      Arguments for segment()
  #----------------------------------------------------------------------------#
  lines <- .PerspLines(a, tick.len, tick.dir)
  segments(lines$inner$x, lines$inner$y, lines$outer$x, lines$outer$y, ...)
}

PerspTickLabels <- function(a, tick.len=strwidth("M"), tick.dir=c(1,0), ...) {
  # Add tick mark labels to an axis of a persp plot
  # Args: tick.len: Length of ticks
  #       tick.dir: Direction of ticks in other two dimensions
  #       ...:      Arguments for text()
  #----------------------------------------------------------------------------#
  x <- a[[1]]; y <- a[[2]]; z <- a[[3]]
  if      (length(x) > 1) labels <- as.character(x)  # If treating x-axis
  else if (length(y) > 1) labels <- as.character(y)  # If treating y-axis
  else if (length(z) > 1) labels <- as.character(z)  # If treating z-axis
  lines <- .PerspLines(a, tick.len, tick.dir)
  text(lines$outer$x, lines$outer$y, labels, ...)
}

PerspAxisLabel <- function(a, label="Move me", ...) {
  # Add an axis label to a an axis of a persp plot
  # Args: label: Text of the axis label
  #       ...:   Arguments for text()
  #----------------------------------------------------------------------------#
  x <- a[[1]]; y <- a[[2]]; z <- a[[3]]; pmat <- a[[4]]
  if      (length(x) > 1) x <- (min(x) + max(x)) / 2  # If treating x-axis
  else if (length(y) > 1) y <- (min(y) + max(y)) / 2  # If treating y-axis
  else if (length(z) > 1) z <- (min(z) + max(z)) / 2  # If treating z-axis
  text(trans3d(x, y, z, pmat), labels=label, ...)
}

.PerspLines <- function(a, gap, dir) {
  # Private helper function for PerspTicks and PerspTickLabels. Compute two
  # lines parallel to an axis for spanning the ticks.
  # Args: gap: The distance between outer and inner linee
  #       dir: Vector of size 2 with values 1, 0, or -1
  # Returns: List:
  #            $inner: trans3d line identical with axis
  #            $outer: trans3d line parallel to $inner
  #----------------------------------------------------------------------------#
  x <- a[[1]]; y <- a[[2]]; z <- a[[3]]; pmat <- a[[4]]
  inner <- trans3d(x, y, z, pmat)
  # Modify parameters for outer line
  if      (length(x) > 1) {  # If treating x-axis
    y <- y + (gap * dir[1])
    z <- z + (gap * dir[2])
  }
  else if (length(y) > 1) {  # If treating y-axis
    x <- x + (gap * dir[1])
    z <- z + (gap * dir[2])
  }
  else if (length(z) > 1) {  # If treating z-axis
    x <- x + (gap * dir[1])
    y <- y + (gap * dir[2])
  }
  outer <- trans3d(x, y, z, pmat)
  list(inner=inner, outer=outer)
}
