
# Test set analysis
cmpl <- Read("completeness/completeness.csv")
cmpl.m <- MatrixTestset(cmpl)

univ <- Read("universality/universality.csv")
univ.m <- MatrixTestset(univ)

#------------------------------------------------------------------------------#
# Internal tests
#------------------------------------------------------------------------------#

# GOAL automata
#-------------#

# Read list of data frames and put list elements in desired order
i.g.all <- ReadSubdirs("internal/goal")[c(1, 7, 6, 2, 3, 5, 4)]

# Get effective samples
i.g <- Eff(i.g.all)

# States: aggregated statistics
i.g.s1 <- Stats(i.g)

# States: dt/da classes statistics
i.g.s2 <- StatsGoal(i.g)

# States: dt/da classes median matrix
i.g.s2.m <- MatrixGoal(i.g, stat="median")

# Time: aggregated statistics
i.g.t1 <- Stats(i.g, col="cpu_time")

# Time: dt/da classes statistics
i.g.t2 <- StatsGoal(i.g, col="cpu_time")

# Time: dt/da classes median matrix
i.g.t2.m <- MatrixGoal(i.g, col="cpu_time", stat="median")


# Michel automata
#---------------#

# Read list of data frames and put list elements in desired order
i.m <- ReadSubdirs("internal/michel")[c(6, 7, 4, 5, 1, 2, 3)]

# States
i.m.s <- Michel(i.m)

# Time
i.m.t <- Michel(i.m, col="cpu_time")


#------------------------------------------------------------------------------#
# External tests
#------------------------------------------------------------------------------#

# GOAL automata
#-------------#

# Without Rank

# Read list of data frames
e.g.all <- ReadSubdirs("external/goal")

# Get effective samples
e.g <- Eff(e.g.all)

# States: aggregated statistics
e.g.s1 <- Stats(e.g)

# States: dt/da classes statistics
e.g.s2 <- StatsGoal(e.g)

# States: dt/da classes median matrix
e.g.s2.m <- MatrixGoal(e.g, stat="median")

# Time: aggregated statistics
e.g.t1 <- Stats(e.g, col="cpu_time")

# Time: dt/da classes statistics
e.g.t2 <- StatsGoal(e.g, col="cpu_time")

# Time: dt/da classes median matrix
e.g.t2.m <- MatrixGoal(e.g, col="cpu_time", stat="median")


# With Rank

# Add the rank data frame to the list of previously read data frames
e.g.rank.all <- e.g.all
e.g.rank.all$rank.tr.ro.goal <- Read("external/goal/rank/rank.tr.ro.goal/rank.tr.ro.goal.csv")

# Get effective samples
e.g.rank <- Eff(e.g.rank.all)

# States: aggregated statistics
e.g.rank.s1 <- Stats(e.g.rank)

# States: dt/da classes statistics
e.g.rank.s2 <- StatsGoal(e.g.rank)

# States: dt/da classes median matrix
e.g.rank.s2.m <- MatrixGoal(e.g.rank, stat="median")

# Time: aggregated statistics
e.g.rank.t1 <- Stats(e.g.rank, col="cpu_time")

# Time: dt/da classes statistics
e.g.rank.t2 <- StatsGoal(e.g.rank, col="cpu_time")

# Time: dt/da classes median matrix
e.g.rank.t2.m <- MatrixGoal(e.g.rank, col="cpu_time", stat="median")


# Michel automata
#---------------#

e.m <- ReadSubdirs("external/michel")

# States
e.m.s <- Michel(e.m)

# Times
e.m.t <- Michel(e.m)
