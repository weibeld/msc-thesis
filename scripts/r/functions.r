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

# Read in an .out file of the Büchi complementation experiments as a data frame
Read <- function(file) {
  read.table(file=file, header=TRUE, sep="\t")
}

# Count the number of timeouts in a data frame
Timeouts <- function(df) {
  table(df$t_out)
}

# Show all the rows having a timeout
TimeoutsShow <- function(df) {
  df[df$t_out == "Y",]
}

# Count the number of memory outs in a data frame
Memouts <- function(df) {
  table(df$m_out)
}

# Show all the rows with a memory out
MemoutsShow <- function(df) {
  df[df$m_out == "Y",]
}

# Determine the effective samples of the data frames passed as arguments. That
# is, all those complementation taks that have been completed in ALL the data
# frames passed as argument. Returns the data frames passed as argument (in a
# list) containing only the effective samples.
EffectiveSamples <- function(df) {
  mask <- logical(nrow(df[[1]])) # Logical vector initialised with all FALSE
  for (frame in df)
    mask <- mask | is.na(frame$states)
  df.reduced <- list()
  for (frame in df)
    df.reduced[[length(df.reduced)+1]] <- frame[!mask,]
  df.reduced
}

# Outputs m = (xn)^n
# m: states of output automaton
# n: states of input automaton
Complexity <- function(n, m) {
  x <- m^(1/n)/n  # x = nth_root(m)/n
  x <- round(x, digits=3)
  write(paste0("(",x,"n)^n"), file="")
}

test <- function(v, func=median, args) {
  func(v)
}



#==============================================================================#
# Graphics drawing framework
# Draw different kinds of graphics in a uniform way.
#==============================================================================#

# User interface function. Uniformly sets graphics device driver parameters
Draw <- function(what, data, width=NULL, height=NULL, pointsize=NULL, dev="quartz",
                 dev.new=TRUE, dev.off=TRUE, file="~/Desktop/out.pdf", ...) {
  Init(dev, dev.new, width, height, pointsize, file)

  if      (what == "stripchart") Stripchart(data, ...)
  else if (what == "boxplot")    Boxplot(data, ...)
  else if (what == "hist")       Hist(data, ...)
  else if (what == "persp")      Persp(data, ...)

  Finish(dev, dev.off)
}

Persp <- function(df, x="da", y="dt", z="states", func=median, func.args=list(), matrix.only=FALSE, ...) {
  x.classes <- as.numeric(names(table(df[[x]])))
  y.classes <- as.numeric(names(table(df[[y]])))
  values <- numeric()
  for (x.class in x.classes) {
    for (y.class in y.classes) {
      class.data <- df[df[[x]] == x.class & df[[y]] == y.class, z]
      values <- c(values, do.call(func, c(list(class.data), func.args)))
    }
  }
  m <- matrix(values, nrow=length(x.classes), ncol=length(y.classes), byrow=TRUE,
              dimnames=list(x.classes, y.classes))
  if (matrix.only) return(m)
   persp(x=x.classes, y=y.classes, z=m, expand=.25,
         ticktype="detailed", nticks=11, cex.axis=0.75, ...)
  #image(x=x.classes, y=y.classes, z=m)
  #heatmap(m)
  #contour(x=x.classes, y=y.classes, z=m, ...)
}

Stripchart <- function(data, ..., labels=NULL, lmar=NULL, xaxp=NULL) {
  # Increase left margin if desired
  SetLeftMargin(lmar)

  # Draw stripchart and axes. lwd sets line width of axes.
  stripchart(data, group.names=labels, las=1, method="jitter", pch=1, xaxt="n",
             lwd=par("lwd"), ...)
  xticks <- axTicks(1, axp=xaxp)
  Axis(1, at=xticks, labels=xticks)
}
  

Boxplot <- function(data, ..., labels=NULL, out=FALSE, out.text=TRUE, out.cex=1, mean=TRUE, yaxp=NULL, lmar=NULL) {
  # Ensure data is in list form
  if (class(data) != "list") data <- list(data)

  # Increase left margin if desired
  SetLeftMargin(lmar)

  # Draw boxplot and axes
  boxplot(data, outline=out, axes=FALSE, ...)
  box()
  Axis(1, at=seq(1,length(data)), labels=labels, numbers=FALSE)
  yticks <- axTicks(2, axp=yaxp)
  Axis(2, at=yticks, labels=yticks)

  # Additional elements for each boxplot
  i <- 1    
  for (x in data) {
    # Text with number of outliers
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
    # Dot showing mean value
    if (mean) {
      points(x=i, y=mean(x, na.rm=TRUE), pch=19)
    }
    i <- i+1
  }
}

Hist <- function(data, ..., lmar=NULL, marks=NULL, marks.cex=1, xaxp=NULL, yaxp=NULL) {
  # Increase left margin if desired
  SetLeftMargin(lmar)

  # Draw histogram and axes
  hist(data, main=NULL, axes=FALSE, ...)
  box()
  Axis(1, at=axTicks(1, xaxp), labels=axTicks(1, xaxp))
  Axis(2, at=axTicks(2, yaxp), labels=axTicks(2, yaxp))

  # Add lines indicating interesting values, e.g. median
  if (!is.null(marks)) {
    for (mark in marks) {
      x <- mark[[1]]
      y <- mark[[2]]
      str <- mark[[3]]
      end.x <- x + strwidth("MN")
      end.y <- y + strheight("M") * 0.8
      lines(c(x, x, end.x), c(0, y, end.y))
      text(end.x, end.y, str, pos=4, cex=marks.cex, offset=0.2)
    }
  } 
}

# Wrapper of axis(). Always adapt line width (lwd) and always format tick labels
# which are numbers.
Axis <- function(side, at, labels, numbers=TRUE, ...) {
  if (numbers) labels <- Format(labels)
  axis(side=side, at=at, labels=labels, lwd=par("lwd"), ...)
}

# Convenience function to set left margin of graphics
SetLeftMargin <- function(lmar) {
  if (!is.null(lmar)) {
    m <- par("mar")
    m[2] <- lmar
    par(mar=m)
  }
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
}

Finish <- function(dev, dev.off) {
  if (dev.off && dev != "quartz") dev.off()
}

# Format floats (t="f") or integers (t="d") for printing.
Format <- function(n, t="d", dec=1, mark=",", all.marks=TRUE) {
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
