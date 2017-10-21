#lang racket

(provide
 create-file2ast
 load-file2sel
 create-rand-file2sel
)

(require "ast.rkt")
(require "sel.rkt")
(require racket/random)

;; manage file to ast/sel mapping
;; file2sel file -> selected tokens
;; file2selspec file -> '((1 2) (3 4) ...)
;; file2ast file -> s#<tran unit>

(define (create-file2ast path)
  "Create AST for a folder of .c and .h code"
  ;; parse a preprocessed c file folder
  ;; for each file, run Helium to dump AST
  ;; create an AST for it
  ;; create a map of filename to AST
  (cond
    [(file-exists? path) (hash path (create-ast-for-file path))]
    [(directory-exists? path)
     (for/hash ([dir (in-directory path)]
                ;; not a directory
                #:when (and (file-exists? dir)
                            (let ([ext (bytes->string/locale
                                        (path-get-extension dir))])
                              (or (string=? ext ".c")
                                  (string=? ext ".h")))))
       (values (path->string dir) (create-ast-for-file dir)))]
    [else (error "Path invalid" path)]))

(define (group-tokens file2ast tokens)
  "Group tokens to their file. Create a file-to-tokens-map."
  (for/hash ([(f ast) (in-hash file2ast)])
    (let ([s (list->seteq (get-tokens ast))])
      (values f (filter (lambda (t) (set-member? s t)) tokens)))))
(define (create-rand-file2sel file2ast n)
  "Create random selection of n tokens in all the ASTs"
  ;; 1. get all tokens
  ;; 2. select random
  ;; 3. group them back to file
  (let ([all-tokens (append
                     (for/list ([(f ast) (in-hash file2ast)])
                       (get-tokens ast)))])
    (let ([samples (random-sample all-tokens n)])
      (print (length samples))
      (group-tokens file2ast samples))))
(define (file2sel->file2selspec file2sel)
  (for/hash ([(f sel) (file2sel)])
    (values f (map (lambda (t) (node-loc t)) sel))))
