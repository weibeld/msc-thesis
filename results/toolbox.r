# R functions for analyising the results of Büchi complementation experiments
# with GOAL on the test automata from the paper "State of Büchi Complementation"
# 
# Test automata available on: http://goal.im.ntu.edu.tw/wiki/lib/exe/fetch.php?
#                             media=goal:ciaa2010_automata.tar.gz
#
# Daniel Weibel <daniel.weibel@unifr.ch> Sep. 2014 - May 2015
#------------------------------------------------------------------------------#

SetDir <- function() {
  # Change working directory so that we can work with relative paths only
  #----------------------------------------------------------------------------#
  setwd("~/Desktop/thesis/results")
}

ReadSingle <- function(file) {
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
    name  <- FormatName(basename(f))
    l[[name]] <- ReadSingle(f)  
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
                      Timeouts          = nrow(df[df$timeout == "Y",]),
                      `Memory excesses` = nrow(df[df$memout  == "Y",]), check.names=FALSE)
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

Stats <- function(list, dat="states", total=FALSE, total.hours=FALSE) {
  # Get statistics aggregated over entire data frame for a list of data frames
  # Args: list:   a list of data frames
  #       dat:    name of the column for which to calculate the statistics
  #       labels: character vector with a label for each data frame in 'list'
  # Returns: a data frame with the stats with one row for each input data frame
  #----------------------------------------------------------------------------#
  i <- 1
  for (df in list) {
    if (any(is.na(df$states))) stop("data frame has NA in column 'states'")
    x <- df[[dat]]
    row <- data.frame(Construction = names(list)[i],
                      Mean         = mean(x),
                      Min.         = min(x),
                      P25          = quantile(x, 0.25, names=FALSE),
                      Median       = median(x),
                      P75          = quantile(x, 0.75, names=FALSE),
                      Max.         = max(x))
    if (total) {
      tot <- sum(x)
      row <- cbind(row, data.frame(Total=tot))
      if (total.hours) {
        h <- round(tot / 360)
        row <- cbind(row, data.frame(`$\\approx$ hours`=h, check.names=FALSE))
      }
    }
    if (i == 1) res <- row else res <- rbind(res, row)
    i <- i + 1
  }
  res
}

