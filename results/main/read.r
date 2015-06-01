
# Test set analysis
cmpl <- Read("completeness/completeness.csv")
univ <- Read("universality/universality.csv")

#------------------------------------------------------------------------------#
# Internal tests
#------------------------------------------------------------------------------#

# GOAL automata
#-------------#

i.g.all <- ReadSubdirs("internal/goal")
i.g <- Eff(i.g.all)

# States: aggregated statistics
i.g.s1 <- Stats(i.g)[c(1, 7, 6, 2, 3, 5, 4), ]
rownames(i.g.s1) <- NULL

# States: dt/da classes statistics
i.g.s2 <- StatsGoal(i.g)

# States: dt/da classes median matrix
i.g.s2.m <- MatrixGoal(i.g, stat="median")

# Time: aggregated statistics
i.g.t1 <- Stats(i.g, col="cpu_time")[c(1, 7, 6, 2, 3, 5, 4), ]
rownames(i.g.t1) <- NULL

# Time: dt/da classes statistics
i.g.t2 <- StatsGoal(i.g, col="cpu_time")

# Time: dt/da classes median matrix
i.g.t2.m <- MatrixGoal(i.g, col="cpu_time", stat="median")


# Michel automata
#---------------#

i.m <- ReadSubdirs("internal/michel")

# States
i.m.s <- Michel(i.m)[c(6, 7, 4, 5, 1, 2, 3), ]
rownames(i.m.s) <- NULL

# Time
i.m.t <- Michel(i.m, col="cpu_time")[c(6, 7, 4, 5, 1, 2, 3), ]
rownames(i.m.t) <- NULL


#------------------------------------------------------------------------------#
# External tests
#------------------------------------------------------------------------------#

# GOAL automata
#-------------#

# Without Rank

e.g.all <- ReadSubdirs("external/goal")
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

e.g.rank.all <- e.g.all
e.g.rank.all$rank.tr.ro.goal <- Read("external/goal/rank/rank.tr.ro.goal/rank.tr.ro.goal.csv")
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
e.m.states <- Michel(e.m)

# Times
e.m.time <- Michel(e.m)
