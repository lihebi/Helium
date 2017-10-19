#lang racket

;; snippet manager

;; use helium --create-snippet to create json file
;; Use this module to load from the json file

;; should support:
;; TODO use key to retrieve snippets
;; TODO recursively get all dependent snippets
;; TODO extract code from flie
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

(define test-filename
  (expand-user-path
   "~/tmp/snippets.json"))

;; (string->jsexpr
;;  (file->string
;;   test-filename))

;; (create-key-index (load-snippet-json test-filename))

;; (map get-keys (load-snippet-json test-filename))
;; (get-keys (car (load-snippet-json test-filename)))
