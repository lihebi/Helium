#lang racket

;; parse source code

;; 1. extract typedefs from (header) file
;; 2. call parse-program

(require c)

(define (get-typedefs dir #:exclude [ex ""])
  (let ([cmd (string-append
              "ctags -f - -R --exclude="
              ex
              " "
              dir
              " | "
              "awk 'BEGIN {FS=\"\\t\"} {if ($4 == \"t\") print $1}'")])
    (map string->symbol
         (string-split
          (with-output-to-string (lambda () (system cmd)))))))

;; (get-typedefs "/usr/include/dirent.h")
;; (get-typedefs "/usr/include" "*/boost/*")

(define (load-or-create-typedefs)
  "in the current directory, locate the file typedef.txt"
  (let ([f "typedef.txt"])
    (if (file-exists? f)
        (map string->symbol (string-split (file->string f)))
        (let ([typedefs (get-typedefs
                    "/usr/include" #:exclude "*/boost/*")])
          (with-output-to-file f
            (lambda ()
              (for ([item typedefs])
                (displayln item))))
          typedefs))))

;; (length (load-or-create-typedefs))

;; (get-typedefs "/usr/include/dirent.h")

;; (get-typedefs "/home/hebi/github/benchmark/buffer-overflow/prep-polymorph/")


;; (define predefined-typedefs '(DIR))

#;
(parse-program
 (string->path
  "/home/hebi/github/benchmark/buffer-overflow/prep-polymorph/polymorph.c")
 #:typedef predefined-typedefs)


(define *standard-headers* '(assert.h
                             complex.h
                             ctype.h
                             errno.h
                             fenv.h
                             float.h
                             inttypes.h
                             iso646.h
                             limits.h
                             locale.h
                             math.h
                             setjmp.h
                             signal.h
                             stdarg.h
                             stdbool.h
                             stddef.h
                             stdint.h
                             stdio.h
                             stdlib.h
                             string.h
                             tgmath.h
                             time.h
                             wchar.h
                             wctype.h
                             ))

(module+ test
  (define *c-path* "/usr/lib/gcc/x86_64-pc-linux-gnu/7.2.0/include/")
  (for ([h *standard-headers*])
    (if (file-exists?
         (string-append *c-path* (symbol->string h)))
        (displayln (format "yes: ~a" h))
        (displayln (format "no: ~a" h)))))

(define (get-search-path [cpp 'cpp])
  (let ([cpp-cmd (case cpp
                   ['cpp "cpp -v /dev/null -o /dev/null 2>&1"]
                   ['clang "clang -x c -v -E /dev/null 2>&1"]
                   [else (error (format "only support cpp or clang, given ~a" cpp))])]
        [awk-cmd (string-append
                  "awk '"
                  "BEGIN {ok=0;}"
                  "/End of search list/ {ok=0;}"
                  "{if (ok) print $0;}"
                  "/#include <...> search starts here/ {ok=1;}"
                  "'")])
    (map string-trim
         (string-split
          (with-output-to-string
            (lambda ()
              (system (string-append cpp-cmd " | " awk-cmd))))
          "\n"))))

(module+ test
  (get-search-path 'clang))

(define (parse-dir dir #:typedef [typedefs '()])
  (for ([f (in-directory dir)])
    (when (file-exists? f)
      (let ([ext (bytes->string/locale (path-get-extension f))])
        (when (or (string=? ext ".c")
                  (string=? ext ".h"))
          (displayln (format "Parsing ~a" f))
          ;; (displayln "hello")
          (with-handlers ([exn:fail:contract? (lambda (exn) (displayln "Exception!!") (print exn) null)])
            (parse-program f #:typedef typedefs))
          #;
          (call-with-exception-handler
           (lambda (e) (print e))
           (lambda () (parse-program f #:typedef typedefs)))
          )))))


#;
(let ([bench "/home/hebi/Downloads/prep-findutils"])
  ;; (println (get-typedefs bench))
  (parse-dir bench
             #:typedef (append
                        (load-or-create-typedefs)
                        (get-typedefs bench))))

;; (println (car (load-or-create-typedefs)))
(module+ test
  (parse-program (string->path "test/a.c"))
  (parse-program (string->path "test/b.c")
                 #:typedef (append
                            (get-typedefs "/home/hebi/Downloads/prep-findutils"))))

;; (parse-statement "if (strchr (state.rel_pathname, '/')) {}")

#;
(parse-statement "      if (strchr (state.rel_pathname, '/'))
 {
   char dir = foo ();
   int result = bar ();
 }
")


#;
(parse-statement "{
   int dir = foo;
   int result = bar;
}")
