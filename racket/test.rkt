#lang racket
(require racket)
(require c)
(require c/parse)

(require racket/file)

;; (let ([f (open-input-file "a.c")])
;;   ;; (displayln (file->string "a.c"))
;;   (parse-program f))

;; (define myfile (open-input-file "a.c"))

;; (parse-program (open-input-file "a.c"))
(parse-program "a.c")
