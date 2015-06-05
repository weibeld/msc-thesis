
GetData <- function() {
  SetDir()

  # Test set analysis
  cmpl <<- ReadSingle("completeness/completeness.csv")
  univ <<- ReadSingle("universality/universality.csv")
  
  # Internal: GOAL test set
  i.g.all <<- ReadSubdirs("internal/goal")[c(1, 8, 7, 2, 3, 5, 4, 6)]
  i.g <<- Eff(i.g.all)

  # Internal: Michel automata
  i.m <<- ReadSubdirs("internal/michel")[c(6, 7, 4, 1, 5, 2, 3)]

  # External without Rank: GOAL test set
  e.g.all <<- ReadSubdirs("external/goal")
  e.g <<- Eff(e.g.all)

  # External with Rank: GOAL test set
  e.g.rank.all <<- e.g.all
  e.g.rank.all$`Rank+TR+RO` <<- ReadSingle("external/goal/rank/rank.tr.ro.goal/rank.tr.ro.goal.csv")
  e.g.rank <<- Eff(e.g.rank.all)

  # External: Michel automata
  e.m <<- ReadSubdirs("external/michel")
}

Completeness <- function() {
  dir <- .MkDir("completeness")
  m <- MatrixTestset(cmpl)
  # LaTeX table
  file <- paste0(dir, "/table.tex")
  LatexTable(m, format="d", align="r|rrrrrrrrrr", file=file)
  # Persp plot
  file <- paste0(dir, "/persp.pdf")
  .ComplUnivPersp(m, file)
}

Universality <- function() {
  dir <- .MkDir("universality")
  m <- MatrixTestset(univ)
  # LaTeX table
  file <- paste0(dir, "/table.tex")
  LatexTable(m, format="d", align="r|rrrrrrrrrr", file=file)
  # Persp plot
  file <- paste0(dir, "/persp.pdf")
  .ComplUnivPersp(m, file)
}

IntGoal <- function() {
  dir <- .MkDir("internal/goal")

  # LaTeX table with timeouts and memory outs
  file <- paste0(dir, "/outs.tex")
  df <- Outs(i.g.all)
  LatexTable(df, include.rownames=FALSE, format="d", file=file)

  # LaTeX table with aggregated statistics of states
  file <- paste0(dir, "/stats.states.tex")
  df <- Stats(i.g)
  LatexTable(df, include.rownames=FALSE, format=c("", "f", "d", "f", "f", "f", "d"), file=file)

  # LaTeX table with aggregated statistics of times
  file <- paste0(dir, "/stats.time.tex")
  df <- Stats(i.g, dat="cpu_time")
  LatexTable(df, include.rownames=FALSE, format="f", digits=2, file=file)

  # Strip chart of states (including decimation to make PDF lighter)
  file <- paste0(dir, "/stripchart.pdf")
  # Prepare a data set with a good part of the rows with few states removed
  i.g.light <- lapply(i.g, function(df) {
    # Remove 'rm.perc' percent of the rows with less than 'threshold' states
    threshold <- 1000; rm.perc <- 0.8
    rows <- rownames(df[df$states > threshold,])  # Rows to keep
    x <- rownames(df[df$states <= threshold,])    # Candidate rows to remove
    rows <- c(rows, sample(x, size=round((1 - rm.perc) * length(x))))
    df[rows,]
  })
  i.g.light <- lapply(i.g.light, function(df) {
    threshold <- 1500; rm.perc <- 0.5
    rows <- rownames(df[df$states > threshold,])  # Rows to keep
    x <- rownames(df[df$states <= threshold,])    # Candidate rows to remove
    rows <- c(rows, sample(x, size=round((1 - rm.perc) * length(x))))
    df[rows,]
  })
  i.g.light <- lapply(i.g.light, function(df) {
    threshold <- 2500; rm.perc <- 0.25
    rows <- rownames(df[df$states > threshold,])  # Rows to keep
    x <- rownames(df[df$states <= threshold,])    # Candidate rows to remove
    rows <- c(rows, sample(x, size=round((1 - rm.perc) * length(x))))
    df[rows,]
  })
  i.g.light <- lapply(i.g.light, function(df) {
    threshold <- 5000; rm.perc <- 0.1
    rows <- rownames(df[df$states > threshold,])  # Rows to keep
    x <- rownames(df[df$states <= threshold,])    # Candidate rows to remove
    rows <- c(rows, sample(x, size=round((1 - rm.perc) * length(x))))
    df[rows,]
  })
  pdf(file=file, width=8, height=5)
  par(mar=c(2.25, 9.25, 0.5, 0.5))
  Stripchart(i.g.light, xlim=c(0, 60000), jitter=0.275)
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
  file <- paste0(dir, "/s.median.image.pdf")
  m.avg <- MatrixAvg(mlist)  # Average of all the state median matrices
  pdf(file=file, width=4.25, height=4.25)
  # less than 500: green; betw. 500 and 1450: yellow; more than 1450: red
  par(mar=c(0.25, 3, 2.75, 0.25))
  Image(m.avg, breaks=c(0, 500, 1450, 6000), col=c("green", "yellow", "red"))
  dev.off()
}

IntMichel <- function() {
  dir <- .MkDir("internal/michel")
  # Barplot with the number of states for all Michel automata for each constr.
  file <- paste0(dir, "/s.barplot.pdf")
  #pdf(file=file, width=7, height=5)
  par(mar=c(8, 6, 1, 0))
  i.m.empty <- lapply(i.m, function(df) { df$states <- 0; df })
  MichelBarplot(i.m, ylim=c(0, 300000), col="white", plot=FALSE)
  # par(new=TRUE)
  #MichelBarplot(i.m, ylim=c(0, 300000), col="white")
  # text(x=-7, y=150000, labels="Complement size", srt=90, xpd=TRUE)
  # abline(h=tail(axTicks(2)), col="gray", lty="dotted", xpd=FALSE)
  #dev.off()
}

ExtGoal <- function() {
dir <- .MkDir("external/goal")
  # LaTeX table with timeouts and memory outs (with Rank)
  file <- paste0(dir, "/outs.tex")
  df <- Outs(e.g.rank.all)
  LatexTable(df, include.rownames=FALSE, format="d", file=file)
  
}


.ComplUnivPersp <- function(m, file) {
  pdf(file=file, width=6, height=4.5)
  par(mar=c(1, 3, 0, 0.5))
  Persp(m, zlim=c(0, 100), theta=-35, phi=15, lin=TRUE, lin.xy=list(x=3, y=1),
        lin.col="gray", col="gray90", shade=0.6, ltheta=-90, lphi=30, zlab="",
        xlab="Transition density", ylab="Acceptance density")
  text(-0.74, 0.03, "Number of automata", srt=92, xpd=TRUE)
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
