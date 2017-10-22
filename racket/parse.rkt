#lang racket

;; parse source code

;; 1. extract typedefs from (header) file
;; 2. call parse-program

(require c)

(define (get-typedefs path)
  "Extract listof symbols"
  ;; execute ctags /path/to/path -> tags
  (let ([cmd (string-append
              "ctags -f - " path
              " | "
              ""
              "awk 'BEGIN {FS=\"\\t\"} {if ($4 == \"t\") print $1}'")])
    ;; (displayln cmd)
    (string-split (with-output-to-string (lambda () (system cmd))))))

(get-typedefs "/usr/include/dirent.h")


;; (parse-program
;;  (string->path
;;   "/home/hebi/github/benchmark/buffer-overflow/prep-polymorph/polymorph.c"))
