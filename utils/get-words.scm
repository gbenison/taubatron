;
; extract all the words from a file
; and print them out, one per line.
;

(use-modules (ice-9 rdelim)
	     (srfi srfi-1))
(debug-set! stack 1000000)

(define infile (open-input-file (cadr (command-line))))

(define (string-filter str)
  (list->string
   (filter char-alphabetic? (string->list str))))

(define (grab-line)
  (let ((line (read-line infile)))
    (if (eof-object? line)
	#f
	(filter (lambda (str)(not (string=? str "")))
		(map string-filter (string-split (string-downcase line) #\ ))))))

(define (string-longer? n)
  (lambda (str)
    (> (string-length str) n)))

(define (delete-adjacent-duplicates lst)
  (let ((this (car lst))
	(rest (cdr lst)))
    (cond ((null? rest) lst)
	  ((equal? this (car rest))
	   (delete-adjacent-duplicates rest))
	  (else (cons this (delete-adjacent-duplicates rest))))))

(define all-words
  (let loop ((result '()))
    (let ((next (grab-line)))
      (if next
	  (loop (append result next))
	  (delete-adjacent-duplicates
	   (filter (string-longer? 2)
		   (sort result string<)))))))

(define (write-line x)
  (display x)
  (newline))

(for-each write-line all-words)


