(defun get_block_count ()
  (setq i 0)
  (setq tmp (tblnext "block" T))
  (while tmp
    (setq i (+ i 1))
    (setq tmp (tblnext "block"))
  )

  (print (strcat "The number of block is: " (itoa i)))
  (setq i nil)
  (setq tmp nil)
  (princ)
)