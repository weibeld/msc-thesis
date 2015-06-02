# R functions for drawing plots of the results of Büchi complementation
# experiments with GOAL
#
# Daniel Weibel <daniel.weibel@unifr.ch> Sep. 2014 - May 2015
#------------------------------------------------------------------------------#


#==============================================================================#
# Small graphics drawing framework
# Draw different kinds of graphics in a uniform way.
#==============================================================================#
Draw <- function(what, data, width=4, height=4, pointsize=8,
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
  if (dev.new) {
    if (dev == "pdf")
      pdf(width=width, height=height, pointsize=pointsize, file=file)
    else if (dev == "quartz")
      quartz(width=width, height=height, pointsize=pointsize)
    # Common graphics parameters
    par(mar=c(4,4,1,1))
    par(mgp=c(2.4,0.9,0))
    par(bty="o")
    par(lwd=0.5)
    par(las=1)
    par(xpd=TRUE)  # Allow plotting outside of plot region
  }
  if      (what == "stripchart") Stripchart(data, ...)
  else if (what == "boxplot")    Boxplot(data, ...)
  else if (what == "hist")       Hist(data, ...)
  else if (what == "dim3")       Dim3(data, ...)

  if (dev.off && dev != "quartz") dev.off()
}


# Stripchart <- function(data, labels=NULL, lmar=NULL, xaxp=NULL, ...) {
#   # Wrapper for stripchart()
#   # Args:
#   #   data:   Data to plot
#   #   labels: Labels for the individual strips
#   #   lmar:   Size of left margin of graphics
#   #   xaxp:   Graphical parameter xaxp specifying x-axis ticks
#   #   ...:    Arguments for stripchart()
#   #----------------------------------------------------------------------------#
#   SetLeftMargin(lmar)
#   stripchart(data, group.names=labels, method="jitter", pch=1, xaxt="n",
#              lwd=par("lwd"), ...)  # lwd sets line width of axes
#   xticks <- axTicks(1, axp=xaxp)
#   Axis(1, at=xticks, labels=xticks)
# }
  

Boxplot <- function(data, labels=NULL, out=FALSE, out.text=TRUE, out.cex=1,
                    mean=TRUE, yaxp=NULL, lmar=NULL, ...) {
  # Wrapper for boxplot()
  # Args:
  #   data:     Data to plot
  #   labels:   Labels for the individual boxplots
  #   out:      Draw outliers
  #   out.text: Add text stating number of outliers
  #   out.cex:  Size of text stating number of outliers
  #   mean:     Add dot showing mean
  #   lmar:     Size of left margin of graphics
  #   yaxp:     Graphical parameter yaxp specifying ticks of y-axis
  #   ...:      Arguments for boxplot()
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
  #   xaxp:  Graphical parameter xaxp specifying x-axis ticks
  #   yaxp:  Graphical parameter yaxp specifying y-axis ticks
  #   ...:   Arguments for hist()
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

# Dim3 <- function(df, type, x.var="da", y.var="dt", z.var="states",
#                  func=median, func.args=list(), matrix.only=FALSE, ...) {
#   # Draw plots for three-dimensional data. Wrapper for persp(), image(),
#   # contour(), and heatmap().
#   # Args:
#   #   df:          Data frame containing the three-dimensional data to plot
#   #   type:        One of "persp", "image", "contour", and "heatmap"
#   #   x.var:       Column of the data frame containing the x-values
#   #   y.var:       Column of the data frame containing the y-values
#   #   z.var:       Column of the data frame containing the z-values
#   #   func:        Function to apply to the classes of z-values
#   #   func.args:   List with further arguments to the function func
#   #   matrix.only: Do not plot, just return data as matrix with the required
#   #                form for the z argument of the underlying graphics functions.
#   #   ...:         Arguments for the underlying graphics functions.
#   # Returns:
#   #   If type="persp": The perspective matrix pmat, needed for adding custom
#   #                    elements (e.g. axes) to a persp plot.
#   # Details:
#   #   The combinations of x.var and y.var column values divide the data frame
#   #   in sets of rows. The vector of z.var column values in each of these sets
#   #   is passed to the function func which distilles it to a single value
#   #   (e.g. median). In this way, there will be one z-value for every combinat-
#   #   ion of x and y-values.
#   #----------------------------------------------------------------------------#
#   # Create matrix summarising three-dimensional data
#   row.indices <- as.numeric(names(table(df[[x.var]])))  # Row names = x.var vals
#   col.indices <- as.numeric(names(table(df[[y.var]])))  # Col names = y.var vals
#   cells <- numeric()  # Cell values of matrix (distilled z.var values)
#   for (r in row.indices) {
#     for (c in col.indices) {
#       z.vals <- df[df[[x.var]] == r & df[[y.var]] == c, z.var]
#       cells <- c(cells, do.call(func, c(list(z.vals), func.args)))
#     }
#   }
#   mat <- matrix(cells, nrow=length(row.indices), ncol=length(col.indices),
#                 byrow=TRUE, dimnames=list(row.indices, col.indices))
#   if (matrix.only) return(mat)
#   # Plot the above matrix
#   if      (type == "persp") {
#     # Good values: shade=0.75, theta=70, expand=0.25, nticks=11
#     persp(x=row.indices, y=col.indices, z=mat, ticktype="detailed", ...)
#   }
#   else if (type == "image") {
#     image(x=row.indices, y=col.indices, z=mat, ...)
#   }
#   else if (type == "contour") {
#     image(x=row.indices, y=col.indices, z=mat, ...)
#   }
#   else if (type == "heatmap") {
#     heatmap(mat, ...)
#   }
# }




# Axis <- function(...) {
#   # Wrapper for axis() with adapted line width by default
#   #----------------------------------------------------------------------------#
#   axis(..., lwd=par("lwd"))
# }

# SetLeftMargin <- function(lmar) {
#   # Set left margin for current graphics device driver
#   # Args:
#   #   lmar: Size of left margin. If NULL, nothing is done.
#   #----------------------------------------------------------------------------#
#   if (!is.null(lmar)) {
#     m <- par("mar")
#     m[2] <- lmar
#     par(mar=m)
#   }
# }







# Format <- function(n, t=1, dec=1, mark=",", all.marks=TRUE) {
#   # Format a vector of integers or floats for printing
#   # Args:
#   #   n:         Numeric or character vector (representing numbers) of any size
#   #   t:         Formatting type. 1=integer, 2=float
#   #   dec:       Number of digits after the decimal point (if t=2)
#   #   mark:      Character to use for the thousand-separator
#   #   all.marks: Use thousand-separator for all numbers >= 1000 (TRUE), or only
#   #              for numbers >= 10'000 (FALSE)
#   # Returns:
#   #   A character vector containing the formatted numbers.
#   #----------------------------------------------------------------------------#
#   n <- as.numeric(n)
#   format <- ifelse(t == 1, "d", "f")
#   if (all.marks)
#     return(formatC(n, format=format, digits=dec, big.mark=mark))
#   # If thousand-separator only for numbers >= 10'000:
#   res <- character()
#   for (scalar in n) {
#     res <- c(res, formatC(scalar, format=format, digits=dec,
#              big.mark=ifelse(scalar < 10000, "", mark)))
#   }
#   res
# }
