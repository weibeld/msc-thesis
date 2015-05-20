
# Read all result files
all <- ReadSubdirs()

# Extract effective samples
eff <- EffSamples(all)

# Number of effective samples
nrow(eff[[1]])

# Number of timeouts and memouts of the different versions
lapply(all, OutT)
lapply(all, OutM)

# Aggregated statistics for the different versions
stats.agg <- rbind(
StatsAgg(eff$fribourg.goal,               v="Fribourg"),
StatsAgg(eff$fribourg.r2c.goal,           v="Fribourg+R2C"),
StatsAgg(eff$fribourg.r2c.c.goal,         v="Fribourg+R2C+C"),
StatsAgg(eff$fribourg.m1.goal,            v="Fribourg+M1"),
StatsAgg(eff$fribourg.m1.m2.goal,         v="Fribourg+M1+M2"),
StatsAgg(eff$fribourg.m1.r2c.goal,        v="Fribourg+M1+R2C"),
StatsAgg(eff$fribourg.m1.r2c.macc.r.goal, v="Fribourg+M1+R2C+MACC+R"))
# Create the LaTeX table; have to set 'align', because all cols are 'character'
LatexTable(FormatDataFrame(stats.agg, digits=1), align="rlrrrrrr",
                           include.rownames=FALSE, booktabs=TRUE)

