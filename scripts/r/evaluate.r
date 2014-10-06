args <- commandArgs(trailingOnly=TRUE)
file <- args[1]

data <- read.table(file, header=TRUE, sep="\t")
items <- nrow(data)
completed <- nrow(na.omit(data))
mean_states <- mean(data$states, na.rm=T)
timeouts <- sum(data$t_out)
memory_outs <- sum(data$m_out)
