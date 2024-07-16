(defun c:wzj_ss ()
    (setq i 0)
    (setq flag 1)
    (while  (and (< i 128) (= flag 1)) 
        (progn
            (setq i (+ i 1))
            (if 
                (not (vl-catch-all-error-p
                    (setq tmp
                        (vl-catch-all-apply 
                            '(lambda () (ssget "l"))
                            '()
                        )
                    )
                ))
                (setq a (cons tmp a))
                (setq flag 0)
            )
        )
    )
    (setq i (- i 1))
    (print (strcat "ssets remains: " (itoa i)))
    (print (strcat "ssets used (max 128): " (itoa (- 128 i))))
    (setq a nil)
    (gc)
    (princ)
)
