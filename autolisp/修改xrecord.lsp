#xrecord的修改=删除 + 添加新的
(setq xrec '((0 . "XRECORD") (100 . "AcDbXrecord") (280 . 1) (90 . 2) (90 . 1) (90 . 0) (300 . "") (300 . "新")))
(setq xname (entmakex xrec))
(distsearch (namedobjdict) "_Model_Object_Dict_Name")  # 获取句柄
(setq mdic (handent "C344")
(dictadd mdic "1" xname)

