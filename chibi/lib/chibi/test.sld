
(define-library (chibi test)
  (export
   test test-error test-assert test-not test-values
   test-group current-test-group
   test-begin test-end ;; test-syntax-error ;; test-info
   ;; test-vars test-run ;; test-exit
   current-test-verbosity current-test-epsilon current-test-comparator
   current-test-applier current-test-handler current-test-skipper
   current-test-group-reporter test-failure-count
   current-test-epsilon current-test-comparator)
  (import (scheme) (srfi 39) (srfi 98) (chibi time) (chibi ast))
  (include "test.scm"))
