()
(nop)
(1 2 3)
(defvar vec1 (+ (list 1 2 3) (list 4 5 6)))
(defvar vec1 (* 2.0 (list 1 2 3)))
(defvar vec1 (+ (list 1 2 3) (list 4 5 6) (list 7 8 9)))
(set vec1 (dot vec1 vec1))
((print "Ahoj") (print "42"))
(nop (+ 1 1) (print "Ahoj") )
( (+ 1 1) (print "Ahoj") )
(if 1 ((print "1") (print "<--")) ())

(set_domain "Global")
(defvar flg0 0)
(defvar val0 0)
(defvar val1 (neg val0))
(defvar scl 1.2)
(set val0 (* (+ val0 1 2 3) scl))
(defvar val1 1)
(min val1 val0 2 4 8)
(max val1 0)
(defvar tstName "Baten Kaitos") ; Baten Kaitos is now with a space
(defvar tstNum -3.14)
(defvar tstName1 "Baten (Kaitos)") ; 
(defvar tstName2 "Ar Nosurge (Ra) (Ciela)")
(defvar tstName3 tstName1)
(set tstName1 tstName2)
(defvar flg (check_flag "Global" "flg0"))
(set val0 (+ val0 (if (not (= flg 0)) 2 (+ 1 3))))

(defvar vec0 (list 0 1 (+ 1 val0)))
;(if setZero (set val0 0) (nop))
(lset vec0 3 777)
(set val1 (lget vec0 3))
(set val1 (lget vec0 (+ 1 1 1)))
(set val0 (if flg0 1 0))

(defvar mtype 0.0)
(print mtype)
(set mtype "valkyria")
(print mtype)
(set mtype (list "Grichka" 2.0))
(print mtype)


(+ val0 "123") ; this should produce a runtime error

(defvar llst (list (list 1 2 3) (list 4 5 6) "Chacha" "Grichka") )