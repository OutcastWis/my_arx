
(vl-load-com)

; 由界面上的INSERT, 获取所有块, 列出块名
(defun wzj_list_blocks (/ i x x_len block_name block_ref)
    (setq x (ssget "X" '((0 . "INSERT"))))
    (setq x_len (sslength x))
    (setq i 0)
    (repeat x_len
        (setq block_ref (ssname x i))
        (setq i (1+ i))
        
        (setq block_name (cdr (assoc 2 (entget block_ref))))
        (print block_name)
    )

    (princ)
)


; 返回块中对象的数量
(defun wzj_block_count (bname)
    (setq blocks (vla-get-Blocks (vla-get-ActiveDocument (vlax-get-acad-object))))

    (setq i 0)
    (setq count (vla-get-Count blocks))
    (while (< i count)
        (setq b (vla-Item blocks i))
        (setq i (1+ i))
        (if (= (vla-get-Name b) bname)
            (setq i (+ i count))
        )
    )

    (if (= i count) 
        (print (strcat "Not found blocks called: " bname)) 
        (progn
            (setq count (vla-get-Count b))
            (print (strcat "The count of entities in block is: " (itoa count)))
        )
    )

    (princ)
) 


; 炸开块定义bname中的嵌套块. 本质是修改块定义
(defun wzj_explode_block (bname / i b blocks count explode_ents next)
    (setq blocks (vla-get-Blocks (vla-get-ActiveDocument (vlax-get-acad-object))))

    (setq i 0)
    (setq count (vla-get-Count blocks))
    (while (< i count)
        (setq b (vla-Item blocks i))
        (setq i (1+ i))
        (if (= (vla-get-Name b) bname)
            (setq i (+ i count))
        )
    )
   
    (if (= i count) 
        (print (strcat "Not found blocks called: " bname)) 
        (progn  ; 找打了块定义b
            (setq count (vla-get-Count b))
            (print (strcat "The count of entities in block is: " (itoa count)))

            (setq _cnt 1)
            (while (> _cnt 0)
                (setq next (list))
                (setq _cnt 0)
                ; 子对象炸开, 会直接影响块本身. 所以这边使用b就行
                (vlax-for i b
                    (if (= "AcDbBlockReference" (vla-get-ObjectName i))
                    ;(if (vlax-method-applicable-p i 'Explode) ; 不要使用这个, 因为Polyline, Region等也有Explode函数, 会它们导致炸开, 并且变得很慢
                        (progn
                            (setq _cnt (1+ _cnt))
                            ; vla-Explode返回的是variant类型. 使用vlax-variant-value获取内置的safearray
                            ; (vlax-safearray->list (vlax-variant-value (vla-Explode i)))
                            (vla-Explode i)
                            (vla-delete i)
                        )
                    )
                )
                (print (strcat "block refs finded: " (itoa _cnt)))
                (print (strcat "after explode, has sub entities: " (itoa (vla-get-Count b))))
            )
        )
    )
    (print "done")
    (princ)
)



; 显示块中块, 仅一层
(defun wzj_sub_block_in_block (bname / blocks i count b)
    (setq blocks (vla-get-Blocks (vla-get-ActiveDocument (vlax-get-acad-object))))

    (setq i 0)
    (setq count (vla-get-Count blocks))
    (while (< i count)
        (setq b (vla-Item blocks i))
        (setq i (1+ i))
        (if (= (vla-get-Name b) bname)
            (setq i (+ i count))
        )
    )

    (if (= i count) 
        (print (strcat "Not found blocks called: " bname)) 
        (progn
            (setq count (vla-get-Count b))
            (print (strcat "The count of entities in block is: " (itoa count)))

            (vlax-for i b
                (if (= "AcDbBlockReference" (vla-get-ObjectName i))
                    (print (vla-get-Name i))
                )
            )
        )
    )

    (print "done")
    (print)
)


; 炸开所有块中的标注
(defun wzj_explode_dim_in_block ()
    (setq _blocks (vla-get-Blocks (vla-get-ActiveDocument (vlax-get-acad-object))))
    (vlax-for _b _blocks
        (setq _bname (vla-get-Name _b))
        (if (not (wcmatch _bname "`**"))  
            (progn
                (print (strcat "In block: " _bname))
                (command ".bedit" _bname)
                (setq _ents (ssget "_X" '((0 . "*DIM*"))))
                ; 画线, 然后删除. 确保bclose的时候一定需要参数
                (command ".line" "1,1,0" "0,0,0" "")
                (entdel (entlast))
                ; ssget没有选中对象, 则返回nil
                (if (not (null _ents))                  
                    (foreach _i (mapcar 'cadr (ssnamex _ents))
                        (command ".explode" _i)
                    )                  
                )
                (command "bclose" "S")
            )
        )
    )

    (print "done")
    (princ)
)



; 读取外部文件, 转为dxf
(defun wzj_to_dxf (dir)
    (vlax-import-type-library
    :tlb-filename "C:\\wzj\\git_clone\\chintcourtyardautodesign\\doc\\ObjectARX\\R24\\inc-x64\\axdb24enu.tlb" ;; change to correct path if needed
    :methods-prefix "acdbm-"
    :properties-prefix "acdbp-"
    :constants-prefix "acdbc-")
  
    (setq _files (vl-directory-files dir "*.dwg" 1))
    (foreach _i _files
        (setq _i (strcat dir "\\" _i))
        (setq _drawing (vlax-create-object "objectDBX.AxDbDocument.24"))
        (acdbm-open _drawing _i)
        (vlax-invoke-method _drawing 'DxfOut (vl-string-subst ".dxf" ".dwg" _i))
        (vlax-release-object _drawing)
        (print (strcat _i "  =>  Ok"))
    )

    (print "done") 
    (princ)
)


; 炸开预选集中的所有INSERT, 以及内嵌的INSERT
(defun wzj_ss_explode_insert ()
    (setq _ss (ssget "I"))
    (setq _len (sslength _ss))
    (setq _i 0)
    (repeat _len
        (setq _ent (ssname _ss _i))
        (setq _i (1+ _i))
      
        (if (= "INSERT" (cdr (assoc 0 (entget _ent))))
            (progn
                (setq _o (vlax-ename->vla-object _ent))
                (setq _cur (list _o))
                (setq _nxt (list))
                ; 循环炸开
                (while (> (length _cur) 0)
                    (foreach _o _cur
                        (if (= "AcDbBlockReference" (vla-get-ObjectName _o))
                            (setq _subs 
                                ; vla-Explode返回的是variant类型. 使用vlax-variant-value获取内置的safearray
                                (vlax-safearray->list (vlax-variant-value (vla-Explode _o)))
                            )
                            (setq _subs nil)
                        )
                        (setq _nxt (append _nxt _subs))
                    )
                    (print (strcat "after explode, add sub entities: " (itoa (length _nxt))))
                    (setq _cur _nxt)
                    (setq _nxt (list))
                )
                (entdel _ent) ; 删除源对象
            )
        )
    )
    
    (print "done")
    (princ)
)