(defvar a (get_arg "a"))
(defvar b (get_arg "b"))
(defvar c (get_arg "c"))
(defvar d (- (* b b) (* 4 a c)))
(defvar x (list 0 0))
(defvar nroots 0)

(set nroots (if ( < d 0) 0 (if ( = d 0) 1 2) ) )

(if (< d 0) (println "Negative discriminant") ())
(set nroots (if (= d 0) 1 (if (> d 0) 2 0)))

(if (>= d 0) ( (lset x 0 (/ (+ (neg b) (sqrt d)) (* 2 a))) (lset x 1 (/ (- (neg b) (sqrt d) ) (* 2 a)))) ())

(print_eol (if (> nroot 0)(lget x 0)))
(print_eol (if (> nroot 1)(lget x 1)))