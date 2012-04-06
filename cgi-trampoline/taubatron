#!/bin/sh

exec guile -s $0 $@

FIXME this suffers from the send-response-line-twice bug of CGI scripts

!#

(use-modules (srfi srfi-1)
	     (web http)
	     (web request)
	     (web response)
	     (web uri)
	     (web client)
	     (ice-9 rw)
	     (rnrs bytevectors))

(define tt-server-socket-name "/tmp/test.sock")

(define (header-from-env sym env-var)
  (and (getenv env-var)
       (parse-header sym (getenv env-var))))

(define headers
  (fold
   (lambda (sym env-var result)
     (let ((header (header-from-env sym env-var)))
       (if header (cons (cons sym header) result) result)))
   '()
   '(content-length    content-type)
   '("CONTENT_LENGTH" "CONTENT_TYPE")))

;; Any input to read? Grab it.
(define content #f)
(if (getenv "CONTENT_LENGTH")
    (begin
      (set! content (make-string (string->number (getenv "CONTENT_LENGTH"))))
      (read-string!/partial content)))

(define uri (build-uri 'http
		       #:host "localhost"
		       #:path "/taubatron"))

(define request
  (build-request uri
		 #:method (getenv "REQUEST_METHOD")
		 #:headers headers))

(define (make-socket)
  (let ((result (socket PF_UNIX SOCK_STREAM 0)))
    (connect result AF_UNIX tt-server-socket-name)
    result))

(define (send-request uri content socket)
  (let ((request (write-request request socket)))
    (if content
	(write-request-body request
			    (string->utf8 content))))
  (force-output socket)
  (read-response socket))

(let* ((response (send-request uri content (make-socket)))
       (body (read-response-body response)))
  (write-headers (response-headers response) (current-output-port))
  (display "\r\n" (current-output-port))
  (if body (display (utf8->string body))))


