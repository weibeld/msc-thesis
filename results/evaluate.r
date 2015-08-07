
GetData <- function() {
  SetDir()

  # Test set analysis
  cmpl <<- ReadSingle("testset/completeness/completeness.csv")
  univ <<- ReadSingle("testset/universality/universality.csv")
  empt <<- ReadSingle("testset/emptiness/emptiness.csv")
  
  # Internal: GOAL test set
  i.g.all <<- ReadSubdirs("internal/goal")[c(1, 8, 7, 2, 5, 4, 3, 6)]
  i.g <<- Eff(i.g.all)

  # Internal: Michel automata
  i.m <<- ReadSubdirs("internal/michel")[c(4, 6, 3, 1, 2, 5)]

  # External: GOAL test set with Rank
  e.g.with_rank.all <<- ReadSubdirs("external/goal")[c(2, 4, 3, 1)]
  e.g.with_rank <<- Eff(e.g.with_rank.all)

  # External: GOAL test set without Rank
  e.g.all <<- e.g.with_rank.all
  e.g.all[3] <<- NULL  # Delete third element which is Rank
  e.g <<- Eff(e.g.all)

  # External: Michel automata
  e.m <<- ReadSubdirs("external/michel")[c(2, 4, 3, 1)]
}

All <- function() {
  GoalTestSet()
  IntGoal()
  IntMichel()
  ExtGoal()
  ExtMichel()
}

GoalTestSet <- function() {
  dir <- .MkDir("testset")
  
  # Completeness
  m <- MatrixTestset(cmpl)
  .GoalTestSetTable(m, paste0(dir, "/compl.table.tex"))
  .GoalTestSetPersp(m, paste0(dir, "/compl.persp.pdf"))

  # Universality
  m <- MatrixTestset(univ)
  .GoalTestSetTable(m, paste0(dir, "/univ.table.tex"))
  .GoalTestSetPersp(m, paste0(dir, "/univ.persp.pdf"))

  # Emptiness
  m <- MatrixTestset(empt)
  .GoalTestSetTable(m, paste0(dir, "/empt.table.tex"))
  .GoalTestSetPersp(m, paste0(dir, "/empt.persp.pdf"))
}

.GoalTestSetTable <- function(m, file) {
  options(warn=-1)  # Disable warnings (because of custom column type 'R')
  LatexTable(m, format="d", include.rownames=TRUE, align="r|RRRRRRRRRR",
             hline.after=0, file=file)
  options(warn=0)  # Re-enable warnings
}

.GoalTestSetPersp <- function(m, file) {
  pdf(file=file, width=6, height=4.5)
  par(mar=c(0.125, 2, 0, 0.5))
  # Draw persp plot. Note: cex.axis applies only to tick marks
  Persp(m, zlim=c(0, 100), theta=-44, phi=15, lin=TRUE, lin.xy=list(x=3, y=1),
        lin.col="gray", col="lightskyblue", shade=0.6, ltheta=-90, lphi=30, zlab="",
        xlab="Transition density", ylab="Acceptance density", cex.axis=0.95)
  text(-0.75, 0.03, "Number of automata", srt=92, xpd=TRUE)
  dev.off()
}

