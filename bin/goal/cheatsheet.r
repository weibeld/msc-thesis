# Vectors (all elts. same type)
teams <- c("PHI","NYM","FLA","ATL","WSN")
w <- c(92, 89, 94, 72, 59)
l <- c(70, 73, 77, 90, 102)

# Lists (elts. can have different types)
list <- list(teams=teams, wins=w, losses=l)

# Data frame (list of equally long vectors)
df <- data.frame(teams=teams, wins=w, losses=l)

# Accessing elements of vector
w[c(1,5)]
1:3 # Same as c(1,2,3), numerical vector: 1 2 3
w[1:3]
w > 90		# Gives boolean vector: TRUE FALSE TRUE FALSE FALSE
w[w > 90]
teams[w > 90]
df$teams[df$w > 90]

# Defining a function
double <- function(x) {2*x} 

# List existing objects
objects()
ls() # Alias of above

# Saving and loading objects
save(df, file="./df.rdata")
rm(df) # Unset variable
load("./df.rdata")	# Creates variable df and loads object into it

# Help (about function)
help(data.frame)
?data.frame		# Alias of above
example(data.frame)	# EXECUTES example expressions with this function

# Read text file into data frame
df <- read.table(file="out", header=TRUE, sep="\t")
write.table(df, file="out_from_r", sep="\t", row.names=FALSE, quote=FALSE)

# Column names of data frame
names(df)

# Merge character vectors
a <- c("hello", "world")
b <- c("foo", "bar")
paste(a,b,sep="-")  # Gives character vector: "hello-foo" "world-bar"

# Set operations
c <- 1:20
d <- 10:30
intersect(c,d)
union(c,d)
setdiff(c,d)  # What is in c but not in d?
setequal(c,d)



