;;; 打印选择集中所有对象的句柄
;;; @param [in] ss, 选择集
(defun show_ss(ss / i)
       (setq i 0)
       (repeat (sslength ss)
              (setq ent (ssname ss i))
              (print (assoc 5 (entget ent)))
              (setq i (+ i 1))
       )
       (print)
)

