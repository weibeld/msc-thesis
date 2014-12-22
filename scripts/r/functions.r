# Functions for analysing the results of the B??chi complementation experiments.
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


#==============================================================================#
# Data frame manipulation functions
#==============================================================================#
Read <- function(file) {
  # Read in a .out file a s a data frame
  #----------------------------------------------------------------------------#
  read.table(file=file, header=TRUE, sep="\t")
}

Timeouts <- function(df) {
  # Count number of timeouts
  #----------------------------------------------------------------------------#
  table(df$t_out)
}

Memouts <- function(df) {
  # Count number of memory outs
  #----------------------------------------------------------------------------#
  table(df$m_out)
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

#==============================================================================#
# Small graphics drawing framework
# Draw different kinds of graphics in a uniform way.
#==============================================================================#
Draw <- function(what, data, width=NULL, height=NULL, pointsize=NULL,
                 dev="quartz", dev.new=TRUE, dev.off=TRUE,
                 file="~/Desktop/out.pdf", ...) {
  # Entry point to the small graphics framework.
  # Args:
  #   what:      One of "stripchart", "boxplot", "hist", or "dim3"
  #   data:      Data to plot. Passed as first arg to underlying graphics func.
  #   width, height: Width and height of graphics device driver output
  #   pointsize: Point size of graphics device driver
  #   dev:       Grahpics device driver to use
  #   dev.new:   Create new driver instance (TRUE), or use existing (FALSE)
  #   dev.off:   Close driver on exit (TRUE), or let it open (FALSE)
  #   file:      Name of file to save graphics driver output to
  #   ...:       Arguments to underlying graphics functions
  #----------------------------------------------------------------------------#
  Init(dev, dev.new, width, height, pointsize, file)
  if      (what == "stripchart") Stripchart(data, ...)
  else if (what == "boxplot")    Boxplot(data, ...)
  else if (what == "hist")       Hist(data, ...)
  else if (what == "dim3")       Dim3(data, ...)
  Finish(dev, dev.off)
}

# Custom initialisation with default values of PDF graphics device driver
Init <- function(dev, dev.new, width, height, pointsize, file) {

  if (dev.new) {
    # Default values for all graphics devices
    if (is.null(width))     width     <- 4
    if (is.null(height))    height    <- 4
    if (is.null(pointsize)) pointsize <- 8

    # Start appropriate graphics device driver
    
    if (dev == "pdf")
      pdf(file=file, width=width, height=height, pointsize=pointsize)
    else if (dev == "quartz")
      quartz(width=width, height=height, pointsize=pointsize)
  }

  # Set default graphics parameters
  par(mar=c(4,4,1,1))
  par(mgp=c(2.4,0.9,0))
  par(bty="o")
  par(lwd=0.5)
  par(las=1)
  par(xpd=TRUE) # Allow plotting outside of plot region
}

Finish <- function(dev, dev.off) {
  if (dev.off && dev != "quartz") dev.off()
}


Stripchart <- function(data, labels=NULL, lmar=NULL, xaxp=NULL, ...) {
  # Wrapper for stripchart()
  # Args:
  #   data:   Data to plot
  #   labels: Labels for the individual strips
  #   lmar:   Size of left margin of graphics
  #   xaxp:   Graphical parameter xaxp specifying x-axis ticks
  #   ...:    Arguments for stripchart()
  #----------------------------------------------------------------------------#
  SetLeftMargin(lmar)
  stripchart(data, group.names=labels, las=1, method="jitter", pch=1, xaxt="n",
             lwd=par("lwd"), ...)  # lwd sets line width of axes
  xticks <- axTicks(1, axp=xaxp)
  Axis(1, at=xticks, labels=xticks)
}
  

Boxplot <- function(data, labels=NULL, out=FALSE, out.text=TRUE, out.cex=1,
                    mean=TRUE, yaxp=NULL, lmar=NULL, ...) {
  # Wrapper for boxplot()
  # Args:
  #   data: Data to plot
  #   labels: Labels for the individual boxplots
  #   out: Draw outliers
  #   out.text: Add text stating number of outliers
  #   out.cex: Size of text stating number of outliers
  #   mean: Add dot showing mean
  #   lmar: Size of left margin of graphics
  #   yaxp: Graphical parameter yaxp specifying ticks of y-axis
  #   ...: Arguments for boxplot()
  #----------------------------------------------------------------------------#
  if (!is.list()) data <- list(data)  # We need data in list form
  SetLeftMargin(lmar)
  boxplot(data, outline=out, axes=FALSE, ...)
  box()
  Axis(1, at=seq(1,length(data)), labels=labels, numbers=FALSE)
  yticks <- axTicks(2, axp=yaxp)
  Axis(2, at=yticks, labels=yticks)
  # Additional decoration for each boxplot
  i <- 1    
  for (x in data) {
    # Dot showing mean value
    if (mean) {
      points(x=i, y=mean(x, na.rm=TRUE), pch=19)
    }
    # Text stating number of outliers
    if (out.text) {
      info <- boxplot.stats(x)
      out.nb <- length(info$out)
      out.perc <- 100*out.nb/info$n
      out.max <- max(info$out)
      text <- paste0(out.nb, " outliers (",Format(out.perc, "f"),"%)\n",
                     "Max. ",Format(out.max))
      upper.whisker <- info$stats[5]
      text(x=i, y=upper.whisker, labels=text, pos=3, cex=out.cex)
    }
    i <- i+1
  }
}


Hist <- function(data, lmar=NULL, lines=NULL, lines.cex=1, xaxp=NULL, yaxp=NULL,
                 ...) {
  # Wrapper for hist()
  # Args:
  #   data:  Data to plot
  #   lmar:  Size of left margin of graphics
  #   lines: List of lists of length 3 in following form:
  #          comp 1: x-position of vertical line
  #          comp 2: y-position of end of vertical line
  #          comp 3: Label for line
  #   lines.cex: Text size of labels of vertical lines
  #   xaxp: Graphical parameter xaxp specifying x-axis ticks
  #   yaxp: Graphical parameter yaxp specifying y-axis ticks
  #   ...: Arguments for hist()
  #----------------------------------------------------------------------------#
  SetLeftMargin(lmar)
  hist(data, main=NULL, axes=FALSE, ...)
  box()
  Axis(1, at=axTicks(1, xaxp), labels=axTicks(1, xaxp))
  Axis(2, at=axTicks(2, yaxp), labels=axTicks(2, yaxp))
  # Add vertical lines
  if (!is.null(lines)) {
    for (line in lines) {
      x <- line[[1]]; y <- line[[2]]; str <- line[[3]]
      end.x <- x + strwidth("MN")
      end.y <- y + strheight("M") * 0.8
      lines(c(x, x, end.x), c(0, y, end.y))
      text(end.x, end.y, str, pos=4, cex=lines.cex, offset=0.2)
    }
  } 
}

# Plots for three-dimensional data. Includes persp, image, contour, and heatmap
# plots. The data to plot is determined by three columns (x.var, y.var, z.var)
# of a data frame (df). The assumptions on this data frame are the following:
# the values of x.var and y.var divide the data frame in |x.var| * |y.var|
# classes. That is, there must be one or more rows (classes) in the data frame
# for every combination of the x.var and y.var values. Each of these xy
# classes has a z-value. This z-value is a function of the values of the z.var
# column. (This function can be specified (func). The first argument to func
# will be a vector of the z.var values and further argments can be specified as
# a list with func.args.) What results is a three-dimensional data relation with
# a set of x-values, a set of y-value, and a z-value for every combination of x
# and y-values. This data relation is then plotted as either a persp, image,
# contour, or heatmap plot (type). The reason that all these different plots
# are in the same wrapper function is that they are all based on the same
# data structure: a matrix where the row indices are taken as the x-values, the
# column indices as the y-values, and the cells as the z-values. This bare
# matrix can be returned with matrix.only=TRUE
Dim3 <- function(df, type, x.var="da", y.var="dt", z.var="states",
                 func=median, func.args=list(), matrix.only=FALSE, ...) {
  # Draw plots for three-dimensional data. Wrapper for persp(), image(),
  # contour(), and heatmap().
  # Args:
  #   df:          Data frame containing the three-dimensional data to plot
  #   type:        One of "persp", "image", "contour", and "heatmap"
  #   x.var:       Column of the data frame containing the x-values
  #   y.var:       Column of the data frame containing the y-values
  #   z.var:       Column of the data frame containing the z-values
  #   func:        Function to apply to the classes of z-values
  #   func.args:   List with further arguments to the function func
  #   matrix.only: Do not plot, just return data as matrix with the required
  #                form for the z argument of the underlying graphics functions.
  #   ...:         Arguments for the underlying graphics functions.
  # Returns:
  #   If type="persp": The perspective matrix pmat, needed for adding custom
  #                    elements (e.g. axes) to a persp plot.
  # Details:
  #   The combinations of x.var and y.var column values divide the data frame
  #   in sets of rows. The vector of z.var column values in each of these sets
  #   is passed to the function func which distilles it to a single value
  #   (e.g. median). In this way, there will be one z-value for every combinat-
  #   ion of x and y-values.
  #----------------------------------------------------------------------------#
  # Create matrix summarising three-dimensional data
  row.indices <- as.numeric(names(table(df[[x.var]])))  # Row names = x.var vals
  col.indices <- as.numeric(names(table(df[[y.var]])))  # Col names = y.var vals
  cells <- numeric()  # Cell values of matrix (distilled z.var values)
  for (r in row.indices) {
    for (c in col.indices) {
      z.vals <- df[df[[x.var]] == r & df[[y.var]] == c, z.var]
      cells <- c(cells, do.call(func, c(list(z.vals), func.args)))
    }
  }
  mat <- matrix(cells, nrow=length(row.indices), ncol=length(col.indices),
                byrow=TRUE, dimnames=list(row.indices, col.indices))
  if (matrix.only) return(mat)
  # Plot the above matrix
  if      (type == "persp") {
    # Good values: shade=0.75, theta=70, expand=0.25, nticks=11
    persp(x=row.indices, y=col.indices, z=mat, ticktype="detailed", ...)
  }
  else if (type == "image") {
    image(x=row.indices, y=col.indices, z=mat, ...)
  }
  else if (type == "contour") {
    image(x=row.indices, y=col.indices, z=mat, ...)
  }
  else if (type == "heatmap") {
    heatmap(mat, ...)
  }
}


Axis <- function(...) {
  # Add an axis to a normal two-dimensional plot. Adapts line width by default.
  # Args:
  #   ...: Arguments to axis()
  #----------------------------------------------------------------------------#
  axis(..., lwd=par("lwd"))
}

SetLeftMargin <- function(lmar) {
  # Set left margin of a graphics
  # Args:
  #   lmar: Size of left margin. If NULL, nothing is done.
  #----------------------------------------------------------------------------#
  if (!is.null(lmar)) {
    m <- par("mar")
    m[2] <- lmar
    par(mar=m)
  }
}

# Format floats (t="f") or integers (t="d") for printing.
Format <- function(n, t="d", dec=1, mark=",", all.marks=TRUE) {
  n <- as.numeric(n)
  res <- numeric()
  if (!all.marks) {
    for (scalar in n) {
      m <- ifelse(scalar<10000, "", mark)
      res <- c(res, formatC(scalar, format=t, digits=dec, big.mark=m))
    }
  }
  else {
    res <- formatC(n,format=t, digits=dec, big.mark=mark)
  }
  res
}

#==============================================================================#
# Custom axes, axis ticks, and axis labels for persp plot
# All function take a list a of the following form as their argument:
#   a[[1]]: Ticks of x-axis (if drawing x-axis), or x-position of other axis
#   a[[2]]: Ticks of y-axis (if drawing x-axis), or y-position of other axis
#   a[[3]]: Ticks of z-axis (if drawing x-axis), or z-position of other axis
#   a[[4]]: Perspective matrix (pmat) returned by persp
#==============================================================================#
PerspAxis <- function(a, ...) {
  # Add an axis to a persp plot.
  # Args:
  #   a:   The list described above
  #   ...: Arguments for lines()
  #----------------------------------------------------------------------------#
  lines(trans3d(a[[1]], a[[2]], a[[3]], a[[4]]), ...)
}

PerspTicks <- function(a, tickl.len=strwidth("M"), tick.dir=c(1,0), ...) {
  # Add axis tick marks to one axis of a persp plot.
  # Args:
  #   tick.len: Length of ticks
  #   tick.dir: Direction of ticks in other two dimensions. Vector of size 2
  #             with values 1, 0, or -1. If size is 1, second elt. is set to 0.
  #   ...:      Arguments for segment()
  #----------------------------------------------------------------------------#
  if (length(tick.dir) < 2) tick.dir[2] <- 0
  lines <- PerspLines(a, tick.len, tick.dir)
  segments(lines$inner$x, lines$inner$y, lines$outer$x, lines$outer$y, ...)
}

PerspAxisLabel <- function(a, label="Move me", ...) {
  # Add an axis label to a an axis of a persp plot.
  # Args:
  #   a:     The list described above
  #   label: Text of the axis label
  #   ...:   Arguments for text()
  #----------------------------------------------------------------------------#
  x <- a[[1]]; y <- a[[2]]; z <- a[[3]]; pmat <- a[[4]]
  if      (length(x) > 1) x <- (min(x) + max(x)) / 2  # If label for x-axis
  else if (length(y) > 1) y <- (min(y) + max(y)) / 2  # If label for y-axis
  else if (length(z) > 1) z <- (min(z) + max(z)) / 2  # If label for z-axis
  text(trans3d(x, y, z, pmat), labels=label, ...)
}

PerspTickLabels <- function(a, labels, tick.len=strwidth("M"), tick.dir=c(1,0),
                            ...) {
  # Add tick mark labels to an axis of a persp plot.
  # Args:
  #   a:        The list described above
  #   labels:   The tick mark labels
  #   tick.len: Length of ticks
  #   tick.dir: Direction of ticks in other two dimensions. Vector of size 2
  #             with values 1, 0, or -1. If size is 1, second elt. is set to 0.
  #   ...:      Arguments for text()
  #----------------------------------------------------------------------------#
  x <- a[[1]]; y <- a[[2]]; z <- a[[3]]
  if (length(tick.dir) < 2) tick.dir[2] <- 0
  lines <- PerspLines(a, tick.len, tick.dir)
  text(lines$outer$x, lines$outer$y, labels, ...)
}

PerspLines <- function(a, gap, dir) {
  # Compute two lines parallel to one axis spanning the ticks.
  # Private helper function for PerspTicks and PerspTickLabels.
  # Args:
  #   a:   The list described above
  #   gap: The distance between outer and inner linee (length of ticks)
  #   dir: Vector of size 2 with values 1, 0, or -1. Direction of outer line to
  #        inner line in the other two dimensions.
  # Returns:
  #   List:
  #   $inner: trans3d line identical with axis
  #   $outer: trans3d line parallel to $inner
  #----------------------------------------------------------------------------#
  x <- a[[1]]; y <- a[[2]]; z <- a[[3]]; pmat <- a[[4]]
  inner <- trans3d(x, y, z, pmat)
  # Modify parameters for outer line
  if      (length(x) > 1) {  # If lines on x-axis
    y <- y + (gap * dir[1])
    z <- z + (gap * dir[2])
  }
  else if (length(y) > 1) {  # If lines on y-axis
    x <- x + (gap * dir[1])
    z <- z + (gap * dir[2])
  }
  else if (length(z) > 1) {  # If lines on z-axis
    x <- x + (gap * dir[1])
    y <- y + (gap * dir[2])
  }
  outer <- trans3d(x, y, z, pmat)
  list(inner=inner, outer=outer)
}
