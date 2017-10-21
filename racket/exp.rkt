#lang racket

(require "ast.rkt")
(require "sel.rkt")
(require "patch.rkt")



;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; tests

;; (define rawast (read (open-input-file (expand-user-path "~/tmp/a.lisp"))))
;; (define ast (create-ast rawast))

;; (get-tokens ast)

;; (flatten (travel ast))
;; (print-hier (travel ast))

;; (print-hier (travel (create-ast-for-file
;;                      "/home/hebi/github/benchmark/craft/grammar/a.c")))



;; (create-file->ast-map "/home/hebi/github/benchmark/craft/grammar")
;; (define test-dir "/home/hebi/github/benchmark/craft/prep")
;; (create-file->ast-map test-dir)

;; (load-sel (create-file->ast-map test-dir)
;;           (open-input-file "/home/hebi/tmp/sel/0.json"))


(define file2ast
  (create-file2ast "/home/hebi/github/benchmark/craft/prep"))
(create-rand-file2sel file2ast 1)

;; (patch file->ast-map sel-set)
