#lang racket

(provide
 ;; sel.json -> file->sel-map
 load-file->sel-map
 ;; file->ast file->sel => (set tokens)
 load-sel
 )

(require racket/random)
(require json)
(require "ast.rkt")

(define (loc<= lhs rhs)
  (cond
   [(< (car lhs) (car rhs)) #t]
   [(> (car lhs) (car rhs)) #f]
   [(<= (cadr lhs) (cadr rhs)) #t]
   [else #f]))

(define (is-this-token? token loc)
  (let ([begin-loc (drop-right (node-loc token) 2)]
        [end-loc (drop (node-loc token) 2)])
    (and (loc<= begin-loc loc)
         (loc<= loc end-loc))))

(define (group-tokens file2ast tokens)
  "Group tokens to their file. Create a file-to-tokens-map."
  (for ([(f ast) (in-hash file2ast)])
    (let ([s (list->set (get-tokens ast))])
      (values f (filter (lambda (t) (set-member? s t)) tokens)))))

;; file2sel file -> selected tokens
;; file2selspec file -> '((1 2) (3 4) ...)
;; file2ast file -> s#<tran unit>

(define (create-rand-file2sel file2ast n)
  "Create random selection of n tokens in all the ASTs"
  ;; 1. get all tokens
  ;; 2. select random
  ;; 3. group them back to file
  (let ([all-tokens (append
                     (for/list ([(f ast) (in-hash file2ast)])
                       (get-tokens ast)))])
    (let ([samples (random-sample all-tokens n)])
      (group-tokens file2ast samples))))

(define (file2sel->file2selspec file2sel)
  (for/hash ([(f sel) (file2sel)])
    (values f (map (lambda (t) (node-loc t)) sel))))

(define (load-selspec spec-file)
  "TODO Load spec file into file2sel"
  )

;; old sel-file format

(define (load-file->sel-map sel-file)
  (let ([jobj (read-json (open-input-file sel-file))])
    (for/hash ([item jobj])
      (values (hash-ref item 'file)
              ;; this is a list
              (hash-ref item 'sel)))))

(define (load-sel-single ast sels)
  (filter-not null?
              (let ([tokens (get-tokens ast)])
                (for*/list ([t tokens]
                            [s sels])
                  (let ([loc (list (hash-ref s 'line)
                                   (hash-ref s 'col))])
                    (if (is-this-token? t loc) t null))))))

(define (load-sel file->ast-map file->sel-map)
  "return a set of selected tokens"
  (list->set
   (filter-not
    null?
    ;; flatten the inner list
    (flatten
     (for/list ([(key value) (in-hash file->ast-map)])
       (if (hash-has-key? file->sel-map key)
           (load-sel-single value (hash-ref file->sel-map key))
           null))))))

