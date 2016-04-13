## create build rate plot

# The input file format:

# leafsize, snippet_size, build_or_not
# 12, 34, true
# 15, 76, false

## buildrate <- read.csv(file="out.csv", header=TRUE, sep=",");
## attributes(buildrate)
## names(buildrate)
## str(buildrate)
## buildrate

## length(buildrate)
## length(buildrate$leaf_size)
## max(buildrate$leaf_size)

data<- read.csv(file="out.csv")
str(data)
library(dplyr)
# library(ggplot2)
result <- summarise(
  group_by(data, leaf_size),
  ratio= sum(build_or_not==" true")/length(build_or_not)
)

plot(x=result$leaf_size, y=result$ratio, data=result)
lines(x=result$leaf_size, y=result$ratio, data=result)
