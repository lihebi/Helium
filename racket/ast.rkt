#lang racket
(provide
 ;; folder -> ASTs
 create-file->ast-map
 ;; .c|.h -> AST
 create-ast-for-file
 ;; sel.json -> file->sel-map
 load-file->sel-map
 ;; file->ast file->sel => (set tokens)
 load-sel
 get-tokens
 token-node->string
 pretty-print-ast)

(require json)
(require racket/path)

(struct node (loc))
;; expr
(struct expr node (src))
(struct token node (src) #:transparent)
;; decl
(struct decl node ())
(struct decl:trans-unit decl (decls))
(struct decl:function decl (ret name lparen params rparen body))
;; stmt
(struct stmt node ())
(struct stmt:decl stmt (src))
(struct stmt:expr stmt (src))
(struct stmt:break stmt ())
(struct stmt:continue stmt ())
(struct stmt:comp stmt (lbrace stmts rbrace))

;; loop
(struct stmt:for stmt (kw lparen init semi1 test semi2 incr rparen body))
(struct stmt:while stmt (kw lparen test rparen body))
(struct stmt:do stmt (kw-do body kw-while lparen test rparen semi))

;; conditional
(struct stmt:if stmt (kw-if lparen test rparen cons kw-else alt))
(struct stmt:switch stmt (kw lparen test rparen lbrace cases rbrace))

(struct stmt:return stmt (kw expr semi))
(struct stmt:case stmt (kw test colon body))
(struct stmt:default stmt (kw colon body))

;; keep this up-to-date!!
(define (is-token? n)
  (or (expr? n)
      (token? n)
      (stmt:expr? n)
      (stmt:decl? n)
      (stmt:break? n)
      (stmt:continue? n)))

(define (token-node->string n)
  (cond
   [(expr? n) (expr-src n)]
   [(token? n) (token-src n)]
   [(stmt:expr? n) (stmt:expr-src n)]
   [(stmt:decl? n) (stmt:decl-src n)]
   [(stmt:break? n) "break;"]
   [(stmt:continue? n) "continue;"]
   [else (print n) (error "debug!!")]))

(define (describe-node n)
  (cond
   [(expr? n) (format "expr:~a" (expr-src n))]
   [(token? n) (format "token:~a" (token-src n))]
   [(stmt:expr? n) (format "stmt:expr:~a" (stmt:expr-src n))]
   [(stmt:break? n) "stmt:break"]
   [(stmt:continue? n) "stmt:continue"]
   [(decl:trans-unit? n) "decl:trans-unit"]
   [(decl:function? n) "decl:function"]
   [(stmt:comp? n) "stmt:comp"]
   [(stmt:for? n) "stmt:for"]
   [(stmt:while? n) "stmt:while"]
   [(stmt:do? n) "stmt:do"]
   [(stmt:return? n) "stmt:return"]
   [(stmt:if? n) "stmt:if"]
   [(stmt:switch? n) "stmt:switch"]
   [(stmt:case? n) "stmt:case"]
   [(stmt:default? n) "stmt:default"]
   [else (error "debug!!")]))

;; (decl:trans-unit-decls ast)
;; (travel ast)
(define (travel n)
  "Return a hierarchical list of nodes. Use flatten if desired."
  (cond
    [(expr? n) n]
    [(token? n) n]
    [(stmt:expr? n) n]
    [(stmt:decl? n) n]
    [(stmt:break? n) n]
    [(stmt:continue? n) n]
    [(decl:trans-unit? n) (cons n (map travel (decl:trans-unit-decls n)))]
    [(decl:function? n) (cons n (map travel (list (decl:function-ret n)
                                                  (decl:function-name n)
                                                  (decl:function-lparen n)
                                                  (decl:function-params n)
                                                  (decl:function-rparen n)
                                                  (decl:function-body n))))]
    [(stmt:comp? n) (cons n (map travel
                                 `(,(stmt:comp-lbrace n)
                                   ,@(stmt:comp-stmts n)
                                   ,(stmt:comp-rbrace n))))]
    [(stmt:for? n) (cons n (map travel (list (stmt:for-kw n)
                                             (stmt:for-lparen n)
                                             (stmt:for-init n)
                                             (stmt:for-semi1 n)
                                             (stmt:for-test n)
                                             (stmt:for-semi2 n)
                                             (stmt:for-incr n)
                                             (stmt:for-rparen n)
                                             (stmt:for-body n))))]
    [(stmt:while? n) (cons n (map travel (list (stmt:while-kw n)
                                               (stmt:while-lparen n)
                                               (stmt:while-test n)
                                               (stmt:while-rparen n)
                                               (stmt:while-body n))))]
    [(stmt:do? n) (cons n (map travel (list (stmt:do-kw-do n)
                                            (stmt:do-body n)
                                            (stmt:do-kw-while n)
                                            (stmt:do-lparen n)
                                            (stmt:do-test n)
                                            (stmt:do-rparen n)
                                            (stmt:do-semi n))))]
    [(stmt:return? n) (cons n (map travel (list (stmt:return-kw n)
                                                (stmt:return-expr n)
                                                (stmt:return-semi n))))]
    [(stmt:if? n) (cons n (map travel (list (stmt:if-kw-if n)
                                            (stmt:if-lparen n)
                                            (stmt:if-test n)
                                            (stmt:if-rparen n)
                                            (stmt:if-cons n)
                                            (stmt:if-kw-else n)
                                            (stmt:if-alt n))))]
    [(stmt:switch? n) (cons n (map travel `(,(stmt:switch-kw n)
                                            ,(stmt:switch-lparen n)
                                            ,(stmt:switch-test n)
                                            ,(stmt:switch-rparen n)
                                            ,(stmt:switch-lbrace n)
                                            ,@(stmt:switch-cases n)
                                            ,(stmt:switch-rbrace n))))]
    [(stmt:case? n) (cons n (map travel (list (stmt:case-kw n)
                                              (stmt:case-test n)
                                              (stmt:case-colon n)
                                              (stmt:case-body n))))]
    [(stmt:default? n) (cons n (map travel (list (stmt:default-kw n)
                                                 (stmt:default-colon n)
                                                 (stmt:default-body n))))]
    [(null? n) null] ; empty list
    [else (displayln n) (error "debug!!")]))


(define (get-tokens n)
  (filter is-token?
          (flatten (travel n))))

(define (get-loc obj)
  (map string->number (string-split (symbol->string (list-ref obj 1)) ":")))

(define create-ast
  (lambda (obj)
    (case (car obj)
      ['TokenNode (token (get-loc obj) (third obj))]
      ['TranslationUnitDecl (decl:trans-unit (get-loc obj)
                                             (map create-ast (cddr obj)))]
      ['FunctionDecl (decl:function (get-loc obj)
                                    (create-ast (third obj)) ; return
                                    (create-ast (fourth obj)) ; name
                                    (token '(0 0 0 0) "(")
                                    (create-ast (fifth obj)) ; param
                                    (token '(0 0 0 0) ")")
                                    (if (>= (length obj) 6)
                                        (create-ast (sixth obj))
                                        null))]
      ['CompoundStmt (stmt:comp (get-loc obj)
                                    (create-ast (list-ref obj 2))
                                    (map create-ast
                                         (drop-right (drop obj 3) 1))
                                    (create-ast (last obj)))]
      ['DeclStmt (stmt:decl (get-loc obj)
                            (third obj))]
      ['IfStmt (stmt:if (get-loc obj)
                        (create-ast (third obj)) ; if
                        (token '(0 0 0 0) "(")
                        (create-ast (fourth obj)) ; test
                        (token '(0 0 0 0) ")")
                        (create-ast (fifth obj)) ; cons
                        (if (> (length obj) 5)
                            (create-ast (sixth obj)) ; else
                            null)
                        (if (> (length obj) 5)
                            (create-ast (seventh obj)) ; alt
                            null))]
      ['SwitchStmt (stmt:switch (get-loc obj)
                                (create-ast (third obj)) ; switch
                                (token '(0 0 0 0) "(")
                                (create-ast (fourth obj)) ; test
                                (token '(0 0 0 0) ")")
                                (token '(0 0 0 0) "{")
                                (map create-ast (drop obj 4)) ; cases
                                (token '(0 0 0 0) "}"))]
      ['CaseStmt (stmt:case (get-loc obj)
                            (create-ast (third obj)) ; case
                            (create-ast (fourth obj)) ; test
                            (token '(0 0 0 0) ":")
                            (create-ast (fifth obj)))] ; body
      ['DefaultStmt (stmt:default (get-loc obj)
                                  (create-ast (fourth obj)))]
      ['WhileStmt (stmt:while (get-loc obj)
                              (create-ast (third obj)) ; while
                              (token '(0 0 0 0) "(")
                              (create-ast (fourth obj)) ; test
                              (token '(0 0 0 0) ")")
                              (create-ast (fifth obj)))] ; body
      ['DoStmt (stmt:do (get-loc obj)
                        (create-ast (third obj)) ; do
                        (create-ast (fourth obj)) ; body
                        (create-ast (fifth obj)) ; while
                        (token '(0 0 0 0) "(")
                        (create-ast (sixth obj)) ; test
                        (token '(0 0 0 0) ")")
                        )]
      ['ForStmt (stmt:for (get-loc obj)
                          (create-ast (third obj)) ; for
                          (token '(0 0 0 0) "(")
                          (create-ast (fourth obj)) ; init
                          (token '(0 0 0 0) ";")
                          (create-ast (fifth obj)) ; test
                          (token '(0 0 0 0) ";")
                          (create-ast (sixth obj)) ; incr
                          (token '(0 0 0 0) ")")
                          (create-ast (seventh obj)))] ; body
      ['BreakStmt (stmt:break (get-loc obj))]
      ['ContinueStmt (stmt:continue (get-loc obj))]
      ['ReturnStmt (stmt:return (get-loc obj)
                                (create-ast (third obj)) ; return
                                (if (> (length obj) 3)
                                    (create-ast (fourth obj)) ; expr
                                    null)
                                (token '(0 0 0 0) ";"))]
      ['ExprStmt (stmt:expr (get-loc obj) (third obj))]
      ['Expr (expr (get-loc obj) (third obj))]
      [else (println (car obj)) (error "debug!!!") null])))


(define (print-hier hier-lst [level 0])
  "Print hierarchically"
  (if (list? hier-lst) (begin
                         ;; (println "-----")
                         (map (lambda (x) (print-hier x (add1 level))) hier-lst))
      (displayln (format "~a~a" (make-string level #\space) (describe-node hier-lst)))))

(define (pretty-print-ast ast)
  (print-hier (travel ast)))

(define (create-file->ast-map path)
  "Create AST for a folder of .c and .h code"
  ;; parse a preprocessed c file folder
  ;; for each file, run Helium to dump AST
  ;; create an AST for it
  ;; create a map of filename to AST
  (for/hash ([dir (in-directory path)]
             ;; not a directory
             #:when (and (file-exists? dir)
                         (let ([ext (bytes->string/locale (path-get-extension dir))])
                           (or (string=? ext ".c")
                               (string=? ext ".h")))))
    (values (path->string dir) (create-ast-for-file dir))))

;; (eq? (string->path "hello") (string->path "hello"))

(define (create-ast-for-file path)
  (let ([helium-cmd (format "helium --dump-ast ~a" path)])
    (create-ast
     (read
      (open-input-string
       (with-output-to-string (lambda () (system helium-cmd))))))))


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
;; TEST
;; (define myast (create-ast-for-file "/home/hebi/github/benchmark/craft/grammar/a.c"))
;; (pretty-print-ast myast)

;; (define file->sel-map (load-file->sel-map "/home/hebi/tmp/sel/0.json"))
;; (define file->ast-map (create-file->ast-map "/home/hebi/github/benchmark/craft/prep"))
;; (define sel-set (load-sel file->ast-map file->sel-map))
;; (describe-node myast)
