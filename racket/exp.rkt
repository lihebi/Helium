#lang racket

(require "ast.rkt")
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


(define file->sel-map (load-file->sel-map "/home/hebi/tmp/sel/0.json"))
(define file->ast-map (create-file->ast-map "/home/hebi/github/benchmark/craft/prep"))
(define sel-set (load-sel file->ast-map file->sel-map))

;; (patch file->ast-map sel-set)