IntGoal <- function() {
  dir <- .MkDir("internal/goal")

  # Table with timeouts and memory outs
  file <- paste0(dir, "/outs.tex")
  df <- Outs(i.g.all)
  LatexTable(df, format="d", file=file)

  # Table with statistics of states
  file <- paste0(dir, "/s.stats.tex")
  df <- Stats(i.g)
  LatexTable(df, format=c("", "f", "d", "f", "f", "f", "d"), file=file)

  # Table with statistics of times
  file <- paste0(dir, "/t.stats.tex")
  stats <- Stats(i.g, dat="cpu_time", total=TRUE, total.hours=TRUE)
  format <- c("", "f", "f", "f", "f", "f", "f", "f", "d")
  align  <- c("rLrrrrrrRr")
  options(warn=-1)  # Disable warnings (because of custom column type 'R', 'L')
  LatexTable(stats, format=format, align=align, digits=1, file=file)
  options(warn=0)  # Re-enable warnings

  # Strip chart of states (including decimation to make PDF lighter)
  file <- paste0(dir, "/s.stripchart.pdf")
  # Prepare a "light" data set by decimating the dense data points
  i.g.light <- .Decimate(i.g, 1000, 0.8)
  i.g.light <- .Decimate(i.g.light, 1500, 0.5)
  i.g.light <- .Decimate(i.g.light, 2500, 0.25)
  i.g.light <- .Decimate(i.g.light, 5000, 0.1)
  pdf(file=file, width=8, height=5)
  par(mar=c(3.5, 9.25, 0.5, 0.5))
  Stripchart(i.g.light, xlim=c(0, 60000), jitter=0.275)
  text(x=30000, y=-1, labels="Complement size", xpd=TRUE)
  dev.off()

  # Strip chart of times (including decimation)
  file <- paste0(dir, "/t.stripchart.pdf")
  i.g.light <- .Decimate(i.g, 10, 0.5, dat="cpu_time")
  i.g.light <- .Decimate(i.g.light, 25, 0.5, dat="cpu_time")
  pdf(file=file, width=8, height=5)
  par(mar=c(3.5, 9.25, 0.5, 0.5))
  Stripchart(i.g.light, dat="cpu_time", xlim=c(0, 600), jitter=0.275)
  text(x=300, y=-1, labels="Execution time (seconds)", xpd=TRUE)
  dev.off()

  # Persp plots with median of produced states
  mlist <- MatrixGoal(i.g)
  PerspBatch(mlist, pdf=TRUE, dir=dir, prefix="s.median.", width=6, height=5,
             mar=c(1, 2.5, 0, 0), zlim=c(0, 6500), z.colors=TRUE, zlab="",
             xlab="Transition density", ylab="Acceptance density", lin=TRUE,
             lin.xy=list(x=1, y=0.1), lin.col="gray", theta=150, phi=25,
             shade=0.5, ltheta=30, lphi=20, custom.zlab=function() {
             text(-0.73, 0.06, "States (median)", srt=94) })

  # Image plot showing difficulty levels for all the dt/da classes
  file <- paste0(dir, "/difficulty.pdf")
  m.avg <- MatrixAgg(mlist, mean)  # Average of all the state median matrices
  pdf(file=file, width=4.1, height=4.1)
  par(mar=c(0.25, 3, 2.75, 0.25))
  # less than 500: green; betw. 500 and 1600: yellow; more than 1600: red
  Image(m.avg, breaks=c(0, 500, 1600, 6000), col=c("green", "yellow", "red"))
  dev.off()

  # Matrices corresponding to all the persp plots (in separate folder)
  dir <- .MkDir("internal/goal/matrices")
  m.lst <- MatrixGoal(i.g)
  .Matrices(m.lst, dir)
}

