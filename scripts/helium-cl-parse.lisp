#!/usr/bin/sbcl --script

;; (defun hello-world()
;;   (princ "Hello World\n")
;;   (format t "Hello World~%")
;;   (print sb-ext:*posix-argv*)
;;   (format t "~%"))

;; (hello-world)

;; read from standard input
;; output to standard output


;; 1. it define a data-seg-loc function, accessors
;; 2. the symbol data becomes a type name
;; 3. data-p is defined
;; 4. make-data is defined, with &key all-slots
;; (defstruct data
;;   seg-loc
;;   proc-num
;;   branch-num
;;   loop-num
;;   compilible
;;   var-num
;;   test-num
;;   gen-time
;;   total-time
;;   stmt-cov
;;   branch-cov
;;   pass-num
;;   fail-num
;;   total-reach-poi
;;   total-fail-poi)



;; (defclass bank-account ()
;;   ((customer-name
;;     :initarg :customer-name)
;;    (balance
;;     :initarg :balance
;;     :initform 0)))

;; (defgeneric balance (account))

;; (defmethod balance ((account bank-account))
;;   (slot-value account 'balance))


(defclass data()
  ((seg-loc :accessor seg-loc :initform 0)
   (proc-num :accessor proc-num :initform 0)
   (branch-num :accessor branch-num :initform 0)
   (loop-num :accessor loop-num :initform 0)
   (compilible :accessor compilible :initform nil)
   (var-num :accessor var-num :initform 0)
   (test-num :accessor test-num :initform 0)
   (gen-time :accessor gen-time :initform 0.0)
   (total-time :accessor total-time :initform 0.0)
   (stmt-cov :accessor stmt-cov :initform "")
   (branch-cov :accessor branch-cov :initform "")
   (pass-num :accessor pass-num :initform 0)
   (fail-num :accessor fail-num :initform 0)
   (total-reach-poi :accessor total-reach-poi :initform 0)
   (total-fail-poi :accessor total-fail-poi :initform 0)))

(defgeneric print-header(the-data)
  (:documentation 
   "This is the prototype. No implementation defined here."))
(defgeneric print-data(data)
  (:documentation "This is also the prototype"))

(defmethod print-header((the-data data))
  ;; ~^ will remove the last comma, nice
  (format t "~{~A~^,~}~%"
          '(seg-loc proc-num branch-num loop-num
            compilible var-num test-num gen-time
            total-time stmt-cov branch-cov
            pass-num fail-num total-reach-poi total-fail-poi)))

(defmethod print-data((data data))
  (format t "printing data: ~%")
  (format t "~{~A~^,~}~%"
          `(,(seg-loc data)
             ,(proc-num data)
             ,(branch-num data)
             ,(loop-num data)
             ,(compilible data)
             ,(var-num data)
             ,(test-num data)
             ,(gen-time data)
             ,(total-time data)
             ,(stmt-cov data)
             ,(branch-cov data)
             ,(pass-num data)
             ,(fail-num data)
             ,(total-reach-poi data)
             ,(total-fail-poi data)
             )))

(defun get-second(line)
  "Get the second component of the line splitted by colon"
  (subseq line (1+ (position #\: line))))

(defun test-defun()
  )

(defmacro test-macro(aa bb)
  (loop for b in bb
     collect `(b))
  `(hello world))
(macroexpand-1 '(test-macro aa bb))


(defmacro do-primes-a ((var start end) &body body)
  (append '(do)
          (list  (list (list var
                             (list 'next-prime start)
                             (list 'next-prime (list '1+ var)))))
          (list (list (list '> var end)))
          body))

(macroexpand-1 '(do-primes (p 0 19) (format t "~d " p)))

(defmacro fill-data(line data pairs)
  (loop for pair in pairs
     for match = (car pair)
     for accessor = (cdr pair)
     collect `((search ,match ,line) (setf (,accessor ,data) (get-second ,line)))))
  ;; (loop for pair in pairs
  ;;      pair))
       ;; (let ((match (car pair))
       ;;       (accessor (cdr pair)))
;;   collect `((search ,match ,line) (setf (,accessor ,data) (get-second ,line))))))

(macroexpand-1
 '(fill-data line mydata
            (("Segment Size" . seg-loc)
             ("Procedure Number" . proc-num)
             ("Branch Number" . branch-num))))


(defun main()
  (let ((mydata (make-instance 'data))) ;; the data
    (print-header mydata)
    (with-open-file (stream
                     "/home/hebi/github/helium/scripts/cabextract-1.2.random.output.txt"
                     ;; "cabextract-1.2.random.output.txt"
                     )
      ;; *standard-input*
      (loop for line = (read-line stream)
         while line do
         ;; (format t "~a~%" line)
           (cond
             ((search "Processing query" line)
              ;; dump the data and clear the data
              (print-data mydata)
              (setf mydata (make-instance 'data)))
             (fill-data line mydata
                        '(("Segment Size" . seg-loc)
                          ("Procedure Number" . proc-num)
                          ("Branch Number" . branch-num)))
             ;; ((fill-data "Segment Size" line seg-loc mydata))
             ;; ((search "Segment Size" line)
             ;;  (setf (seg-loc mydata) (get-second line)))
             ;; ((search "Procedure Number" line)
             ;;  (setf (proc-num mydata) (get-second line)))
             ;; ((search "Branch Number" line)
             ;;  (setf (branch-num mydata) (get-second line)))
             ;; ((search "Loop Number" line)
             ;;  (setf (loop-num mydata) (parse-integer (get-second line))))
             ;; ((search "Compile" line)
             ;;  (setf (compilible mydata) (get-second line)))
             ;; ((search "Number of input" line)
             ;;  (setf (var-num mydata) (get-second line)))
             ;; ((search "Number of tests" line)
             ;;  (setf (test-num mydata) (get-second line)))
             ;; ((search "Total Testing Time" line)
             ;;  (setf (total-time mydata) (get-second line)))
             ;; ((search "Stmt Coverage" line)
             ;;  (setf (stmt-cov mydata) (get-second line)))
             ;; ((search "Branch Coverage" line)
             ;;  (setf (branch-cov mydata) (get-second line)))
             ;; ((search "Number of Pass Test" line)
             ;;  (setf (pass-num mydata) (get-second line)))
             ;; ((search "Number of Fail Test" line)
             ;;  (setf (fail-num mydata) (get-second line)))
             ;; ((search "Total reach poi" line)
             ;;  (setf (total-reach-poi mydata) (get-second line)))
             ;; ((search "Total fail poi" line)
             ;;  (setf (total-fail-poi mydata) (get-second line)))
             (t))))))

(main)
