#lang racket

(require c)

(define myast (car (parse-program "int a;")))

(type:primitive-name (decl:vars-type myast))

(id:label-name (type:struct-tag (decl:vars-type (car (parse-program "struct  A *a;")))))

;; manage the threads
;; (custodian-shutdown-all (current-custodian))

(define-syntax-rule (debug sexp)
  ;; (printf "[debug] ~v\n" sexp)
  null
  )

(define (gen node)
  (cond
    [(not node) ""]
    ;; ID
    [(id:label? node) (debug "id:label")
                      (symbol->string (id:label-name node))]
    [(id:var? node) (debug "id:var")
                    (symbol->string (id:var-name node))]
    ;; TYPE
    [(type:pointer? node) (debug "type:pointer")
                          (string-append
                           "*"
                           ;; TODO base
                           (gen (type:pointer-qualifiers node)))]
    [(type:struct? node) (debug "type:struct")
                         (string-append
                          "struct "
                          (gen (type:struct-tag node)))]
    [(type:primitive? node) "TODO type:primitive?"]
    ;; DECL
    [(decl:typedef? node) (debug "decl:typedef")
                          (string-append
                           "typedef "
                           (gen (decl:typedef-type node))
                           " "
                           (string-join
                            (map gen (decl:typedef-declarators node))
                            ",")
                           ";")]
    [(decl:vars? node) (string-append
                        (gen (decl:vars-storage-class node))
                        " "
                        (gen (decl:vars-type node))
                        " "
                        (string-join
                         (map gen (decl:vars-declarators))
                         ","))]
    [(decl:formal? node)]
    [(decl:declarator? node) (debug "decl:declarator")
                             (string-append
                              (gen (decl:declarator-type node))
                              " "
                              (gen (decl:declarator-id node))
                              ;; TODO init
                              )]
    [(decl:function? node) "TODO decl:function?" node]
    [else (debug node) (pretty-print node) (error "debug here")]))

(gen (car (parse-program "typedef struct XXX *aaa, DDD;")))

(gen (car (parse-program
           (string->path
            "/home/hebi/github/benchmark/craft/grammar/a.c"))))


(parse-program "const int * const a;")
