#lang racket

;; snippet manager

;; use helium --create-snippet to create json file
;; Use this module to load from the json file

;; should support:
;; - use key to retrieve snippets
;; - recursively get all dependent snippets


(require json)

(struct snip (name file begin-loc end-loc ))
(struct func-snip snip ())
(struct var-snip snip ())
(struct typedef-snip snip ())
;; union struct
(struct record-snip snip ())
(struct record-decl-snip snip ())
(struct enum-snip snip ())
(struct func-decl-snip snip ())

(define load-snippet-json
  (lambda (json-file)
    (let ([all-snips (string->jsexpr
                      (file->string
                       (expand-user-path
                        json-file)))])
      (for/list ([snip-jexp all-snips])
        (case (hash-ref snip-jexp 'kind)
          [("FunctionSnippet") (func-snip snip-jexp)]
          [("VarSnippet") (var-snip snip-jexp)]
          [("TypedefSnippet") (typedef-snip snip-jexp)]
          [("RecordSnippet") (record-snip snip-jexp)]
          [("EnumSnippet") (enum-snip snip-jexp)]
          [("FunctionDeclSnippet") (func-decl-snip snip-jexp)]
          [("RecordDeclSnippet") (record-decl-snip snip-jexp)])))))

(define test-filename
  (expand-user-path
   "~/data/test/prep/sl-master/snippets.json"))

;; (string->jsexpr
;;  (file->string
;;   test-filename))

(load-snippet-json test-filename)
