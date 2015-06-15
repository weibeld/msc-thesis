
GetData <- function() {
  SetDir()

  # Test set analysis
  cmpl <<- ReadSingle("completeness/completeness.csv")
  univ <<- ReadSingle("universality/universality.csv")
  empt <<- ReadSingle("emptiness/emptiness.csv")
  
  # Internal: GOAL test set
  i.g.all <<- ReadSubdirs("internal/goal")[c(1, 8, 7, 2, 3, 5, 4, 6)]
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
  Appendices()
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

  # LaTeX table with timeouts and memory outs
  file <- paste0(dir, "/outs.tex")
  df <- Outs(i.g.all)
  LatexTable(df, format="d", file=file)

  # LaTeX table with aggregated statistics of states (effective samples)
  file <- paste0(dir, "/s.stats.tex")
  df <- Stats(i.g)
  LatexTable(df, format=c("", "f", "d", "f", "f", "f", "d"), file=file)

  # LaTeX table with aggregated statistics of times (effective samples)
  file <- paste0(dir, "/t.stats.tex")
  df <- Stats(i.g, dat="cpu_time")
  LatexTable(df, format="f", digits=2, file=file)

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
  text(x=30000, y=-1, labels="Complement size (states)", xpd=TRUE)
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
}

IntMichel <- function() {
  dir <- .MkDir("internal/michel")

  # Table with the complement sizes
  file <- paste0(dir, "/s.table.tex")
  LatexTable(MichelTable(i.m), format="d", align="rlrrrrrr", file=file)

  # Barplot with complement sizes
  file <- paste0(dir, "/s.barplot.pdf")
  pdf(file=file, width=7, height=5)
  par(mar=c(7.5, 6, 1, 0.125))
  MichelBarplot(i.m, ylim=c(0, 300000), col="white", lin=TRUE)
  text(x=-6.75, y=150000, labels="Complement size", srt=90, xpd=TRUE)
  dev.off()

  # Barplot with execution times
  file <- paste0(dir, "/t.barplot.pdf")
  pdf(file=file, width=7, height=5)
  par(mar=c(7.5, 6, 1, 0.125))
  MichelBarplot(i.m, dat="cpu_time", col="white", lin=TRUE)
  text(x=-6.75, y=50000, labels="Running time (sec.)", srt=90, xpd=TRUE)
  dev.off()
}

ExtGoal <- function() {
dir <- .MkDir("external/goal")
  
  # LaTeX table with timeouts and memory outs (with Rank)
  file <- paste0(dir, "/outs.tex")
  df <- Outs(e.g.with_rank.all)
  LatexTable(df, format="d", file=file)
  
  # Stripchart with complement sizes (effective samples with Rank)
  file <- paste0(dir, "/s.stripchart.with_rank.pdf")
  e.g.with_rank.light <- .Decimate(e.g.with_rank, 1000, 0.8)
  pdf(file=file, width=8, height=5)
  par(mar=c(2.25, 10.25, 0.5, 0.5))
  Stripchart(e.g.with_rank.light)
  dev.off()

  # LaTeX table with aggregated stats of states (effective samples with Rank)
  file <- paste0(dir, "/s.stats.with_rank.tex")
  df <- Stats(e.g.with_rank)
  LatexTable(df, format=c("", "f", "d", "f", "f", "f", "d"), file=file)

  # LaTeX table with aggregated stats of times (effective samples with Rank)
  file <- paste0(dir, "/t.stats.with_rank.tex")
  df <- Stats(e.g.with_rank, dat="cpu_time")
  LatexTable(df, format="f", digits=2, file=file)

  # Stripchart with complement sizes (effective samples without Rank)
  file <- paste0(dir, "/s.stripchart.pdf")
  e.g.light <- .Decimate(e.g, 500, 0.8)
  e.g.light <- .Decimate(e.g.light, 2000, 0.3)
  e.g.light <- .Decimate(e.g.light, 3000, 0.1)
  pdf(file=file, width=8, height=5)
  par(mar=c(2.25, 10.25, 0.5, 0.5))
  Stripchart(e.g.light)
  dev.off()
  
  # LaTeX table with aggregated stats of states (effective samples without Rank)
  file <- paste0(dir, "/s.stats.tex")
  df <- Stats(e.g)
  LatexTable(df, format=c("", "f", "d", "f", "f", "f", "d"), file=file)

  # LaTeX table with aggregated stats of times (effective samples without Rank)
  file <- paste0(dir, "/t.stats.tex")
  df <- Stats(e.g, dat="cpu_time")
  LatexTable(df, format="f", digits=2, file=file)

  # Persp plots with median of produced states
  mlist <- MatrixGoal(e.g)
  PerspBatch(mlist, pdf=TRUE, dir=dir, prefix="s.median.", width=6, height=5,
             mar=c(1, 2.5, 0, 0), zlim=c(0, 3000), z.colors=TRUE, zlab="",
             xlab="Transition density", ylab="Acceptance density", lin=TRUE,
             lin.xy=list(x=1, y=0.1), lin.col="gray", theta=150, phi=25,
             shade=0.5, ltheta=30, lphi=20, custom.zlab=function() {
             text(-0.73, 0.06, "States (median)", srt=94) })
}


