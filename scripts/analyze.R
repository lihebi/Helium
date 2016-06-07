

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

# @param y data frame, the response, should have name
# @param x data frame, the variable
# @return the string, or NULL
TransferFunctionSingle <- function(y, x) {
  # print(x)
  # print(y)
  # print(data.frame(y=y[[1]], x=x[[1]]))
  dat = data.frame(y=y[[1]], x=x[[1]])
  res = lm(y ~ x, data=dat)
  # print(summary(res))
  if (summary(res)$r.squared > 0.9) {
    ret = list(response=names(y), variable=names(x), coefficients=res$coefficients)
    # print(res$coefficients)
    return(ret)
  }
}

PrintTransferFunction <- function(func) {
  str <- paste(
    func$response, " = ",
    round(func$coefficients[2], digits=1), " * ", func$variable,
    " + ", round(func$coefficients[1], digits=1));
  return (str)
}

## setwd("~/tmp")

## accept command line csv file, and output invariants and transfer functions.


csvfile = commandArgs()[1]
csv = read.csv(csvfile, header=TRUE)



funcs = TransferFunction(csv);
print("Transfer functions:")
print(funcs)
