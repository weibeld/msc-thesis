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

#==============================================================================#
# Graphics functions
# Draw different kinds of plots from the Büchi complementation data.
#==============================================================================#

Draw <- function(what, width=NULL, height=NULL, pointsize=NULL, dev="quartz",
                 dev.new=TRUE, dev.off=TRUE, file="~/Desktop/out.pdf", ...) {
  Init(dev, dev.new, width, height, pointsize, file)

  if      (what == "stripchart") Stripchart(...)
  else if (what == "boxplot")    Boxplot(...)
  else if (what == "hist")       Hist(...)

  Finish(dev, dev.off)
}

Stripchart <- function(data, ..., labels=NULL, lmar=NULL, xaxp=NULL) {
  # Increase left margin if desired
  if (!is.null(lmar)) {
    m <- par("mar")
    m[2] <- lmar
    par(mar=m)
  }
  # Draw stripchart. lwd sets line width of axes.
  stripchart(x, group.names=labels, las=1, method="jitter", pch=1, xaxt="n",
             lwd=par("lwd"), ...)
  # Add x-axis (according to user specification, if !is.null(xaxp)
  tick.pos <- axTicks(1, axp=xaxp)
  tick.labels <- Format(tick.pos, t="d")
  axis(1, at=tick.pos, labels=tick.labels, lwd=par("lwd"))
}


Boxplot <- function(data, ..., labels=NULL, out=FALSE, out.text=TRUE, mean=TRUE) {
  # Ensure data is in list form
  if (class(data) != "list") data <- list(data)

  # Draw boxplot diagram
  boxplot(data, names=labels, outline=out, xaxt="n", yaxt="n", ...)

  # Add customised axes
  axis(1, at=seq(1,length(data)), labels=labels, lwd=par("lwd"))
  axis(2, at=axTicks(2), labels=Format(axTicks(2), t="d"), lwd=par("lwd"))

  # Additional elements for each boxplot
  i <- 1    
  for (x in data) {
    # Text with number of outliers
    if (out.text) {
      info <- boxplot.stats(x)
      out.nb <- length(info$out)
      out.perc <- 100*out.nb/info$n
      out.max <- max(info$out)
      text <- paste0(out.nb, " outliers (",Format(out.perc, t="f"),"%)\n",
                     "Max. ",Format(out.max,t="d", all.marks=FALSE))
      upper.whisker <- info$stats[5]
      text(x=i, y=upper.whisker, labels=text, pos=3, cex=0.675)
    }
    # Dot showing mean value
    if (mean) {
      points(x=i, y=mean(x, na.rm=TRUE), pch=19)
    }
    i <- i+1
  }
}

Hist <- function(data, ..., lmar=NULL, lines=NULL, cex.lines=1, xaxp=NULL, yaxp=NULL) {
  # Increase left margin if desired
  if (!is.null(lmar)) {
    m <- par("mar")
    m[2] <- lmar
    par(mar=m)
  }

  hist(data, main=NULL, axes=FALSE, ...)
  box()
  axis(1, at=axTicks(1, xaxp), labels=Format(axTicks(1, xaxp)), lwd=par("lwd"))
  axis(2, at=axTicks(2, yaxp), labels=Format(axTicks(2, yaxp)), lwd=par("lwd"))

  if (!is.null(lines)) {
    for (line in lines) {
      x <- line[[1]]
      str <- line[[2]]
      y <- line[[3]]
      # abline(v=x)
      # box.w <- strwidth(str, cex=cex.lines) + strwidth("m", cex=cex.lines)
      # box.h <- strheight(str, cex=cex.lines) * 2
      # rect(xleft=x-box.w/2, ybottom=y-box.h/2, xright=x+box.w/2, ytop=y+box.h/2, col="white", border="black")
      # text(x, y, str, cex=cex.lines)
      # end.x <- x + ((par("usr")[2] - par("usr")[1]) / 20)
      # end.y <- y + ((par("usr")[4] - par("usr")[3]) / 40)
      end.x <- x + strwidth("MN")
      end.y <- y + strheight("M") * 0.8
      lines(c(x, x, end.x), c(0, y, end.y))
      text(end.x, end.y, str, pos=4, cex=cex.lines, offset=0.2)
    }
  } 
}


hist.states <- function(df,xmax=10000,ymax=2000,binsize=100,
    title="Histogram",xlabel="Number of states",ylabel="Complements",
    file="~/Desktop/histogram.pdf",width=7,height=7) {
  # Start graphics device and set graphics params
  pdf(file)
  global.params()
  binbreaks <- seq(from=0,to=100000,by=binsize)
  # Draw histogram
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
  # Close graphics device (saves graphic to file)
  dev.off()
}

# Create a (draft) histogram for any data vector.
hist.generic <- function(vector,xmax,ymax,binsize,file="~/Desktop/histogram.pdf") {
  # Start graphics device and set graphics params
  pdf(file)
  global.params()
  binbreaks <- seq(from=0,to=100000,by=binsize)
  # Draw histogram
  hist(vector,breaks=binbreaks,xlim=c(0,xmax),ylim=c(0,ymax))
  # Draw median and 95th percentile line
  median <- median(vector)
  perc95 <- quantile(vector,0.95)
  textsize=0.75
  abline(v=median,col="blue")
  text(x=median,y=0,paste0("Median (",f(median),")"),cex=textsize,col="blue",pos=1,offset=0.25)
  abline(v=perc95,col="red")
  text(x=perc95,y=0,paste0("95th percentile (",f(perc95),")"),cex=textsize,col="red",pos=1,offset=0.25)
  # Shut down graphics device, saves graphic to file
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
