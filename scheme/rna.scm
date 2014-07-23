(begin

  ;; RNA
  ;; Vars:
  ;;  has-fs-bond (1 bit), fs-bond-dir (3 bits), has-rs-bond (1 bit), rs-bond-dir (3 bits),
  ;;  has-fa-bond (1 bit), fa-bond-dir (3 bits), has-ra-bond (1 bit), ra-bond-dir (3 bits)
  ;;  sense-base (2 bits), anti-base (2 bits), has-anti (1 bit)
  ;; Registers:
  ;;  (0,1)  location of fs-bond cell
  ;;  (2,3)  direction & inverse-direction from origin to fs-bond cell
  ;;  (4,5)  direction & inverse-direction from target cell to fs-bond cell
  ;;  (6,7)  location of rs-bond cell
  ;;  (8,9)  direction & inverse-direction from origin to rs-bond cell
  ;; (10,11) direction & inverse-direction from target cell to rs-bond cell
  ;; (12,13) location of fa-bond cell
  ;; (14,15) direction & inverse-direction from origin to fa-bond cell
  ;; (16,17) direction & inverse-direction from target cell to fa-bond cell
  ;; (18,19) location of ra-bond cell
  ;; (20,21) direction & inverse-direction from origin to ra-bond cell
  ;; (22,23) direction & inverse-direction from target cell to ra-bond cell
  ;; (24,25) location of target cell for move
  ;;  26     complement of sense base

  (define rna-has-fs-bond-var "has-fs-bond")
  (define rna-has-rs-bond-var "has-rs-bond")
  (define rna-fs-bond-dir-var "fs-bond-dir")
  (define rna-rs-bond-dir-var "rs-bond-dir")

  (define rna-has-fa-bond-var "has-fa-bond")
  (define rna-has-ra-bond-var "has-ra-bond")
  (define rna-fa-bond-dir-var "fa-bond-dir")
  (define rna-ra-bond-dir-var "ra-bond-dir")

  (define rna-sense-base-var "sense-base")
  (define rna-anti-base-var "anti-base")
  (define rna-has-anti-var "has-anti")

  (define rna-merge-mismatch-prob .01)
  (define rna-detach-prob .01)

  (define (rna-particle name)
    `(particle
      (name ,name)
      (vars
       (varsize (name ,rna-has-fs-bond-var) (size 2))
       (varsize (name ,rna-fs-bond-dir-var) (size 3))
       (varsize (name ,rna-has-rs-bond-var) (size 2))
       (varsize (name ,rna-rs-bond-dir-var) (size 3))
       (varsize (name ,rna-has-fa-bond-var) (size 2))
       (varsize (name ,rna-fa-bond-dir-var) (size 3))
       (varsize (name ,rna-has-ra-bond-var) (size 2))
       (varsize (name ,rna-ra-bond-dir-var) (size 3))
       (varsize (name ,rna-sense-base-var) (size 2))
       (varsize (name ,rna-anti-base-var) (size 2))
       (varsize (name ,rna-has-anti-var) (size 1)))

      (colrule (var ,rna-sense-base-var) (hexmul "20000"))
      (colrule (var ,rna-anti-base-var) (hexmul "80000") (hexinc "14ffff"))
;      (rule (scheme "(rna-move-rule)"))
))


  ;; Top level:
  ;; Load register 26 with complement of sense base
  ;; Goto attempt-move

  ;; attempt-move:
  ;; If (has-anti)
  ;;  ...with probability (complementary ? p_detach_match : p_detach_mismatch)...
  ;;    ...select sense/antisense at random...
  ;;    ...call attempt-detach with appropriate var labels
  ;;  else (regular sense+antisense rna-bond-cascade)
  ;; else (sense-only rna-bond-cascade)


  ;; (attempt-detach has-fwd fwd-dir has-rev rev-dir base has-fwd-other fwd-dir-other has-rev-other rev-dir-other base-other)
  ;; ...do an rna-bond-cascade *exclusively for sense or antisense*, creating a candidate-nbr-dirs for that side...
  ;; ...attempt detach, moving into target cell if it is empty.


  ;; helper functions for iterating over all possible combinations of bonds (a "cascade")
  ;; argument list of cascade-func is as follows:
  ;;  (cascade-func tag has-bond-var bond-dir-var partner-has-bond-var partner-bond-dir-var bond-base-reg next-func)
  ;; returns a function that takes arg list similar to init-args
  ;; final-func also takes arg list similar to init-args
  ;; antisense-func-maker takes a function of type cascade-func for the antisense part of the cascade,
  ;; and returns a similar function that can optionally stop at antisense (e.g. to do a detach move)
  (define (ds-or-as-cascade cascade-func final-func antisense-func-maker init-args)
    (apply
     (as-cascade-func
      cascade-func
      (antisense-func-maker
       (ss-cascade-func cascade-func final-func)))
     init-args))

  (define (ds-cascade cascade-func final-func init-args)
    (ds-or-as-cascade cascade-func final-func (lambda (ss-cascade) ss-cascade) init-args))

  (define (diverted-ds-cascade cascade-func final-ds-func final-antisense-func antisense-prob init-args)
    (ds-or-as-cascade
     cascade-func final-ds-func
     (lambda (ss-cascade-func)
       (lambda args
	 (apply-random-switch
	  `((,antisense-prob ,(apply final-antisense-func args))
	    (,(- 1 antisense-prob) ,(apply ss-cascade-func args))))))
     init-args))

  (define (sense-cascade cascade-func final-func init-args)
    (apply
     (ss-cascade-func cascade-func final-func)
     init-args))

  (define (antisense-cascade cascade-func final-func init-args)
    (apply
     (as-cascade-func cascade-func final-func)
     init-args))

  (define (as-cascade-func cascade-func final-func)
    (cascade-func
     "-ra"
     rna-has-ra-bond-var
     rna-ra-bond-dir-var
     rna-has-fs-bond-var
     rna-fs-bond-dir-var
     rna-has-fa-bond-var
     rna-fa-bond-dir-var
     rna-rs-bond-dir-var
     2
     18
     (cascade-func
      "-fa"
      rna-has-fa-bond-var
      rna-fa-bond-dir-var
      rna-has-rs-bond-var
      rna-rs-bond-dir-var
      rna-has-ra-bond-var
      rna-ra-bond-dir-var
      rna-fs-bond-dir-var
      1
      12
      final-func)))

  (define (ss-cascade-func cascade-func final-func)
    (cascade-func
     "-rs"
     rna-has-rs-bond-var
     rna-rs-bond-dir-var
     rna-has-fs-bond-var
     rna-fs-bond-dir-var
     rna-has-fa-bond-var
     rna-fa-bond-dir-var
     rna-ra-bond-dir-var
     2
     6
     (cascade-func
      "-fs"
      rna-has-fs-bond-var
      rna-fs-bond-dir-var
      rna-has-rs-bond-var
      rna-rs-bond-dir-var
      rna-has-ra-bond-var
      rna-ra-bond-dir-var
      rna-fa-bond-dir-var
      1
      0
      final-func)))

  ;; create the cascade for the top-level rule
  (define (rna-move-subrule)
    (let ((init-args `("try-random-step" "" () ,moore-dirs)))
      (subrule
       "rna-move-subrule"
       (switch-var
	self self-type rna-has-anti-var
	`((0 
	   ,(ss-cascade
	     rna-bond-cascade rna-drift-rule init-args))
	  (1
	   ,(switch-var
	     self self-type rna-sense-base-var
	     (map
	      (lambda (sense-base)
		(load-rule
		 `((26 ,(- 3 sense-base)))
		 (goto "rna-ds-move-subrule")))))))))
      (subrule
       "rna-ds-move-subrule"
       (diverted-ds-cascade
	rna-bond-cascade rna-drift-rule rna-detach-rule rna-detach-prob init-args))))

  ;; TODO: write rna-detach-rule

  ;; rna-bond-cascade: the function for creating the bond verification cascade
  ;; if has-bond-var is TRUE, verify that the bond is mutual, and add to confirmed-bond-list
  ;; then pass control to next-in-cascade
  (define (rna-bond-cascade
	   tag
	   has-bond-var bond-dir-var
	   sense-partner-has-bond-var sense-partner-bond-dir-var
	   anti-partner-has-bond-var anti-partner-bond-dir-var
	   merged-bond-dir-var
	   expected-partner-has bond-base-reg next-in-cascade)
    (lambda (subrule-prefix subrule-suffix confirmed-bond-list candidate-nbr-dirs)
      (switch-var
       origin self-type has-bond-var
       `((0 ,(next-in-cascade subrule-prefix subrule-suffix confirmed-bond-list candidate-nbr-dirs))
	 (1 ,(bind-and-verify tag 0 bond-dir-var
			      sense-partner-has-bond-var sense-partner-bond-dir-var
			      expected-partner-has bond-base-reg next-in-cascade
			      subrule-prefix subrule-suffix confirmed-bond-list candidate-nbr-dirs))
	 (2 ,(bind-and-verify tag 1 bond-dir-var
			      anti-partner-has-bond-var anti-partner-bond-dir-var
			      expected-partner-has bond-base-reg next-in-cascade
			      subrule-prefix subrule-suffix confirmed-bond-list candidate-nbr-dirs))))))

  (define (make-connect-tag goes-to-anti)
    (if (goes-to-anti) "2a" "2s"))

  (define (bind-and-verify tag goes-to-anti bond-dir-var partner-has-bond-var partner-bond-dir-var
			   expected-partner-has-bond-var bond-base-reg next-in-cascade
			   subrule-prefix subrule-suffix confirmed-bond-list candidate-nbr-dirs)
    (let ((goes-to-anti-tag (make-connect-tag goes-to-anti)))
      (bind-moore-dir
       bond-dir-var
       (lambda (loc dir)
	 (let* ((inv-dir (moore-back dir)))
	   ;; TODO: bond verification goes here
	   (next-in-cascade
	    subrule-prefix
	    (string-append subrule-suffix tag goes-to-anti-tag)
	    (cons (list bond-dir-var loc dir inv-dir bond-base-reg partner-has-bond-var partner-bond-dir-var)
		  confirmed-bond-list)
	    (grep
	     (lambda (nbr-dir)
	       (moore-neighbor? (loc-minus loc (moore-loc nbr-dir)))) candidate-nbr-dirs)))))))


  ;; rna-drift-rule(confirmed-bond-list,candidate-nbr-dirs): the rule at the bottom of the bond verification cascade
  ;;  select random neighbor, load registers, jump to appropriate subrule
  (define (rna-drift-rule subrule-prefix subrule-suffix confirmed-bond-list candidate-nbr-dirs)
    (apply-random-switch
     (map
      (lambda (move-dir)
	(let* ((move-loc (moore-loc move-dir)))
	  `((1 ,(load-bond-and-target-registers
		 candidate-nbr-dirs
		 ,(string-concatenate
		   subrule-prefix
		   subrule-suffix)))))))))

  (define (load-bond-and-target-registers candidate-nbr-dirs rule-name)
    (load-rule
     (append
      `(24 ,(car move-loc))
      `(25 ,(cadr move-loc))
      (map (lambda (dirvar-loc-dir-invdir)
	     (let* ((bond-dir-var (car dirvar-loc-dir-invdir)) ;0 bond-dir-var
		    (loc (cadr dirvar-loc-dir-invdir)) ;1 loc
		    (dir (caddr dirvar-loc-dir-invdir)) ;2 dir
		    (inv-dir (cadddr dirvar-loc-dir-invdir)) ;3 inv-dir
		    (bond-base-reg (caddddr dirvar-loc-dir-invdir)) ;4 bond-base-reg
					;5 partner-has-bond-var
					;6 partner-bond-dir-var
		    (new-dir (moore-dir (loc-minus move-loc loc)))
		    (new-inv-dir (moore-back new-dir)))
	       `((,bond-base-reg ,(car loc))
		 (,(+ bond-base-reg 1) ,(cadr loc))
		 (,(+ bond-base-reg 2) ,dir)
		 (,(+ bond-base-reg 3) ,inv-dir)
		 (,(+ bond-base-reg 4) ,new-dir)
		 (,(+ bond-base-reg 5) ,new-inv-dir))))
	   candidate-nbr-dirs))
     `(goto rule-name)))


  ;; random-step-XX: (where XX is concatenation of all dir-vars for which bonds are present)
  ;;   Switch on move target:
  ;;    (empty) move into it
  ;;    (contains RNA with no anti) merge with probability (complementary ? p_attach_match : p_attach_mismatch)

  ;; random-step-subrule-cascade: the function for creating the various drift rules
  (define (random-step-subrule-cascade
	   tag
	   has-bond-var bond-dir-var
	   sense-partner-has-bond-var sense-partner-bond-dir-var
	   anti-partner-has-bond-var anti-partner-bond-dir-var
	   merged-bond-dir-var
	   expected-partner-has-bond-var bond-base-reg next-in-cascade)
    (let ((goes-to-anti-tag (make-connect-tag 1))
	  (goes-to-sense-tag (make-connect-tag 0)))
      (lambda (subrule-prefix subrule-suffix self-has-anti confirmed-bond-reg-list)
	(begin
	  (next-in-cascade
	   subrule-prefix
	   (string-append subrule-suffix tag goes-to-sense-tag)
	   self-has-anti
	   (cons (list has-bond-var bond-dir-var sense-partner-has-bond-var sense-partner-bond-dir-var merged-bond-dir-var bond-base-reg)
		 confirmed-bond-reg-list))
	  (next-in-cascade
	   subrule-prefix
	   (string-append subrule-suffix tag goes-to-anti-tag)
	   self-has-anti
	   (cons (list has-bond-var bond-dir-var anti-partner-has-bond-var anti-partner-bond-dir-var merged-bond-dir-var bond-base-reg)
		 confirmed-bond-reg-list))
	  (next-in-cascade subrule-prefix subrule-suffix self-has-anti confirmed-bond-reg-list)))))

  ;; cascade to define drift rules & associated subrules
  (ds-cascade random-step-subrule-cascade make-random-step-rule `("try-random-step-ds" "" 1 ()))  ;; double-stranded
  (sense-cascade random-step-subrule-cascade make-random-step-rule `("try-random-step-ss" "" 0 ()))  ;; single-stranded

  ;; the function at the bottom of the drift rule cascade
  (define rna-merge-subrule-name "rna-merge-subrule")
  (define (merge-bonds confirmed-bond-reg-list)
    (if
     (null? confirmed-bond-reg-list)
     nop-rule
     (let* ((confirmed-bond-reg (car confirmed-bond-reg-list))
	    (has-bond-var (car confirmed-bond-reg))
	    (bond-dir-var (cadr confirmed-bond-reg))
	    (partner-has-bond-var (caddr confirmed-bond-reg))
	    (partner-bond-dir-var (cadddr confirmed-bond-reg))
	    (merged-bond-dir-var (caddddr confirmed-bond-reg))
	    (bond-base-reg (cadddddr confirmed-bond-reg))
	    (rest-of-confirmed-bond-reg-list (cdr confirmed-bond-reg-list))
	    (merge-remaining-bonds (merge-bonds rest-of-confirmed-bond-reg-list)))
       (indirect-set-var-from-register
	'(24 25)
	self-type merged-bond-dir-var (+ bond-base-reg 4)
	(indirect-set-var-from-register
	 (list bond-base-reg (+ bond-base-reg 1))
	 self-type partner-bond-dir-var (+ bond-base-reg 5))
	merge-remaining-bonds))))

  (define (make-random-step-rule subrule-prefix subrule-suffix self-has-anti confirmed-bond-reg-list)
    (let* ((subrule-name (string-append subrule-prefix subrule-suffix))
	   (qualified-merge-subrule-name (string-append rna-merge-subrule-name subrule-suffix)))
      (begin
	(subrule
	 subrule-name
	 (indirect-switch-type  ;; Switch on move target
	  '(24 25)
	  `((,empty-type   ;; (empty) move into it
	     (update-registers confirmed-bond-reg-list (,indirect-move-self '(24 25))))
	    (,self-type    ;; (contains RNA)
	     ,(if
	       ,self-has-anti
	       nop-rule
	       ;; TODO: allow unbonded particles to form new bonds
	       (indirect-switch-var
		'(24 25) self-type rna-has-anti-var
		`(0   ;; (no antisense base in source or target cell)
		  ,(indirect-compare-var-to-register
		    '(24 25) self-type rna-sense-base 26
		    `(eq ,qualified-merge-subrule-name)  ;; complementary: merge
		    `(neq  ;; non-complementary: merge with probability rna-merge-mismatch-prob
		      ,(random-rule
			rna-merge-mismatch-prob
			qualified-merge-subrule-name))))))))))
	;; merge subrule
	(subrule
	 qualified-merge-subrule-name
	 (copy-self-var-to-indirect
	  self-type rna-sense-base-var '(24 25) self-type rna-anti-base-var
	  (copy-self-var-to-indirect
	   self-type rna-has-fs-bond-var '(24 25) self-type rna-has-fa-bond-var
	   (copy-self-var-to-indirect
	    self-type rna-has-rs-bond-var '(24 25) self-type rna-has-ra-bond-var
	    (merge-bonds confirmed-bond-reg-list))))))))

  ;; function to update bond vars in a drift move
  (define (update-registers confirmed-bond-reg-list next-rule)
    (if
     (null? confirmed-bond-reg-list)
     next-rule
     (let* ((confirmed-bond (car confirmed-bond-reg-list))
	    (has-bond-var (car confirmed-bond))
	    (bond-dir-var (cadr confirmed-bond))
	    (partner-has-bond-var (caddr confirmed-bond))
	    (partner-bond-dir-var (cadddr confirmed-bond))
	    (bond-base-reg (caddddr confirmed-bond))
	    (rest-of-confirmed-bond-reg-list (cdr confirmed-bond-reg-list)))
       (update-registers
	rest-of-confirmed-bond-reg-list
	(set-var-from-register
	 origin self-type bond-dir-var (+ bond-base-reg 4)
	 (indirect-set-var-from-register
	  (list bond-base-reg (+ bond-base-reg 1))
	  self-type partner-bond-dir-var (+ bond-base-reg 5)
	  next-rule))))))
)
