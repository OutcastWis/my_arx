;;; ��ӡѡ�������ж���ľ��
;;; @param [in] ss, ѡ��
(defun show_ss(ss / i)
       (setq i 0)
       (repeat (sslength ss)
              (setq ent (ssname ss i))
              (print (assoc 5 (entget ent)))
              (setq i (+ i 1))
       )
       (print)
)

