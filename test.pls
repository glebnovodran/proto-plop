(set_domain "Global")
(defvar flg0 0)
(defvar val0 0)
(defvar scl 1.2)
(set val0 (* (+ val0 1 2 3) scl))
(defvar val1 1)
(min val1 val0 2 4 8)
(max val1 0)
(defvar tstName "Baten Kaitos") ; Baten Kaitos is now with a space
(defvar tstNum -3.14)
(defvar tstName1 "Baten (Kaitos)") ; 
(defvar tstName2 "Ar Nosurge (Ra) (Ciela)")
(defvar flg (check_flag "Global" "flg0"))
(set val0 (+ val0 (if (not (= flg 0)) 2 (+ 1 3))))