StatsGoal <- function(list, dat="states") {
  # Get statistics split up by the 110 transition/acceptance density classes
  # Args: list: named list of GOAL result data frames with no NA states
  #       data:  name of the column for which to calculate the statistics
  # Returns: list of data frames with one row for each dt/da class
  #----------------------------------------------------------------------------#
  res.list <- list()
  i <- 1
  for (df in list) {
    if (any(is.na(df$states))) stop("data frame has NA in column 'states'")
    j <- 1
    for (t in Dt()) {
      for (a in Da()) {
        x <- df[df$dt == t & df$da == a, dat]
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

MichelTable <- function(list, dat="states", fit.fun=TRUE, std.err=TRUE, raw=FALSE) {
  # Combine results for Michel automata in a single data frame
  # Args: list:    named list of Michel result data frames
  #       dat:     name of the column to display in the result
  #       fit.fun: add column with fitted state growth function (an)^n
  #       std.err: add column with std. error of fitted function
  #       raw:     if TRUE, print fitted curve and std. error as raw numbers,
  #                if FALSE, format them for LaTeX printing
  # Returns: data frame with one row for each data frame in 'list'
  #----------------------------------------------------------------------------#
  i <- 1
  for (df in list) {
    row <- data.frame(Construction = names(list)[i],
                      `Michel 1`   = df[1, dat],
                      `Michel 2`   = df[2, dat],
                      `Michel 3`   = df[3, dat],
                      `Michel 4`   = df[4, dat], check.names=FALSE)
    if (fit.fun) {
      x <- c(3, 4, 5, 6)
      y <- df[[dat]]
      fit <- nls(y ~ (a*x)^x, start=list(a=1))
      a <- summary(fit)$parameters[1]
      if (raw) text <- paste0("(", a, "n)^n")
      else     text <- paste0("$(", Float(a, d=2), "n)^n$")
      row <- cbind(row, data.frame(`Fitted curve`=text, check.names=FALSE))
      if (std.err) {
        s <- summary(fit)$parameters[2]
        if (raw) text <- paste0(s*100, "%")
        else     text <- paste0(Float(s*100, d=2), "\\%")
        row <- cbind(row, data.frame(`Std. error`=text, check.names=FALSE))
      }
    }
    if (i == 1) res <- row else res <- rbind(res, row)
    i <- i + 1
  }
  res
}

MichelBarplot <- function(list, dat="states", gap=1, lab.rot=45, lab.dst=0.33,
                          lin=FALSE, lin.col="gray", lin.lty="dotted",
                          lin.lwd=par("lwd"), yaxp=par("yaxp"),
                          ylim=c(0, max(sapply(lapply(list, `[`, dat), max))), ...) {
  # Create a barplot with one bar group for every construction
  # Args: list:    named list of Michel result data frames
  #       dat:     column name of the data to plot
  #       ylim:    y-range, defaults from zero to greatest data point
  #       gap:     gap between bar groups (assume bar width of 1)
  #       lab.rot: rotation angle for the group labels on the x-axis
  #       lab.dst: distance between group labels and x-axis
  #       lin:     if TRUE, draw horizontal helper lines at y-axis ticks
  #       lin.col, lin.lty, lin.lwd: colour, type, and width for helper lines
  #       yaxp:    ticks on y-axis
  #       ...:   arguments to 'barplot'
  # Note: if there are issues after setting 'gap' or xaxs="i", try to fix them
  #       by adding an x-axis with 'axis(1)' and setting 'xlim' explicitly.
  #----------------------------------------------------------------------------#
  # Create matrix with: rows: Michel automata; cols: constructions
  v <- numeric()
  m <- sapply(lapply(list, `[`, , dat), append, v)
  nbars <- nrow(m); ngroups <- ncol(m)
  # Draw empty plot and then helper lines
  if (lin) {
    m.empty <- m; m.empty[,] <- 0  # Plot zeros as dummy data
    barplot(m.empty, beside=TRUE, xaxt="n", yaxt="n", space=c(0, gap), width=1, ylim=ylim, ...)
    par(yaxp=yaxp)
    abline(h=tail(axTicks(2), -1), col=lin.col, lty=lin.lty)  # Add helper lines
    par(new=TRUE)
  }
  # Draw barplot; the values of each row of 'm' form a group
  barplot(m, beside=TRUE, xaxt="n", yaxt="n", space=c(0, gap), width=1, ylim=ylim, ...)
  # Draw custom y-axis
  par(yaxp=yaxp)
  axis(2, at=axTicks(2), labels=Int(axTicks(2)), las=1)
  # Note: the following assumes that the width of a bar is 1 (see barplot above)
  # Ensure a tick at the right edge of the last bar of each group
  par(xaxp=c(0, (nbars + gap) * ngroups, ngroups))  # Test with axis(1)
  # Draw group labels in the center of each bar group (nbars / 2)
  x.pos <- head(axTicks(1), -1) + gap + (nbars / 2)
  y.pos <- -lab.dst * diff(axTicks(2))[1]
  text(x=x.pos, y=y.pos, labels=colnames(m), xpd=TRUE, pos=2, offset=0, srt=lab.rot)
}

MatrixGoal <- function(list, dat="states", stat="median") {
  # Get a single statistics as dt/da matrices
  # Args: list: named list of GOAL result data frames with no NA states
  #       dat:  column for which to get the statistics
  #       stat: statistics (column name of result of 'StatsGoal')
  # Returns: list of matrices with elements named after the input data frames
  #----------------------------------------------------------------------------#
  res <- list()
  s <- StatsGoal(list, dat=dat)
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
  if ("empty"     %in% colnames(df)) col <- "empty"     else
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

LatexTable <- function(x, format="f", digits=1, align=NULL, include.rownames=FALSE, ...) {
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

  # If input is data frame: format it
  if (is.data.frame(x)) {
    if (length(format) == 1) format <- rep_len(format, ncol(x))
    i <- 1
    for (col in x) {
      # Format numbers and keep them right-aligned
      if (class(col) == "numeric" | class(col) == "integer") {
        # Format 'numeric' and 'integer' columns, result is 'character'
        if (format[i] == "f") x[[i]] <- Float(col, d=digits) else
        if (format[i] == "d") x[[i]] <- Int(col)
        # Even though the column is now 'character', we want it right-aligned
        if (is.null(align)) align.auto <- paste0(align.auto, "r")
      }
      # If the column is 'character' or 'factor', do left-align
      else {
        if (is.null(align)) align.auto <- paste0(align.auto, "l")
      }
      i <- i + 1
    }
    
  } else

  # If input is matrix: format it
  if (is.matrix(x)) {
    if (format[1] == "f") x <- Float(x, d=digits) else
    if (format[1] == "d") x <- Int(x)
    # All values in a matrix right-aligned
    if (is.null(align))
      for (i in seq(1, ncol(x))) align.auto <- paste0(align.auto, "r")
  }

  else stop("argument 'x' must be data frame or matrix, found ", class(x))

  # Use the automatically determined alignment if no 'align' is specified
  if (is.null(align)) align <- align.auto

  # Create the LaTeX table
  library(xtable)
  print(xtable(x, align=align), include.rownames=include.rownames,
        sanitize.text.function=identity, floating=FALSE, ...)
}

MatrixAgg <- function(lst.m, func=mean) {
  # Aggregate the corresponding cells of a list of matrices
  # Args: lst.m: list of matrics
  #       func:  function taking as input a numerical vector and returning a
  #              single number
  # Returns: a matrix with the aggregated values of the input matrices
  #----------------------------------------------------------------------------#
  nr <- nrow(lst.m[[1]]); nc <- ncol(lst.m[[1]])
  agg.vals <- numeric()
  for (r in seq(1, nr)) {
    for (c in seq(1, nc)) {
      vals <- sapply(lst.m, `[`, r, c)
      agg.vals <- c(agg.vals, func(vals))
    }
  }
  matrix(agg.vals, nrow=nr, ncol=nc, byrow=TRUE, dimnames=dimnames(lst.m[[1]]))
}

Image <- function(m, ...) {
  # Draw an image plot of a dt/da or da/dt matrix
  # Args: m: a dt/da or da/dt matrix (rows/columns)
  #       ...: arguments for 'image'
  #----------------------------------------------------------------------------#
  nr <- nrow(m); nc <- ncol(m)
  m <- t(m)[,nr:1]  # This makes the image having the same layout as matrix 'm'
  # Draw image plot
  image(x=1:nc, y=1:nr, z=m, xaxt="n", yaxt="n", xlab="", ylab="", ...)
  if (nr == 11 & nc == 10) {  # Initial matrix is rows=dt, cols=da (dt/da)
    x.val <- Float(Da()); x.lab <- "Acceptance density"
    y.val <- Float(Dt()); y.lab <- "Transition density"
  }
  else if (nr == 10 & nc == 11) {  # Initial matrix is rows=da, cols=dt (da/dt)
    x.val <- Float(Dt()); x.lab <- "Transition density"
    y.val <- Float(Da()); y.lab <- "Acceptance density"
  }
  usr <- par("usr")  # Get extreme values of x-axis [1,2], and y-axis [3,4]
  par(xpd=TRUE)      # Allow drawing outside of plot area
  gap1 <- 0.25       # Gap between plot and "tick marks"
  gap2 <- 1.2        # Gap between plot and axis labels
  # Labels on x-axis (top)
  text(x=1:nc, y=usr[4]+gap1, labels=x.val, pos=3, offset=0)
  x.mid <- (usr[2] - usr[1]) / 2 + usr[1]  # Middle of x-axis
  text(x=x.mid, y=usr[4]+gap2, labels=x.lab)
  # Labels on y-axis (left)
  text(x=usr[1]-(gap1 * nc / (nr+0.5)), y=nr:1, labels=y.val, pos=2, offset=0)
  y.mid <- (usr[4] - usr[3]) / 2 + usr[3]  # Middle of y-axis
  text(x=usr[1]-gap2-0.15, y=y.mid, labels=y.lab, srt=90)
  # Add grid
  abline(h=seq(usr[3], usr[4]), v=seq(usr[1], usr[2]), xpd=FALSE)
}

Stripchart <- function(list, dat="states", jitter=0.25, ...) {
  # Draw stripcharts from column 'dat' from a list of data frames
  # Args: list:   named list of data frames
  #       dat:    column with the values to plot
  #       jitter: width of the strips (higher values are wider)
  #----------------------------------------------------------------------------#
  data <- lapply(list, `[[`, dat)  # Named list of vectors
  data <- rev(data)      # Reverse list to obtain top-down order of strips
  las <- 1               # All axis tick labes horizontal
  pch <- 1               # Circles as plotting symbols
  stripchart(data, method="jitter", jitter=jitter, las=las, pch=pch, xaxt="n", ...)
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

PerspBatch <- function(list, pdf=FALSE, dir=".", prefix="", width=7, height=7, mar=par("mar"),
                       zlim=c(0, max(sapply(list, max))), z.colors=TRUE, z.color.breaks=20,
                       custom.zlab=NULL, ...) {
  # Draw a batch of persp plots with identical parameters
  # Args: list:        named list of dt/da or da/dt matrices (rows/columns)
  #       pdf:         use 'pdf' driver (TRUE) or 'quartz' (FALSE)
  #       dir:         directory where to save the PDF files (if pdf=TRUE)
  #       prefix:      prefix string for filenames (filename is list elt. name)
  #       width, height: width and height of the output graphics
  #       mar:         margins of the output graphics
  #       zlim:        common zlim for all the plots
  #       z.colors:    if TRUE, use a z-dependent colour scheme
  #       custom.zlab: a function without arguments placing text on the plot
  #       ...:         arguments to 'Persp'
  #----------------------------------------------------------------------------#
  max <- max(sapply(list, max))  # Greatest element of all the matrices
  pdf.files <- paste0(dir, "/", prefix, names(list), ".pdf")  # Names of output files
  i <- 1
  for (m in list) {
    if (pdf) pdf(file=pdf.files[i], width=width, height=height)
    else     quartz(width=width, height=height)
    par(mar=mar)  # Set margins
    if (z.colors) Persp(m, zlim=zlim, col=FacetColors(m, max, breaks=z.color.breaks), ...)
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
  d        <- 6                 # Lessen the "perspective effect" (default is 1)

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

FacetColors <- function(m, maxmax=max(m), breaks=20) {
  # Calculate colours for each of the facets (rectangles) in a persp plot. The
  # return value can be directly used for the 'col' option of 'persp'.
  # Args: m:      matrix from which a persp plot is to be drawn
  #       maxmax: if a batch of persp plots is created: the maximum value of all
  #               the matrices so that we can have same colours on same z values
  # Returns: vector of colours for the (ncol-1) * (nrow-1) facets of a persp
  # Note: https://stat.ethz.ch/pipermail/r-help/2003-September/039104.html
  #       This function has been found on this URL, I have no idea how it works.
  #----------------------------------------------------------------------------#
  palette <- terrain.colors(breaks)  # colours from green to brown to white
  r <- nrow(m); c <- ncol(m)
  # This is part of the magic...
  m.avg <- (m[-1, -1] + m[-1, -(c-1)] + m[-(r-1), -1] + m[-(r-1), -(c-1)]) / 4
  # Low peak -> few breaks -> few colours (starting from beginning of palette)
  breaks <- (breaks * max(m)) / maxmax
  if (breaks < 2) breaks <- 2  # Must be >= 2 for 'cut'
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
  # Convert a filename of the form "fribourg.r2c.c.goal.csv" to "Fribourg+R2C+C"
  #----------------------------------------------------------------------------#
  l <- strsplit(c, ".", fixed=TRUE)        # List of vectors with tokens
  sapply(l, function(t) {
              s <- length(t)-2             # Number of needed tokens
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

# Convert seconds to minues, hours, and days
Sec2Min  <- function(sec) { sec / 60       }
Sec2Hrs  <- function(sec) { sec / 3600     }  # 60 * 60
Sec2Days <- function(sec) { sec / 86400    }  # 60 * 60 * 24
Sec2Yrs  <- function(sec) { sec / 31557600 }  # 60 * 60 * 24 * 365.25

Perc <- function(n1, n2) {
  # How many percents is 'n1' of 'n2'? 'n2' is taken as 100%
  #----------------------------------------------------------------------------#
  (100 * n1) / n2
}

Complexity <- function(n, m) {
  # Write two numbers n and m in the form m = (xn)^n
  #----------------------------------------------------------------------------#
  x <- m^(1/n)/n  # x = nth_root(m)/n
  x <- round(x, digits=3)
  write(paste0("(",x,"n)^n"), file="")
}
