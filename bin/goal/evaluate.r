args <- commandArgs(trailingOnly=TRUE)
file <- args[1]

data <- read.delim(file, comment.char="#")
items <- nrow(data)
completed <- nrow(na.omit(data))
mean_states <- mean(data$states, na.rm=T)
timeouts <- sum(data$t_out)
memory_outs <- sum(data$m_out)