ExtMichel <- function() {
  dir <- .MkDir("external/michel")

  # Table with the complement sizes
  file <- paste0(dir, "/s.table.tex")
  LatexTable(MichelTable(e.m), format="d", align="rlrrrrrr", file=file)

  # Table with the execution times
  file <- paste0(dir, "/t.table.tex")
  LatexTable(MichelTable(e.m), dat="cpu_time", align="rlrrrrrr", file=file)

  # Barplot with complement sizes
  file <- paste0(dir, "/s.barplot.pdf")
  pdf(file=file, width=7, height=5)
  par(mar=c(8, 6, 1, 0.125))
  MichelBarplot(e.m, ylim=c(0, 180000), yaxp=c(0, 180000, 9), col="white", lin=TRUE)
  text(x=-3.5, y=100000, labels="Complement size", srt=90, xpd=TRUE)
  dev.off()

  # No barplot for execution times, because the value for Piterman (Michel 4)
  # is so high that the other values cannot be distinguished anymore
}

Appendices <- function() {
  dir <- .MkDir("appendices")

  # Matrices of median complement sizes of internal tests
  m.lst <- MatrixGoal(i.g)
  .AppendixMatrices(m.lst, dir)

  # Matrices of median complement sizes of external tests (without Rank)
  m.lst <- MatrixGoal(e.g)
  # Remove Fribourg, because we have it already in the internal tests
  m.lst$`Fribourg+M1+R2C` <- NULL
  .AppendixMatrices(m.lst, dir)

  # CPU time stats for GOAL test set (internal)
  file <- paste0(dir, "/i.g.t.stats.tex")
  stats <- Stats(i.g, dat="cpu_time", total=TRUE, total.hours=TRUE)
  format <- c("", "f", "f", "f", "f", "f", "f", "f", "d")
  LatexTable(stats, format=format, file=file, digits=1)

  # CPU time stats for GOAL test set (external with Rank -> 7,204 eff. samples)
  file <- paste0(dir, "/e.g.t.stats.with_rank.tex")
  stats <- Stats(e.g.with_rank, dat="cpu_time", total=TRUE, total.hours=TRUE)
  format <- c("", "f", "f", "f", "f", "f", "f", "f", "d")
  LatexTable(stats, format=format, file=file, digits=1)

  # CPU time stats for GOAL test set (external without Rank -> 10,998 eff. samples)
  file <- paste0(dir, "/e.g.t.stats.without_rank.tex")
  stats <- Stats(e.g, dat="cpu_time", total=TRUE, total.hours=TRUE)
  format <- c("", "f", "f", "f", "f", "f", "f", "f", "d")
  LatexTable(stats, format=format, file=file, digits=1)

  # CPU times for Michel test set (internal)
  file <- paste0(dir, "/i.m.t.tex")
  LatexTable(MichelTable(i.m, dat="cpu_time"), align="rlrrrrrr", file=file)

  # CPU times for Michel test set (external)
  file <- paste0(dir, "/e.m.t.tex")
  LatexTable(MichelTable(e.m, dat="cpu_time"), align="rlrrrrrr", file=file)
}

.AppendixMatrices <- function(m.lst, dir) {
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


.Decimate <- function(list, threshold, rm.perc) {
  # Remove 'rm.perc' percent of the rows with less than 'threshold' states from
  # the data frames in 'list'
  #----------------------------------------------------------------------------#
  lapply(list, function(df) {
    rows <- rownames(df[df$states > threshold,])  # Rows to keep
    x <- rownames(df[df$states <= threshold,])    # Candidate rows to remove
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
