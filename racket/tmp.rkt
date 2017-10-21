#lang racket


(struct posn (x y [z #:auto]) #:auto-value 1)
(posn-x (posn 1 2))
(posn-z (posn 1 2))
