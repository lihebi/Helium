#lang racket

(require json)
(require racket/path)

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

(define (describe-node n)
  (cond
   [(expr? n) (format "expr:~a" (expr-src n))]
   [(token? n) (format "token:~a" (token-src n))]
   [(stmt:expr? n) (format "stmt:expr:~a" (stmt:expr-src n))]
   [(stmt:break? n) "stmt:break"]
   [(stmt:continue? n) "stmt:continue"]
   [(decl:trans-unit? n) "decl:trans-unit"]
   [(decl:function? n) "decl:function"]
   [(stmt:compound? n) "stmt:compound"]
   [(stmt:for? n) "stmt:for"]
   [(stmt:while? n) "stmt:while"]
   [(stmt:do? n) "stmt:do"]
   [(stmt:return? n) "stmt:return"]
   [(stmt:if? n) "stmt:if"]
   [(stmt:switch? n) "stmt:switch"]
   [(stmt:case? n) "stmt:case"]
   [(stmt:default? n) "stmt:default"]
   [else null]))

;; (decl:trans-unit-decls ast)
;; (travel ast)
(define (travel n)
  "Return a hierarchical list of nodes. Use flatten if desired."
  (cond
   [(expr? n) n]
   [(token? n) n]
   [(stmt:expr? n) n]
   [(stmt:break? n) n]
   [(stmt:continue? n) n]
   [(decl:trans-unit? n) (cons n (map travel (decl:trans-unit-decls n)))]
   [(decl:function? n) (cons n (map travel (list (decl:function-ret n)
                                                 (decl:function-name n)
                                                 (decl:function-params n)
                                                 (decl:function-body n))))]
   [(stmt:compound? n) (cons n (map travel (stmt:compound-stmts n)))]
   [(stmt:for? n) (cons n (map travel (list (stmt:for-init n)
                                            (stmt:for-test n)
                                            (stmt:for-incr n)
                                            (stmt:for-body n))))]
   [(stmt:while? n) (cons n (map travel (list (stmt:while-test n)
                                              (stmt:while-body n))))]
   [(stmt:do? n) (cons n (map travel (list (stmt:do-test n)
                                           (stmt:do-body n))))]
   [(stmt:return? n) (cons n (map travel (list (stmt:return-expr n))))]
   [(stmt:if? n) (cons n (map travel (list (stmt:if-test n)
                                           (stmt:if-cons n)
                                           (stmt:if-alt n))))]
   [(stmt:switch? n) (cons n (map travel (cons (stmt:switch-test n)
                                               (stmt:switch-cases n))))]
   [(stmt:case? n) (cons n (map travel (list (stmt:case-test n)
                                             (stmt:case-body n))))]
   [(stmt:default? n) (cons n (travel (stmt:default-body n)))]
   [else null]))


(define (get-loc obj)
  (string-split (symbol->string (list-ref obj 1)) ":"))

(define create-ast
  (lambda (obj)
    (case (car obj)
      ['TokenNode (token (get-loc obj) (third obj))]
      ['TranslationUnitDecl (decl:trans-unit (get-loc obj)
                                             (map create-ast (cddr obj)))]
      ['FunctionDecl (decl:function (get-loc obj)
                                    (create-ast (third obj))
                                    (create-ast (fourth obj))
                                    (create-ast (fifth obj))
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


(define (print-hier hier-lst [level 0])
  "Print hierarchically"
  (if (list? hier-lst) (begin
                         ;; (println "-----")
                         (map (lambda (x) (print-hier x (add1 level))) hier-lst))
      (displayln (format "~a~a" (make-string level #\space) (describe-node hier-lst)))))

(define (create-asts-for-folder path)
  ;; parse a preprocessed c file folder
  ;; for each file, run Helium to dump AST
  ;; create an AST for it
  ;; create a map of filename to AST
  (for/hash ([dir (in-directory path)])
    (cond
     [(directory-exists? dir) #(void)]
     [(file-exists? dir)
      (let ([ext (bytes->string/locale (path-get-extension dir))])
        (when (or (string=? ext ".c")
                  (string=? ext ".h"))
          ;; run command
          (values dir (create-ast-for-file))))])))

(define (create-ast-for-file path)
  (let ([helium-cmd (format "helium --dump-ast ~a" path)])
    (create-ast
     (read
      (open-input-string
       (with-output-to-string (system helium-cmd)))))))

;; selection
(define (load-sel file-to-ast-map sel-file)
  ;; 1. read
  ;; 2. for each file, locate the AST
  ;; 3. based on the loc, locate the nodes
  ;; 4. return the SET of selected nodes
  (let ([jobj (read-json sel-file)])
    (for ([item jobj])
      (let ([file (hash-ref item 'file)]
            [sels (hash-ref item 'sel)])
        (let ([ast (hash-ref file-to-ast-map file)])
          (let ([tokens (get-tokens ast)])
            (append (for/list ([sel sels])
                      (let ([loc (list (hash-ref sel 'line) (hash-ref sel 'col))])
                        ;; this should be exactly one
                        ;; this will be expensive
                        (filter is-this-token? tokens))))))))))

(define (get-tokens n)
  (map (lambda (n)
         (cond
          [(expr? n) n]
          [(stmt:decl? n) n]
          [(stmt:expr? n) n]
          [(token? n) n]
          [else null]))
       (flatten (travel n))))



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


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; tests

(define rawast (read (open-input-file (expand-user-path "~/tmp/a.lisp"))))
(define ast (create-ast rawast))

(get-tokens ast)

(flatten (travel ast))
(print-hier (travel ast))
