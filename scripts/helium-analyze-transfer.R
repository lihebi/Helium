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
                if (as.character(value)=="(nil)") {
                    cat(paste(name, "=",  "nil", "\n"))
                } else {
                    cat(paste(name, "=",  value, "\n"))
                }
            }
        }
    }
}

GetFormula <- function(yname, res) {
    ## print("Getting formula")
    b = res$coefficients[1]
    k = res$coefficients[-1]
    k = round(k, digit=3)
    b = round(b, digit=3)
    parts <- c()
    for (kname in names(k)) {
        ## print("Meta:")
        ## print(kname)
        ## print(k[kname])
        ## print("Else:")
        if (is.na(k[kname])) next;
        if (k[kname]==1) {
            part <- kname
        } else if (k[kname]==0) {
            part <- ""
        } else {
            part <- paste(k[kname], "*", kname)
        }
        parts <- c(parts, part)
    }
    ## print(parts)
    rhs <- paste(parts, collapse=" + ")
    form <- paste(yname, "=", rhs)
    return (form);
}


ComputeNumber <- function(frame, xname, yname) {
    names(frame)=c(xname,yname)
    res <- lm(frame)
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

ComputeAddr <- function(frame, xname, yname) {
    ## cat("hello")
    ## print("frame1")
    ## print(frame[[1]])
    ## print("frame2")
    ## print(frame[[2]])
    ## browser()

    ## print(frame[[1]] == frame[[2]])

    if (all(as.character(frame[[1]]) == as.character(frame[[2]]))) {
        cat(paste(yname,"=",xname, "\n"))
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
                subframe <- data[c(j,i)]
                subframe <- na.omit(subframe)
                if (dim(subframe)[1] < 3) next;
                if (contains(xname, "addr") && contains(yname, "addr")) {
                    ComputeAddr(subframe, xname, yname);
                } else if (contains(xname, "addr") || contains(yname, "addr")){
                    ## one of them is addr, do nothing
                } else {
                    ## both are numbers ?
                    ComputeNumber(subframe, xname, yname);
                }
            }
        }
    }
}




sub[c("output_int_buf.strlen",InputNameIndex)]

MultiTrans <- function(frame) {
    OutputNameIndex=c()
    InputNameIndex=c()
    for (name in names(frame)) {
        ## print(name)
        if (contains(name, "addr")) next;
        if (substr(name, 1, 6) == "output") {
            OutputNameIndex=c(OutputNameIndex, name)
        }
        if (substr(name, 1, 5) == "input") {
            InputNameIndex=c(InputNameIndex, name)
        }
    }
    ## print(InputNameIndex)
    ## print(OutputNameIndex)

    for (name in OutputNameIndex) {
        subframe <- frame[c(name, InputNameIndex)]
        subframe <- na.omit(subframe)
        res <- lm(subframe)
        rsq <- summary(res)$r.squared
        if (!is.na(rsq) && rsq == 1) {
            ## print(summary(res))
            form <- GetFormula(names(subframe)[1], res);
            ## cat(form,"\n")
            print(form)
        }
    }
}

## use some test data doing testing
## Data Frame
## y=c(3,4,5)
## x1=c(1,2,4)
## x2=c(2,2,1)
## df = data.frame(y,x1,x2)
## res <- lm(df)
## summary(res)
## b=res$coefficients[1]
## k=res$coefficients[-1]


##############################
## The commands
##############################

args = commandArgs(trailingOnly=TRUE)
## csvfile = "result.csv"
csvfile = args[1]
## csv = read.csv(csvfile, header=TRUE)

## do not replace [] with .
csv = read.csv(csvfile, header=TRUE, check.names=FALSE, stringsAsFactors=FALSE)

## only the failure traces
sub <- subset(csv, reach_code>=5)
sub <- subset(csv, reach_code==5 & status_code == 1)
## remove last two columns: (code)
sub <- sub[1:(length(csv)-2)]

ComputeTransferFunction(sub);
ComputeConstant(sub);

## MultiTrans(sub);
