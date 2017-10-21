;; Back up files

;; old sel-file format

(define (load-file2sel sel-file)
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

