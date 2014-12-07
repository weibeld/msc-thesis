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

stripchart.states <- function(df,...,labels=NULL,min=5000,lmargin=6,xaxp=NULL,width=4,height=4,
  file="~/Desktop/stripchart.pdf") {
  # Start graphics device and set graphics params
  pdf(file,width=width,height=height,pointsize=8)
  global.params()
  
  #par(ps=5)
  #par(lwd=0.5)
  # Increase left margin
  m <- par("mar")
  m[2] <- lmargin
  par(mar=m) 
  # Prepare data
  vectors <- list()
  if (class(df) != "list") df <- list(df)
  for (frame in df)
    vectors[[length(vectors)+1]] <- frame[frame$states>=min,"states"]
  # Draw stripchart
  par(xaxs="r")
  if (is.null(xaxp)) xaxt <- NULL else xaxt <- "n"
  stripchart(vectors,group.names=labels,method="jitter",pch=1,las=1,lwd=par("lwd"),xaxt=xaxt,...)
  if (!is.null(xaxp)) {
    ticks <- axTicks(1,xaxp)
    tick.labels <- formatC(ticks,format="d",big.mark=" ")
    axis(1,at=ticks,labels=tick.labels,lwd=par("lwd"))
  }
  # grid(lty="solid",col="black",ny=NA)
  #abline(v=c(10000,25000,45000))
  # axis(side=2,at=1:4,labels=labels)
  # Close graphics device (saves graphic to file)
  dev.off()
}

# Create boxplots of multiple experiments (number of states of complements)
# in a single chart.
boxplot.states <- function(...,labels=NULL,ymax=NULL,width=7,height=7,
    file="~/Desktop/boxplot.pdf") {
  # Start graphics device and set graphics params
  pdf(file,width=width,height=height)
  global.params()
  # Prepare data
  vectors <- list()
  for (frame in list(...)) vectors[[length(vectors)+1]] <- frame$states
  if (!is.null(ymax)) ymax <- c(0,ymax)
  # Draw boxplot
  boxplot(vectors,outline=FALSE,names=labels,ylim=ymax,ylab="Complement size")
  # Add additional elements
  text <- character()
  text.pos <- numeric()
  means <- numeric()
  for (vector in vectors) {
    info <- boxplot.stats(vector)
    # Parameters of this boxplot
    outliers.number <- length(info$out)
    outliers.percent <- 100*outliers.number/info$n
    outliers.max <- max(info$out)
    upper.whisker <- info$stats[5]
    # Add to structure for all boxplots
    text <- c(text, paste0(outliers.number, " outliers (",f(outliers.percent),"%)\nMax. ",i(outliers.max)))
    text.pos <- c(text.pos, upper.whisker)
    means <- c(means,mean(vector,na.rm=TRUE))
  }
  text(text.pos,text,pos=3,cex=0.675) # pos=3 == above
  points(means,pch=19)
  # Close graphics device (saves graphic to file)
  dev.off()
}

# Create histogram of number of states of complement automata of an experiment.
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

# Set global graphics parameters that apply to all graphics.
global.params <- function() {
  par(mar=c(4,4,1,1),bty="o",mgp=c(2.4,0.9,0),mex=1,tcl=-0.5,lwd=0.75,bg="green")
}

# Format a floating point number for printing. Returns a string.
f <- function(n,dec=1,mark=",") {
  if (n < 10000) mark <- "" # Thousands marks only for numbers >= 10'000
  formatC(n,format="f",digits=dec,big.mark=mark)
}

# Format an integer for printing. Returns a string.
i <- function(n,mark="'") {
  if (n < 10000) mark <- "" # Thousands marks only for numbers >= 10'000
  formatC(n,format="d",big.mark=mark)
}
