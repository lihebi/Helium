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
              "awk 'BEGIN {FS=\"\\t\"} {if ($4 == \"t\") print $1}'")])
    ;; (displayln cmd)
    (string-split (with-output-to-string (lambda () (system cmd))))))

(define (get-dir-typedefs dir #:exclude [ex ""])
  (let ([cmd (string-append
              "ctags -f - -R --exclude="
              ex
              " "
              dir
              " | "
              "awk 'BEGIN {FS=\"\\t\"} {if ($4 == \"t\") print $1}'")])
    (string-split (with-output-to-string (lambda () (system cmd))))))

;; (get-dir-typedefs "/usr/include" "*/boost/*")
(define (load-or-create-typedefs)
  "in the current directory, locate the file typedef.txt"
  (let ([f "typedef.txt"])
    (if (file-exists? f)
        (map string->symbol (string-split (file->string f)))
        (let ([typedefs (get-dir-typedefs
                    "/usr/include" #:exclude "*/boost/*")])
          (with-output-to-file f
            (lambda ()
              (for ([item typedefs])
                (displayln item))))
          typedefs))))

(length (load-or-create-typedefs))

;; (get-typedefs "/usr/include/dirent.h")


(define predefined-typedefs '(DIR))

(parse-program
 (string->path
  "/home/hebi/github/benchmark/buffer-overflow/prep-polymorph/polymorph.c")
 #:typedef predefined-typedefs)


;; TODO parse local typedefs

(define (parse-dir dir)
  (for ([f (in-directory dir)])
    (when (file-exists? f)
      (let ([ext (bytes->string/locale (path-get-extension f))])
        (when (or (string=? ext ".c")
                  (string=? ext ".h"))
          (displayln (format "Parsing ~a" f))
          (parse-program f #:typedef (load-or-create-typedefs)))))))
(parse-dir "/home/hebi/github/benchmark/buffer-overflow/prep-polymorph/")

(require racket/path)
(bytes->string/locale (path-get-extension "~/tmp/include.json"))
