#lang racket

(require "ast.rkt")

(provide
 patch-single
 patch)


(define (patch-single ast sel)
  "TODO patch a single AST"
  )

(define (patch file->ast-map file->sel-map)
  "TODO patch multiple ASTs")


;; (define test-file "/home/hebi/github/benchmark/craft/grammar/a.c")
;; (define test-sel '((12 5)))
;; (define test-ast (create-ast-for-file test-file))
;; (patch-single test-ast test-sel)