ExtGoal <- function() {
  dir <- .MkDir("external/goal")
  
  # Table with timeouts and memory outs (with Rank)
  file <- paste0(dir, "/outs.tex")
  df <- Outs(e.g.with_rank.all)
  LatexTable(df, format="d", file=file)

  # Table with statistics of states (with Rank -> 7,204 effective samples)
  file <- paste0(dir, "/s.stats.with_rank.tex")
  df <- Stats(e.g.with_rank)
  LatexTable(df, format=c("", "f", "d", "f", "f", "f", "d"), file=file)

  # Table with statistics of states (without Rank -> 10,998 effective samples)
  file <- paste0(dir, "/s.stats.tex")
  df <- Stats(e.g)
  LatexTable(df, format=c("", "f", "d", "f", "f", "f", "d"), file=file)

  # Table with statistics of times (with Rank -> 7,204 effective samples)
  file <- paste0(dir, "/t.stats.with_rank.tex")
  stats <- Stats(e.g.with_rank, dat="cpu_time", total=TRUE, total.hours=TRUE)
  format <- c("", "f", "f", "f", "f", "f", "f", "f", "d")
  align  <- c("rLrrrrrrRr")
  options(warn=-1)  # Disable warnings (because of custom column type 'R', 'L')
  LatexTable(stats, format=format, align=align, digits=1, file=file)
  options(warn=0)  # Re-enable warnings

  # Table with statistics of times (without Rank -> 10,998 effective samples)
  file <- paste0(dir, "/t.stats.tex")
  stats <- Stats(e.g, dat="cpu_time", total=TRUE, total.hours=TRUE)
  format <- c("", "f", "f", "f", "f", "f", "f", "f", "d")
  align  <- c("rLrrrrrrRr")
  options(warn=-1)  # Disable warnings (because of custom column types 'R', 'L')
  LatexTable(stats, format=format, align=align, file=file, digits=1)
  options(warn=0)  # Re-enable warnings

  # Stripchart of complement sizes (with Rank -> 7,204 effective samples)
  file <- paste0(dir, "/s.stripchart.with_rank.pdf")
  e.g.with_rank.light <- .Decimate(e.g.with_rank, 1000, 0.8)
  pdf(file=file, width=8, height=4)
  par(mar=c(3.5, 10.25, 0.5, 0.5))
  Stripchart(e.g.with_rank.light, jitter=0.275)
  text(x=60000, y=-0.45, labels="Complement size", xpd=TRUE)
  dev.off()

  # Stripchart of complement sizes (without Rank -> 10,998 effective samples)
  file <- paste0(dir, "/s.stripchart.pdf")
  e.g.light <- .Decimate(e.g, 500, 0.8)
  e.g.light <- .Decimate(e.g.light, 2000, 0.3)
  e.g.light <- .Decimate(e.g.light, 3000, 0.1)
  pdf(file=file, width=8, height=3.1125)
  par(mar=c(3.5, 10.25, 0.5, 0.5))
  Stripchart(e.g.light, jitter=0.275)
  text(x=20000, y=-0.45, labels="Complement size", xpd=TRUE)
  dev.off()

  # Stripchart of times (with Rank -> 7,204 effective samples)
  file <- paste0(dir, "/t.stripchart.with_rank.pdf")
  e.g.with_rank.light <- .Decimate(e.g.with_rank, 10, 0.5, dat="cpu_time")
  pdf(file=file, width=8, height=4)
  par(mar=c(3.5, 10.25, 0.5, 0.5))
  Stripchart(e.g.with_rank.light, dat="cpu_time", xlim=c(0, 450), jitter=0.275)
  text(x=225, y=-0.45, labels="Execution time (seconds)", xpd=TRUE)
  dev.off()

  # Stripchart of times (without Rank -> 10,998 effective samples)
  file <- paste0(dir, "/t.stripchart.pdf")
  e.g.light <- .Decimate(e.g, 5, 0.5, dat="cpu_time")
  e.g.light <- .Decimate(e.g.light, 10, 0.25, dat="cpu_time")
  pdf(file=file, width=8, height=3.1125)
  par(mar=c(3.5, 10.25, 0.5, 0.5))
  Stripchart(e.g.light, dat="cpu_time", xlim=c(0, 450), jitter=0.275)
  text(x=225, y=-0.45, labels="Execution time (seconds)", xpd=TRUE)
  dev.off()

  # Persp plots with median of produced states (without Rank)
  mlist <- MatrixGoal(e.g)
  PerspBatch(mlist, pdf=TRUE, dir=dir, prefix="s.median.", width=6, height=5,
             mar=c(1, 2.5, 0, 0), zlim=c(0, 3000), z.colors=TRUE, zlab="",
             xlab="Transition density", ylab="Acceptance density", lin=TRUE,
             lin.xy=list(x=1, y=0.1), lin.col="gray", theta=150, phi=25,
             shade=0.5, ltheta=30, lphi=20, custom.zlab=function() {
             text(-0.73, 0.06, "States (median)", srt=94) })

  # Matrices corresponding to the persp plots (without Rank)
  dir <- .MkDir("external/goal/matrices")
  m.lst <- MatrixGoal(e.g)
  # Remove Fribourg, because we have it already in the internal tests
  m.lst$`Fribourg+M1+R2C` <- NULL
  .Matrices(m.lst, dir)
}

