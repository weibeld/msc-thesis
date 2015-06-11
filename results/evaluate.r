
GetData <- function() {
  SetDir()

  # Test set analysis
  cmpl <<- ReadSingle("completeness/completeness.csv")
  univ <<- ReadSingle("universality/universality.csv")
  
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
  Completeness()
  Universality()
  IntGoal()
  IntMichel()
  ExtGoal()
  ExtMichel()
}

Completeness <- function() {
  dir <- .MkDir("completeness")
  m <- MatrixTestset(cmpl)
  # LaTeX table
  file <- paste0(dir, "/table.tex")
  LatexTable(m, format="d", include.rownames=TRUE, align="r|rrrrrrrrrr", file=file)
  # Persp plot
  file <- paste0(dir, "/persp.pdf")
  .ComplUnivPersp(m, file)
}

Universality <- function() {
  dir <- .MkDir("universality")
  m <- MatrixTestset(univ)
  # LaTeX table
  file <- paste0(dir, "/table.tex")
  LatexTable(m, format="d", include.rownames=TRUE, align="r|rrrrrrrrrr", file=file)
  # Persp plot
  file <- paste0(dir, "/persp.pdf")
  .ComplUnivPersp(m, file)
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
  LatexTable(Michel(i.m), format="d", file=file)

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
  LatexTable(Michel(e.m), format="d", file=file)

  # Table with the execution times
  file <- paste0(dir, "/t.table.tex")
  LatexTable(Michel(e.m), dat="cpu_time", format="d", file=file)

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


.ComplUnivPersp <- function(m, file) {
  # Draw persp plot for the number of complete and universal automata in the
  # GOAL test set
  #----------------------------------------------------------------------------#
  pdf(file=file, width=6, height=4.5)
  par(mar=c(1, 3, 0, 0.5))
  # Draw persp plot. Note: cex.axis applies only to tick marks
  Persp(m, zlim=c(0, 100), theta=-44, phi=25, lin=TRUE, lin.xy=list(x=3, y=1),
        lin.col="gray", col="deepskyblue", shade=0.6, ltheta=-90, lphi=30, zlab="",
        xlab="Transition density", ylab="Acceptance density", cex.axis=0.95)
  text(-0.75, 0.03, "Number of automata", srt=92, xpd=TRUE)
  dev.off()
}

.BaseDir <- function() { "~/Desktop/thesis/report/figures/r" }

.MkDir <- function(dir) {
  dir <- paste0(.BaseDir(), "/", dir)
  dir.create(dir, recursive=TRUE, showWarnings=FALSE)
  dir
}



# # Test set analysis
# cmpl <- ReadSingle("completeness/completeness.csv")
# cmpl.m <- MatrixTestset(cmpl)

# univ <- ReadSingle("universality/universality.csv")
# univ.m <- MatrixTestset(univ)

# #------------------------------------------------------------------------------#
# # Internal tests
# #------------------------------------------------------------------------------#

# # GOAL automata
# #-------------#

# # Read list of data frames and put list elements in desired order
# i.g.all <- ReadSubdirs("internal/goal")[c(1, 8, 7, 2, 3, 5, 4, 6)]

# # Get effective samples
# i.g <- Eff(i.g.all)

# # States: aggregated statistics
# i.g.s1 <- Stats(i.g)

# # States: dt/da classes statistics
# i.g.s2 <- StatsGoal(i.g)

# # States: dt/da classes median matrix
# i.g.s2.m <- MatrixGoal(i.g, stat="median")

# # Time: aggregated statistics
# i.g.t1 <- Stats(i.g, dat="cpu_time")

# # Time: dt/da classes statistics
# i.g.t2 <- StatsGoal(i.g, dat="cpu_time")

# # Time: dt/da classes median matrix
# i.g.t2.m <- MatrixGoal(i.g, dat="cpu_time", stat="median")


# # Michel automata
# #---------------#

# # Read list of data frames and put list elements in desired order
# i.m <- ReadSubdirs("internal/michel")[c(6, 7, 4, 1, 5, 2, 3)]

# # States
# i.m.s <- Michel(i.m)

# # Time
# i.m.t <- Michel(i.m, dat="cpu_time")


# #------------------------------------------------------------------------------#
# # External tests
# #------------------------------------------------------------------------------#

# # GOAL automata
# #-------------#

# # Without Rank

# # Read list of data frames
# e.g.all <- ReadSubdirs("external/goal")

# # Get effective samples
# e.g <- Eff(e.g.all)

# # States: aggregated statistics
# e.g.s1 <- Stats(e.g)

# # States: dt/da classes statistics
# e.g.s2 <- StatsGoal(e.g)

# # States: dt/da classes median matrix
# e.g.s2.m <- MatrixGoal(e.g, stat="median")

# # Time: aggregated statistics
# e.g.t1 <- Stats(e.g, dat="cpu_time")

# # Time: dt/da classes statistics
# e.g.t2 <- StatsGoal(e.g, dat="cpu_time")

# # Time: dt/da classes median matrix
# e.g.t2.m <- MatrixGoal(e.g, dat="cpu_time", stat="median")


# # With Rank

# # Add the rank data frame to the list of previously read data frames
# e.g.rank.all <- e.g.all
# e.g.rank.all$rank.tr.ro.goal <- ReadSingle("external/goal/rank/rank.tr.ro.goal/rank.tr.ro.goal.csv")

# # Get effective samples
# e.g.rank <- Eff(e.g.rank.all)

# # States: aggregated statistics
# e.g.rank.s1 <- Stats(e.g.rank)

# # States: dt/da classes statistics
# e.g.rank.s2 <- StatsGoal(e.g.rank)

# # States: dt/da classes median matrix
# e.g.rank.s2.m <- MatrixGoal(e.g.rank, stat="median")

# # Time: aggregated statistics
# e.g.rank.t1 <- Stats(e.g.rank, dat="cpu_time")

# # Time: dt/da classes statistics
# e.g.rank.t2 <- StatsGoal(e.g.rank, dat="cpu_time")

# # Time: dt/da classes median matrix
# e.g.rank.t2.m <- MatrixGoal(e.g.rank, dat="cpu_time", stat="median")


# # Michel automata
# #---------------#

# e.m <- ReadSubdirs("external/michel")

# # States
# e.m.s <- Michel(e.m)

# # Times
# e.m.t <- Michel(e.m, dat="cpu_time")
