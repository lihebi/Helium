#lang racket

(provide
 ;; sel.json -> file->sel-map
 ;; file->ast file->sel => (set tokens)
 ;; load-sel
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

(define (create-rand-sel ast n)
  (random-sample (get-tokens ast) n))

(define (sel->selspec sel)
  (map (lambda (t) (node-loc t) sel)))

(define (dump-selspec spec f)
  (with-output-to-file f
    (lambda () (write spec))))
(define (load-selspec f)
  (with-input-from-file f
    (lambda ()
      (read))))

;; (create-ast-for-file "/home/hebi")
