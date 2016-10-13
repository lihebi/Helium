#!/usr/bin/Rscript

## supress warning
options(warn=-1)

TransferFunction <- function(data) {
    ret <- c()
    i<-1
    while (i <= length(data)) {
        j <- i+1
        while (j <= length(data)) {
            col1 = data[i]
            col2 = data[j]
            func = TransferFunctionSingle(col1, col2)
            if (!is.null(func)) {
                ret <- c(ret, PrintTransferFunction(func))
            }
            j = j + 1;
        }
        i = i + 1;
    }
    return(ret)
}

## @param y data frame, the response, should have name
## @param x data frame, the variable
## @return the string, or NULL
TransferFunctionSingle <- function(y, x) {
    ## print(x)
    ## print(y)
    ## print(data.frame(y=y[[1]], x=x[[1]]))
    ## print("Single Transfer for two columns:")
    ## print(names(x))
    ## print(names(y))
    ## print(x[[1]])
    ## print(y[[1]])
    dat = data.frame(y=y[[1]], x=x[[1]])
    res = lm(y ~ x, data=dat)
    ## print(summary(res))
    if (!is.na(summary(res)$r.squared) && summary(res)$r.squared > 0.9) {
        ret = list(response=names(y), variable=names(x), coefficients=res$coefficients)
        ## print(res$coefficients)
        return(ret)
    }
}

PrintTransferFunction <- function(func) {
    ## FIXME digit 1?
    k <- round(func$coefficients[2], digits=1);
    b <- round(func$coefficients[1], digits=1);
    if (k==1) {
        part1 <- paste(func$variable);
    } else {
        part1 <- paste(k,"*",func$variable);
    }
    if (b==0) {
        part2 <- "";
    } else {
        part2 <- paste("+", b);
    }
    str <- paste(func$response, "=", part1, part2);
    return (str)
}

#### setwd("~/tmp")

#### accept command line csv file, and output invariants and transfer functions.


args = commandArgs(trailingOnly=TRUE)
## print("length of args: ")
## print(length(args))
## for (arg in args) {
##     print(arg)
## }

csvfile = args[1]
## print("The data file:")
## print(csvfile)

csv = read.csv(csvfile, header=TRUE)
## print("Inside the file:")
## print(csv)


##############################
## Some small analysis
##############################

## How many tests in total:
total_test <- dim(csv)[[1]]
sub = subset(csv, reach_code>=5)
total_reach_poi <- dim(sub)[[1]]
sub = subset(csv, reach_code==5 & status_code == 1)
total_fail_poi <- dim(sub)[[1]]

cat("Total test: ", total_test, "\n")
cat("Total reach poi: ", total_reach_poi, "\n")
cat("Total fail poi: ", total_fail_poi, "\n")


## remove last two columns: (code)
sub <- sub[1:(length(csv)-2)]
funcs = TransferFunction(sub);



## print out result
cat("Transfer functions:\n")
for (func in funcs) {
    cat(func, "\n")
}

## print("Transfer functions:")
## print("")
## print(funcs)
