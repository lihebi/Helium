#lang racket

(struct node (loc))

(struct expr node (src))

(struct token node (src))

(struct decl node ())
(struct decl:trans-unit decl (decls))
(struct decl:function decl (ret name params body))

(struct stmt node ())
(struct stmt:decl stmt (src))
(struct stmt:expr stmt (src))
(struct stmt:compound stmt (stmts))
(struct stmt:for stmt (init test incr body))
(struct stmt:while stmt (test body))
(struct stmt:do stmt (body test))
(struct stmt:break stmt ())
(struct stmt:continue stmt ())
(struct stmt:return stmt (expr))
(struct stmt:if stmt (test cons alt))
(struct stmt:switch stmt (test cases))
(struct stmt:case stmt (test body))
(struct stmt:default stmt (body))


;; read from a file
;; produce ast in the above hierarchy
(define read-ast
  (lambda (port)
    (let ((raw (read port)))
      (print raw))))

;; (read-ast (open-input-file (expand-user-path "~/tmp/a.lisp")))


(define rawast (read (open-input-file (expand-user-path "~/tmp/a.lisp"))))

(define (get-loc obj)
  (string-split (symbol->string (cadr obj)) ":"))


(define create-ast
  (lambda (obj)
    (case (car obj)
      ['TranslationUnitDecl (decl:trans-unit (get-loc obj)
                                             (map create-ast (cddr obj)))]
      ['FunctionDecl (decl:function (get-loc obj)
                                    (third (third obj))
                                    (third (fourth obj))
                                    (third (fifth obj))
                                    (create-ast (sixth obj)))]
      ['CompoundStmt (stmt:compound (get-loc obj)
                                    (map create-ast
                                         (drop-right (drop obj 3) 1)))]
      ['DeclStmt (stmt:decl (get-loc obj)
                            (third obj))]
      ['IfStmt (stmt:if (get-loc obj)
                        (create-ast (fourth obj))
                        (create-ast (fifth obj))
                        (if (> (length obj) 5)
                            (create-ast (seventh obj))
                            null))]
      ['SwitchStmt (stmt:switch (get-loc obj)
                                (create-ast (fourth obj))
                                (map create-ast (drop obj 4)))]
      ['CaseStmt (stmt:case (get-loc obj)
                            (create-ast (fourth obj))
                            (create-ast (fifth obj)))]
      ['DefaultStmt (stmt:default (get-loc obj)
                                  (create-ast (fourth obj)))]
      ['WhileStmt (stmt:while (get-loc obj)
                              (create-ast (fourth obj))
                              (create-ast (fifth obj)))]
      ['DoStmt (stmt:do (get-loc obj)
                        (create-ast (fifth obj))
                        (create-ast (fourth obj)))]
      ['ForStmt (stmt:for (get-loc obj)
                          (create-ast (fourth obj))
                          (create-ast (fifth obj))
                          (create-ast (sixth obj))
                          (create-ast (seventh obj)))]
      ['BreakStmt (stmt:break (get-loc obj))]
      ['ContinueStmt (stmt:continue (get-loc obj))]
      ['ReturnStmt (stmt:return (get-loc obj)
                                (if (> (length obj) 3)
                                    (create-ast (fourth obj))
                                    null))]
      ['ExprStmt (stmt:expr (get-loc obj) (third obj))]
      ['Expr (expr (get-loc obj) (third obj))]
      [else (println (car obj)) null])))

(define ast (create-ast rawast))


;; (drop '(1 2 3) 1)
;; (second )
