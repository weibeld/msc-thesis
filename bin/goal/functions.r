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

fivenum.states <- function(df) {
  fivenum(df$states, na.rm=TRUE)
}

summary.cputime <- function(df) {
  summary(df$tcpu_t)
}

summary.realtime <- function(df) {
  summary(df$real_t)
}