IntMichel <- function() {
  dir <- .MkDir("internal/michel")

  # # Table with complement sizes
  # file <- paste0(dir, "/s.table.tex")
  # df <- MichelTable(i.m)
  # LatexTable(df, format="d", align="rlrrrrrr", file=file)

  # # Table with execution times
  # file <- paste0(dir, "/t.table.tex")
  # df <- MichelTable(i.m, dat="cpu_time")
  # LatexTable(df, align="rlrrrrrr", file=file)

  # # Barplot with complement sizes
  # file <- paste0(dir, "/s.barplot.pdf")
  # pdf(file=file, width=7, height=5)
  # par(mar=c(7.5, 6, 1, 0.125))
  # MichelBarplot(i.m, ylim=c(0, 300000), col="white", lin=TRUE)
  # text(x=-5.5, y=150000, labels="Complement size", srt=90, xpd=TRUE)
  # dev.off()

  # # Barplot with execution times
  # file <- paste0(dir, "/t.barplot.pdf")
  # pdf(file=file, width=7, height=5)
  # par(mar=c(7.5, 6, 1, 0.125))
  # MichelBarplot(i.m, dat="cpu_time", col="white", lin=TRUE)
  # text(x=-5.5, y=50000, labels="Running time (sec.)", srt=90, xpd=TRUE)
  # dev.off()

  # Extrapolation table
  secyear <- 31557600  # Seconds per year: 60 * 60 * 24 * 365.25
  secday  <- 86400     # Seconds per day:  60 * 60 * 24
  # Coeffcients a of the fitted functions (an)^n
  coef.1.states <- 1.35433834768346   # Fribourg (complement size)
  coef.1.time   <- 1.13678482617192   # Fribourg (execution time)
  coef.2.states <- 0.990158872019168  # Fribourg+M1+M2+R2C (complement size)
  coef.2.time   <- 0.605960897580839  # Fribourg+M1+M2+R2C (execution time)
  x <- 7:10
  y.1.states <- sapply(x, function(n) { (coef.1.states*n)^n })
  y.1.time   <- sapply(x, function(n) { (coef.1.time*n)^n })
  y.2.states <- sapply(x, function(n) { (coef.2.states*n)^n })
  y.2.time   <- sapply(x, function(n) { (coef.2.time*n)^n })
  df <- data.frame(`States $(1.35n)^n$`=y.1.states,
             `Time $(1.14n)^n$`=y.1.time,
             `$\approx$ days/years`=ifelse(y.1.time/secyear < 1.5, paste0(round(y.1.time/secday), " days"), paste0(round(y.1.time/secyear), " years")),
             `States $(0.99n)^n$`=y.2.states,
             `Time $(0.61n)^n$`=y.2.time,
             `$\approx$ days/years`=ifelse(y.2.time/secyear < 1.5, paste0(round(y.2.time/secday), " days"), paste0(round(y.2.time/secyear), " years")),
             check.names=FALSE)
  rownames(df) <- paste0("Michel ", x)
  df 
}


ExtMichel <- function() {
  dir <- .MkDir("external/michel")

  # Table with the complement sizes
  file <- paste0(dir, "/s.table.tex")
  df <- MichelTable(e.m)
  LatexTable(df, format="d", align="rlrrrrrr", file=file)

  # Table with the execution times
  file <- paste0(dir, "/t.table.tex")
  df <- MichelTable(e.m, dat="cpu_time")
  LatexTable(df, align="rlrrrrrr", file=file)

  # Barplot with complement sizes
  file <- paste0(dir, "/s.barplot.pdf")
  pdf(file=file, width=7, height=5)
  par(mar=c(8, 6, 1, 0.125))
  MichelBarplot(e.m, ylim=c(0, 180000), yaxp=c(0, 180000, 9), col="white", lin=TRUE)
  text(x=-3.25  , y=100000, labels="Complement size", srt=90, xpd=TRUE)
  dev.off()

  # Barplot with execution times
  file <- paste0(dir, "/t.barplot.pdf")
  pdf(file=file, width=7, height=5)
  par(mar=c(8, 6, 1, 0.125))
  MichelBarplot(e.m, dat="cpu_time", ylim=c(0, 80000), col="white", lin=TRUE)
  text(x=-3.25, y=40000, labels="Running time (sec.)", srt=90, xpd=TRUE)
  dev.off()            
}


.Matrices <- function(m.lst, dir) {
  # Print the matrices of the persp plots as LaTeX tables for the appendix
  #----------------------------------------------------------------------------#
  i <- 1
  for (m in m.lst) {
    file <- paste0(dir, "/s.median.", names(m.lst)[i], ".tex")
    options(warn=-1)  # Disable warnings (because of custom column type 'R')
    # 'R' is a custom column type defined in the LaTeX document
    LatexTable(m, format="d", include.rownames=TRUE, align="r|RRRRRRRRRR",
               hline.after=0, file=file)
    options(warn=0)  # Re-enable warnings
    i <- i + 1
  }
}


.Decimate <- function(list, threshold, rm.perc, dat="states") {
  # Remove 'rm.perc' percent of the rows with less than 'threshold' states from
  # the data frames in 'list'
  #----------------------------------------------------------------------------#
  lapply(list, function(df) {
    rows <- rownames(df[df[[dat]] > threshold,])  # Rows to keep
    x <- rownames(df[df[[dat]] <= threshold,])    # Candidate rows to remove
    rows <- c(rows, sample(x, size=round((1 - rm.perc) * length(x))))
    df[rows,]
  })
}

.BaseDir <- function() { "~/Desktop/thesis/report/figures/r" }

.MkDir <- function(dir) {
  dir <- paste0(.BaseDir(), "/", dir)
  dir.create(dir, recursive=TRUE, showWarnings=FALSE)
  dir
}
