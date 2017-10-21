#lang racket

;; generate code from AST

(provide
 gen-single
 gen)

(require "ast.rkt")
(require racket/set)
(require racket/string)

(define (pretty-print-c code)
  "Add new lines for c code"
  ;; map through characters
  ;; add new line after { and ;
  (regexp-replace* #rx"[{};]" code "\\0\n"))

;; (filter non-empty-string? (regexp-split #rx"[{};]" "hello{;;worl}"))

;; (regexp-replace* #rx"[{};]" "hello{;;worl}" "\\0\n")

(define (gen-single-dense ast sel-set)
  "Generate code for single AST"
  (string-join
   (map (lambda (t) (token-node->string t))
        (filter
         (lambda (t) (set-member? sel-set t))
         (get-tokens ast))) " "))

(define (gen-single ast sel-set)
  (pretty-print-c (gen-single-dense ast sel-set)))

(define (gen file->ast-map sel-set)
  "TODO Generate code for multiple ASTs")

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; TEST

;; TEST MULTIPLE
;; (define file->sel-map (load-file->sel-map "/home/hebi/tmp/sel/0.json"))
;; (define file->ast-map (create-file->ast-map "/home/hebi/github/benchmark/craft/prep"))
;; (define sel-set (load-sel file->ast-map file->sel-map))

;; TEST SINGLE
;; (define myast (create-ast-for-file "/home/hebi/github/benchmark/craft/grammar/a.c"))
;; (displayln
;;  (gen-single myast (list->set (get-tokens myast))))

