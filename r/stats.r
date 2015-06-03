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

ReadSubdirs <- function(dir=".") {
  # Read all the CSV files in all the subdirectories of the current directory
  # into separate data frames.
  # Returns: a list of data frames, with elements named after the input files
  #----------------------------------------------------------------------------#
  l <- list()
  for (f in Sys.glob(paste0(dir, "/*/*.csv"))) {
    name  <- FormatName(basename(tools::file_path_sans_ext(f)))
    l[[name]] <- Read(f)
  }
  l
}

Outs <- function(list, labels=names(list)) {
  # Get number of timeouts and memory outs for each of a list of data frames
  # Args: list: a list of data frames
  #       labels: character vector with a label for each data frame in 'list'
  # Returns: a data frame with one row for each data frame in 'list'
  #----------------------------------------------------------------------------#
  i <- 1
  for (df in list) {
    row <- data.frame(Construction = labels[i],
                      Time         = nrow(df[df$timeout == "Y",]),
                      Memory       = nrow(df[df$memout  == "Y",]))
    if (i == 1) res <- row else res <- rbind(res, row)
    i <- i + 1
  }
  res
}

Eff <- function(list, vector=FALSE) {
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

EffNb <- function(list, invert=FALSE) {
  # Get the number of effective samples from a list of data frames
  #----------------------------------------------------------------------------#
  n <- nrow(Eff(list)[[1]])
  if (invert) nrow(list[[1]]) - n else n
}

Stats <- function(list, col="states", labels=names(list)) {
  # Get statistics aggregated over entire data frame for a list of data frames
  # Args: list:   a list of data frames
  #       col:    name of the column for which to calculate the statistics
  #       labels: character vector with a label for each data frame in 'list'
  # Returns: a data frame with the stats with one row for each input data frame
  #----------------------------------------------------------------------------#
  i <- 1
  for (df in list) {
    if (any(is.na(df$states))) stop("data frame has NA in column 'states'")
    x <- df[[col]]
    row <- data.frame(Construction = labels[i],
                      Mean         = mean(x),
                      Min.         = min(x),
                      P25          = quantile(x, 0.25, names=FALSE),
                      Median       = median(x),
                      P75          = quantile(x, 0.75, names=FALSE),
                      Max.         = max(x))
    if (i == 1) res <- row else res <- rbind(res, row)
    i <- i + 1
  }
  res
}

StatsGoal <- function(list, col="states") {
  # Get statistics split up by the 110 transition/acceptance density classes
  # Args: list: named list of GOAL result data frames with no NA states
  #       col:  name of the column for which to calculate the statistics
  # Returns: list of data frames with one row for each dt/da class
  #----------------------------------------------------------------------------#
  res.list <- list()
  i <- 1
  for (df in list) {
    if (any(is.na(df$states))) stop("data frame has NA in column 'states'")
    j <- 1
    for (t in Dt()) {
      for (a in Da()) {
        x <- df[df$dt == t & df$da == a, col]
        if (length(x) > 0)
          row <- data.frame(mean   = mean(x),
                            min    = min(x),
                            p25    = quantile(x, 0.25, names=FALSE),
                            median = median(x),
                            p75    = quantile(x, 0.75, names=FALSE),
                            max    = max(x),
                            dt     = t,
                            da     = a,
                            n      = length(x))
        else  # If there are no effective samples in a dt/da class
          row <- data.frame(mean   = NA,
                            min    = NA,
                            p25    = NA,
                            median = NA,
                            p75    = NA,
                            max    = NA,
                            dt     = t,
                            da     = a,
                            n      = 0)
        if (j == 1) res.df <- row else res.df <- rbind(res.df, row)
        j <- j + 1
      }
    }
    res.list[[names(list)[i]]] <- res.df
    i <- i + 1
  }
  res.list
}

Michel <- function(list, col="states", labels=names(list)) {
  # Combine results for Michel automata in a single data frame
  # Args: list:   named list of Michel result data frames
  #       col:    name of the column to display in the result
  #       labels: character vector with a label for each data frame in 'list'
  # Returns: a data frame with one row for each data frame in 'list'
  #----------------------------------------------------------------------------#
  i <- 1
  for (df in list) {
    row <- data.frame(Construction = labels[i],
                      `Michel 1`   = df[1, col],
                      `Michel 2`   = df[2, col],
                      `Michel 3`   = df[3, col],
                      `Michel 4`   = df[4, col], check.names=FALSE)
    if (i == 1) res <- row else res <- rbind(res, row)
    i <- i + 1
  }
  res
}

MatrixGoal <- function(list, col="states", stat="median") {
  # Get a single statistics as dt/da matrices
  # Args: list: named list of GOAL result data frames with no NA states
  #       col:  column for which to get the statistics
  #       stat: statistics (column name of result of 'StatsGoal')
  # Returns: list of matrices with elements named after the input data frames
  #----------------------------------------------------------------------------#
  res <- list()
  s <- StatsGoal(list, col=col)
  i <- 1
  for (df in s) {
    res[[names(list)[i]]] <- matrix(df[[stat]], nrow=11, ncol=10, byrow=TRUE,
                             dimnames=list(dt=Float(Dt()), da=Float(Da())))
    i <- i + 1
  }
  res
}

MatrixTestset <- function(df) {
  # Get number of complete or universal automata for each dt/da class
  # Arg: df:  data frame with results from the completeness or universal test
  # Returns: matrix with number of complete or universal automata
  #----------------------------------------------------------------------------#
  if ("complete"  %in% colnames(df)) col <- "complete"  else
  if ("universal" %in% colnames(df)) col <- "universal" else
  stop("could not find column 'complete' or 'universal' in data frame")
  nb.yes <- integer()
  for (t in Dt()) {
    for (a in Da()) {
      nb.yes <- c(nb.yes, nrow(df[df$dt == t & df$da == a & df[[col]] == "Y",]))
    }
  }
  # Fill a matrix with number of "Y" rows for each dt/da class
  matrix(nb.yes, nrow=11, ncol=10, byrow=TRUE,
         dimnames=list(dt=Float(Dt()), da=Float(Da())))
}

LatexTable <- function(x, format="f", digits=1, align=NULL, ...) {
  # Transform a data frame or matrix to a LaTeX table
  # Args: x:      data frame or matrix
  #       format: character vector with "f", "d", or "" for each column of a
  #               data frame, or just a single "f", "d", or ""
  #       digits: number of digits for numbers falling under "f" formatting
  #       align:  LaTeX column specification with one extra column for row names
  #       ...:    further arguments for 'print.xtable'
  # Returns: writes LaTeX code to the output device
  #----------------------------------------------------------------------------#
  # If 'align' is not specified, set it explicitely to not leave it to 'xtable'
  if (is.null(align)) align.auto <- "r"  # Extra column for rownames

  # Formatting data frame
  if (is.data.frame(x)) {
    if (length(format) == 1) format <- rep_len(format, ncol(x))
    i <- 1
    for (col in x) {
      if (class(col) == "numeric" | class(col) == "integer") {
        # Format 'numeric' and 'integer' columns, result is 'character'
        if (format[i] == "f") x[[i]] <- Float(col, d=digits) else
        if (format[i] == "d") x[[i]] <- Int(col)
        # Even though the column is now 'character', we want it right-aligned
        if (is.null(align)) align.auto <- paste0(align.auto, "r")
      }
      # If the column is 'character' or 'factor'
      else {
        if (is.null(align)) align.auto <- paste0(align.auto, "l")
      }
      i <- i + 1
    }
    
  } else

  # Formatting matrix
  if (is.matrix(x)) {
    if (format[1] == "f") x <- Float(x, d=digits) else
    if (format[1] == "d") x <- Int(x)
    # All values in a matrix right-aligned
    if (is.null(align))
      for (i in seq(1, ncol(x))) align.auto <- paste0(align.auto, "r")
  }
  else stop("argument 'x' must be data frame or matrix, found ", class(x))

  if (is.null(align)) align <- align.auto

  # Create the LaTeX code
  library(xtable)
  print(xtable(x, align=align), ...)
}

Stripchart <- function(list, col="states", ...) {
  #
  # Args: list: named list of data frames
  #       col:  column to plot
  #----------------------------------------------------------------------------#
  data <- lapply(list, `[[`, col)  # Named list of vectors
  data <- rev(data)      # Reverse list to obtain top-down order of strips
  las <- 1               # All axis tick labes horizontal
  pch <- 1               # Circles as plotting symbols
  MarL(10)               # Increase left margin
  stripchart(data, method="jitter", jitter=0.25, las=las, pch=pch, xaxt="n", ...)
  # Axis with formatted tick marks
  axis(1, at=axTicks(1), labels=Int(axTicks(1)))
}

# Contour is less informative than persp, so it could be completely omitted
ContourBatch <- function(list, pdf=TRUE, dir=".", width=7, height=7, mar=par("mar"),
                         zlim=c(0, max(sapply(list, max))), ...) {
  pdf.files <- paste0(dir, "/contour_", names(list), ".pdf")  # Names of output files
  i <- 1
  for (m in list) {
    if (pdf) pdf(file=pdf.files[i], width=width, height=height)
    else     quartz(width=width, height=height)
    par(mar=mar)  # Set margins
    Contour(m, zlim=zlim, ...)
    if (pdf) dev.off()
    i <- i + 1
  }
}

Contour <- function(m, grid=TRUE, ...) {
  # Draw a contour plot of a dt/da or da/dt matrix with some default settings
  # Args: m:    dt/da or da/dt matrix, col and row names are taken as x/y values
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

PerspBatch <- function(list, pdf=TRUE, dir=".", width=7, height=7, mar=par("mar"),
                       zlim=c(0, max(sapply(list, max))), z.colors=TRUE,
                       custom.zlab=NULL, ...) {
  # Draw a batch of persp plots with identical parameters
  # Args: list: named list of dt/da or da/dt matrices (rows/columns)
  #       pdf:         use 'pdf' driver (TRUE) or 'quartz' (FALSE)
  #       dir:         directory where to save the PDF files (if pdf=TRUE)
  #       width, height: width and height of the output graphics
  #       mar:         margins of the output graphics
  #       zlim:        common zlim for all the plots
  #       z.colors:    if TRUE, use a z-dependent colour scheme
  #       custom.zlab: a function without arguments placing text on the plot
  #       ...:         arguments to 'Persp'
  #----------------------------------------------------------------------------#
  max <- max(sapply(list, max))  # Greatest element of all the matrices
  pdf.files <- paste0(dir, "/", names(list), ".pdf")  # Names of output files
  i <- 1
  for (m in list) {
    if (pdf) pdf(file=pdf.files[i], width=width, height=height)
    else     quartz(width=width, height=height)
    par(mar=mar)  # Set margins
    if (z.colors) Persp(m, zlim=zlim, col=FacetColors(m, max), ...)
    else          Persp(m, zlim=zlim, ...)
    # Execute user-provided function for adding text (or anything else) to plot
    if (!is.null(custom.zlab)) {
      par(xpd=TRUE)   # Allow plotting outside of plotting area
      custom.zlab()   # Try: function() { text(-0.68, 0.05, "States", srt=95) }
    }
    if (pdf) dev.off()
    i <- i + 1
  }
}

Persp <- function(m, zlim=range(m, na.rm=TRUE), lin=TRUE, lin.xy=NULL,
                  lin.lty=par("lty"), lin.lwd=par("lwd"), lin.col=par("col"), ...) {
  # Draw a persp plot of a dt/da or da/dt matrix (rows/columns)
  # Args: m:     dt/da or da/dt matrix, col and row names are taken as x/y vals
  #       zlim:  zlim (it's an explicit argument just to make sure it is set)
  #       lin:   if TRUE, add horizontal helper lines at the height of z-ticks
  #              on two of the xz and yz planes
  #       lin.xy: list with named elements 'x' and 'y' specifying x-pos of lines
  #               on yz plane and y-pos of lines on xz plane, respectively
  #       lin.lty, lin.lwd, lin.col: type, width and colour for horizontal lines
  #       ...:   arguments for 'persp'
  # Returns: the perspective matrix 'pmat' for mapping 3D to 2D points
  # Note: persp plots in Christian's thesis: dt/da matrix and theta=225
  #       Other nice values: dt/da, theta=150, ltheta=30, lphi=20, shade=0.5
  #----------------------------------------------------------------------------#
  # Data
  x <- as.numeric(rownames(m))  # Row names are taken as x values
  y <- as.numeric(colnames(m))  # Column names are taken as y values

  # persp options: mandatory
  ticktype <- "detailed"        # Draw ticks on axes (not just arrows)
  nticks   <- 10                # Number of ticks on each axis (approximate)
  # persp options: sensible defaults
  expand   <- 0.75              # Stretching of the z-axis
  d        <- 4                 # Lessen the "perspective effect" (default is 1)

  # Helper lines: (1) create empty plot; (2) draw lines; (3) overlay real plot
  if (lin) {
    # Defaults if one or both of lin.xy$x and lin.xy$y are not set
    if (is.null(lin.xy$x)) lin.xy$x <- min(x)  # x-pos of lines on yz-plane
    if (is.null(lin.xy$y)) lin.xy$y <- min(y)  # y-pos of lines on xz-plane
    # Calculate the same z-ticks that 'persp' will later use (probably)
    z.ticks <- pretty(zlim, nticks)
    # Dummy matrix filled with NA to create empty plot (but desired perspective)
    m.na <- matrix(nrow=nrow(m), ncol=ncol(m))
    # Create empty plot from dummy matrix to get perspective matrix 'pmat'
    pmat <- persp(x=x, y=y, z=m.na,
                  zlim=zlim, expand=expand, d=d, axes=FALSE, box=FALSE, ...)
    old.par <- par(lty=lin.lty, lwd=lin.lwd, col=lin.col)  # Set line style
    # For every z-tick draw a helper line on the specified xz and yz plane
    for (z in z.ticks[2:(length(z.ticks)-1)]) {
      # xz-plane: two points in 3D space -> two points in 2D space -> one line
      lines(trans3d(c(min(x), max(x)), lin.xy$y, z, pmat))
      # yz-plane: two points in 3D space -> two points in 2D space -> one line
      lines(trans3d(lin.xy$x, c(min(y), max(y)), z, pmat))
    }
    par(old.par)   # Revert changes made for setting the line style
    par(new=TRUE)  # Allow subsequent 'persp' to add to existing plot
  }
  # Overlay real plot over empty plot with helper lines
  persp(x=x, y=y, z=m,
        zlim=zlim, ticktype=ticktype, nticks=nticks, expand=expand, d=d, ...)
}

FacetColors <- function(m, maxmax=max(m)) {
  # Calculate colours for each of the facets (rectangles) in a persp plot. The
  # return value can be directly used for the 'col' option of 'persp'.
  # Args: m:      matrix from which a persp plot is to be drawn
  #       maxmax: if a batch of persp plots is created: the maximum value of all
  #               the matrices so that we can have same colours on same z values
  # Returns: vector of colours for the (ncol-1) * (nrow-1) facets of a persp
  # Note: https://stat.ethz.ch/pipermail/r-help/2003-September/039104.html
  #       This function has been found on this URL, I have no idea how it works.
  #----------------------------------------------------------------------------#
  palette <- terrain.colors(20)  # 20 colours from green to brown to white
  r <- nrow(m); c <- ncol(m)
  # This is part of the magic...
  m.avg <- (m[-1, -1] + m[-1, -(c-1)] + m[-(r-1), -1] + m[-(r-1), -(c-1)]) / 4
  # Low peak -> few breaks -> few colours (starting from beginning of palette)
  breaks <- (20 * max(m)) / maxmax
  # Creates a vector of length (nrow(m)-1) * (ncol(m)-1) with colours
  palette[cut(m.avg, breaks=breaks, include.lowest=TRUE)]
}

# These functions can be used to draw custom axes, if Persp has been called with
# 'axes=FALSE'. The three axes have to be drawn one after another. All functions
# take an argument 'a' whereof from the first three elements exactly one is a
# vector (tick positions), and the others are scalars, and the fourth is the
# perspective matrix returned by 'Persp':
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

# Format a numeric as integers or float character strings
Int   <- function(n)      { formatC(n, format="d", big.mark=",") }
Float <- function(n, d=1) { formatC(n, format="f", big.mark=",", digits=d) }

FormatName <- function(c) {
  # Convert a string of the form "fribourg.r2c.c.goal" to "Fribourg+R2C+C"
  #----------------------------------------------------------------------------#
  l <- strsplit(c, ".", fixed=TRUE)        # List of vectors with tokens
  sapply(l, function(t) {
              s <- length(t)-1             # Number of needed tokens
              t <- t[1:s]                  # Discard last token
              t[1] <- CapFirst(t[1])       # Capitalise first token
              if (s > 1)
                t[2:s] <- toupper(t[2:s])  # Capitalise remaining tokens
              paste(t, collapse="+")       # Concatenate tokens with "+"
            })
}

# Capitalise the first character of a string
CapFirst <- function(c) { paste0(toupper(substring(c, 1, 1)), substring(c, 2)) }

# Return transition and acceptance densities
Dt <- function() { c(1.0, 1.2, 1.4, 1.6, 1.8, 2.0, 2.2, 2.4, 2.6, 2.8, 3.0) }
Da <- function() { c(0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0) }

# Set margins on the four sides of the plot (in number of lines)
MarB <- function(size) { mar <- par("mar"); mar[1] <- size; par(mar=mar) }
MarL <- function(size) { mar <- par("mar"); mar[2] <- size; par(mar=mar) }
MarT <- function(size) { mar <- par("mar"); mar[3] <- size; par(mar=mar) }
MarR <- function(size) { mar <- par("mar"); mar[4] <- size; par(mar=mar) }


Complexity <- function(n, m) {
  # Write two numbers n and m in the form m = (xn)^n
  #----------------------------------------------------------------------------#
  x <- m^(1/n)/n  # x = nth_root(m)/n
  x <- round(x, digits=3)
  write(paste0("(",x,"n)^n"), file="")
}
