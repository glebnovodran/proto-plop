(set_domain "Global")
(defvar flg0 0)
(defvar val0 0)
(defvar scl 1.2)
(set val0 (* (+ val0 1 2 3) scl))
(defvar tstName "Baten_Kaitos") ; no spaces in strings
(defvar tstNum -3.14)

(defvar flg (check_flag "Global" "flg0"))
(set val0 (+ val0 (if (not (= flg 0)) 2 (+ 1 3))))
