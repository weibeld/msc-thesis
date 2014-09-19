read <- function(file) {
  read.table(file=file, header=TRUE, sep="\t")
}

nb.timeouts <- function(df) {
  sum(df$t_out)
}

show.timeouts <- function(df) {
  df[df$t_out == 1,]
}

nb.memouts <- function(df) {
  sum(df$m_out)
}

summary.states <- function(df) {
  summary(df$states, na.rm=TRUE)
}

fivenum.states <- function(df) {
  fivenum(df$states, na.rm=TRUE)
}

summary.cputime <- function(df) {
  summary(df$cpu_t)
}

summary.realtime <- function(df) {
  summary(df$real_t)
}
