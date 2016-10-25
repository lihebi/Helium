#!/usr/bin/Rscript

## supress warning
options(warn=-1)

contains <- function(s, pattern) {
    return (regexpr(pattern, s)[1] > -1);
}

startswith <- function(s, pattern) {
    return (regexpr(pattern,s)[1] == 1);
}

ComputeConstant <- function(data) {
    cat ("Constant output variable:\n");
    ## browser()
    for (i in seq(length(data))) {
        col <- data[i];
        name <- names(data)[i];
        if (startswith(name, "output")) {
            ## omit na
            col <- na.omit(col);
            raw <- col[[1]]
            if (length(raw) < 3) next;
            value <- raw[1]
            if (length(which(raw != value)) == 0) {
                cat(paste(name, "=",  value, "\n"))
            }
        }
    }
}



ComputeTransferFunction <- function(data) {
    cat("Transfer functions:\n")
    for (i in seq(length(data))) {
        for (j in seq(length(data))) {
            if (i == j) next;
            xname <- names(data)[i]
            yname <- names(data)[j]
            ## only calculate transfer function for output versus input
            if (substr(xname, 1, 5) == "input" && substr(yname, 1, 6) == "output") {
                ## omit address
                if (contains(xname, "addr") || contains(yname, "addr")) next;
                subframe <- data[c(j,i)]
                subframe <- na.omit(subframe)
                if (dim(subframe)[1] < 3) next;
                res <- lm(subframe)
                rsq <- summary(res)$r.squared
                if (!is.na(rsq) && rsq == 1) {
                    k = res$coefficients[2]
                    b = res$coefficients[1]

                    ## rounding k and b
                    k = round(k, digit=3)
                    b = round(b, digit=3)

                    if (k == 1) {
                        part1 <- xname;
                    } else if (k == 0) {
                        part1 <- "";
                    } else {
                        part1 <- paste(k,"*",xname);
                    }
                    if (b == 0) {
                        part2 <- "";
                    } else if (b > 0) {
                        part2 <- paste("+", b);
                    } else {
                        part2 <- paste("-", -b);
                    }
                    cat(paste(yname, "=", part1, part2, "\n"))
                    ## cat(paste(yname, "=", k, "*", xname, "+", b, "\n"))
                }
            }
        }
    }
}





##############################
## The commands
##############################

args = commandArgs(trailingOnly=TRUE)
## csvfile = "result.csv"
csvfile = args[1]
## csv = read.csv(csvfile, header=TRUE)

## do not replace [] with .
csv = read.csv(csvfile, header=TRUE, check.names=FALSE)

## only the failure traces
sub <- subset(csv, reach_code>=5)
sub <- subset(csv, reach_code==5 & status_code == 1)
## remove last two columns: (code)
sub <- sub[1:(length(csv)-2)]

ComputeTransferFunction(sub);
ComputeConstant(sub);

