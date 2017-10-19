#lang racket



;; snippet manager

;; use helium --create-snippet to create json file
;; Use this module to load from the json file

;; should support:
;; DONE use key to retrieve snippets
;; DONE recursively get all dependent snippets
;; DONE extract code from flie
;; DONE add deps and outers
;; DONE add ID index
;; DONE add key to a hash map


(require json)

(struct snip (id deps outers name file begin-loc end-loc))
(struct func-snip snip ())
(struct var-snip snip ())
(struct typedef-snip snip ())
;; union struct
(struct record-snip snip ())
(struct record-decl-snip snip ())
(struct enum-snip snip (fields))
(struct func-decl-snip snip ())





(define (get-loc jexp)
  (list (hash-ref jexp 'line)
        (hash-ref jexp 'col)))

(define (snip-meta jexp)
  ;; (displayln "hello")
  ;; (displayln jexp)
  ;; (displayln (hash-ref jexp 'name))
  (list (hash-ref jexp 'id)
        (hash-ref jexp 'deps)
        (hash-ref jexp 'outers)
        (hash-ref jexp 'name)
        (hash-ref jexp 'file)
        (get-loc (hash-ref jexp 'begin))
        (get-loc (hash-ref jexp 'end))))

(define load-snippet-json
  (lambda (json-file)
    (let ([all-snips (string->jsexpr
                      (file->string
                       (expand-user-path
                        json-file)))])
      (for/list ([snip-jexp all-snips])
        (case (hash-ref snip-jexp 'kind)
          [("FunctionSnippet")
           (apply func-snip (snip-meta snip-jexp))]
          [("VarSnippet")
           (apply var-snip (snip-meta snip-jexp))]
          [("TypedefSnippet")
           (apply typedef-snip (snip-meta snip-jexp))]
          [("RecordSnippet")
           (apply record-snip (snip-meta snip-jexp))]
          [("EnumSnippet")
           (apply enum-snip (append
                             (snip-meta snip-jexp)
                             (list (hash-ref snip-jexp 'fields))))]
          [("FunctionDeclSnippet")
           (apply func-decl-snip (snip-meta snip-jexp))]
          [("RecordDeclSnippet")
           (apply record-decl-snip (snip-meta snip-jexp))])))))

(define (create-id-index all-snips)
  (for/hash ([snip all-snips])
    (values (snip-id snip) snip)))
(define (get-keys snip)
  (cond
   [(enum-snip? snip)
    (displayln "hello")
    (displayln (enum-snip-fields snip))
    (cons (snip-name snip)
            (enum-snip-fields snip))]
   [else (list (snip-name snip))]))

(define (create-key-index all-snips)
  (for*/hash ([snip all-snips]
              [key (get-keys snip)])
    (values key snip)))



(define (set-add-list st lst)
  (if (null? lst) st
      (set-add-list (set-add st (car lst)) (cdr lst))))

;; worklist and done should be two sets
;; return the done sets in the end
(define (get-all-dep worklist done id-index)
  (if (set-empty? worklist)
      done
      (let ([item (set-first worklist)])
        (let ([newdeps (for/list ([dep (snip-deps (set-first worklist))]
                                  #:when (let ([dep-obj (hash-ref id-index dep)])
                                           (and (not (set-member? worklist dep-obj))
                                                (not (set-member? done dep-obj)))))
                         (hash-ref id-index dep))])
          (get-all-dep (set-add-list (set-rest worklist) newdeps) (set-add done item) id-index)))))



(define (read-file-for-code snip)
  (call-with-input-file (snip-file snip)
    (lambda (file)
      ;; (displayln (format "using ~a" file))
      (let ([begin-loc (snip-begin-loc snip)]
            [end-loc (snip-end-loc snip)])
        ;; (displayln (format "range: ~a ~a" begin-loc end-loc))
        (string-join
         (for/list ([i (in-naturals 1)]
                    #:break (> i (car end-loc)))
           (let ([line (read-line file)])
             ;; (displayln line)
             (cond
              [(< i (car begin-loc)) ""]
              [(= i (car begin-loc)) (substring line (sub1 (cadr begin-loc)))]
              [(< i (car end-loc)) line]
              [(= i (car end-loc)) (substring line 0 (cadr end-loc))]
              [else ""]))) "\n")))))


;; (define (read-file-for-code-until-semicolon snip))

;; (get-all-dep (set (hash-ref id-index 6)) (set) id-index)


;; (set-add-list (set 1 2 3 4) '(5 6 7))







;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Testing code
;; not putting in (module+ test) because geiser seems not able to run through it


(define test-filename
  (expand-user-path
   "~/tmp/snippets.json"))
(define id-index (create-id-index (load-snippet-json test-filename)))
(read-file-for-code (hash-ref id-index 3))
(for ([(key value) id-index])
  (displayln key)
  (read-file-for-code value))



;; (string->jsexpr
;;  (file->string
;;   test-filename))

;; (create-key-index (load-snippet-json test-filename))

;; (map get-keys (load-snippet-json test-filename))
;; (get-keys (car (load-snippet-json test-filename)))
